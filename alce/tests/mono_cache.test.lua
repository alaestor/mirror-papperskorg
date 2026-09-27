local test = require("test_utils")
require("tools.env_mock")

---@diagnostic disable: lowercase-global: as declared by CE's monoscript.lua

local previousHookCalls = 0
local previousHookArguments

rawset(MainForm, "OnProcessOpened", function(processID, processHandle, caption)
    previousHookCalls = previousHookCalls + 1
    previousHookArguments = { processID, processHandle, caption }
end)

local ROOT, ENTITY, PLAYER, SIBLING = 0x100, 0x200, 0x300, 0x400
local IMAGE, ASSEMBLY = 0x900, 0x500
local parents = {
    [ROOT] = 0,
    [ENTITY] = ROOT,
    [PLAYER] = ENTITY,
    [SIBLING] = ENTITY,
}
local names = {
    [ROOT] = "Object",
    [ENTITY] = "Entity",
    [PLAYER] = "Player",
    [SIBLING] = "Sibling",
}
local methodInfo = {
    [0x101] = { name = "Tick", signature = "" },
    [0x201] = { name = "Act", signature = "int" },
    [0x202] = { name = "Act", signature = "string" },
    [0x301] = { name = "Tick", signature = "" },
}
local methods = {
    [ROOT] = { { method = 0x101, name = "Tick", flags = METHOD_ATTRIBUTE_STATIC } },
    [ENTITY] = {
        { method = 0x201, name = "Act", flags = METHOD_ATTRIBUTE_STATIC },
        { method = 0x202, name = "Act", flags = METHOD_ATTRIBUTE_STATIC },
    },
    [PLAYER] = { { method = 0x301, name = "Tick", flags = METHOD_ATTRIBUTE_STATIC } },
    [SIBLING] = {},
}
local fields = {
    [ROOT] = { { name = "rootField", offset = 0x10, monotype = MONO_TYPE_I4, flags = 0 } },
    [ENTITY] = { { name = "entityField", offset = 0x14, monotype = MONO_TYPE_I4, flags = 0 } },
    [PLAYER] = { { name = "playerField", offset = 0x18, monotype = MONO_TYPE_I4, flags = 0 } },
    [SIBLING] = { { name = "siblingField", offset = 0x1c, monotype = MONO_TYPE_I4, flags = 0 } },
}
local fieldCalls, methodCalls, imageCalls

mono_class_getParent = function(id) return parents[id] or 0 end
mono_class_getName = function(id) return names[id] end
mono_class_getNamespace = function() return "Game" end
mono_class_enumFields = function(id, includeParents)
    test.expect(includeParents).to_be_false()
    fieldCalls[id] = (fieldCalls[id] or 0) + 1
    return fields[id]
end
mono_class_enumMethods = function(id, includeParents)
    test.expect(includeParents).to_be_false()
    methodCalls[id] = (methodCalls[id] or 0) + 1
    return methods[id]
end
mono_structfields_getStartOffset = function(items)
    return items[1] and items[1].offset or 0x10
end
mono_method_get_parameters = function() return { parameters = {} } end
mono_method_getSignature = function(id)
    return methodInfo[id].signature, {}, "void"
end
mono_method_getName = function(id) return methodInfo[id].name end
mono_method_getFullName = function(id)
    return "Game." .. methodInfo[id].name .. " ()"
end
mono_splitParameters = function(signature)
    if signature == "" then return {} end
    return { signature }
end
mono_image_enumClassesEx = function()
    imageCalls = imageCalls + 1
    return {
        { Handle = ROOT, FullName = "Object", NameSpace = "Game" },
        { Handle = ENTITY, FullName = "Entity", NameSpace = "Game" },
        { Handle = PLAYER, FullName = "Player", NameSpace = "Game" },
        { Handle = SIBLING, FullName = "Sibling", NameSpace = "Game" },
    }
end
mono_enumAssemblies = function() return { ASSEMBLY } end
mono_getImageFromAssembly = function() return IMAGE end
mono_image_get_name = function() return "mock_image" end

local alce = require("main")

local function resetFixture()
    MainForm.OnProcessOpened(1234, 0, "mock process")
    fieldCalls, methodCalls, imageCalls = {}, {}, 0
end

---@param name string
---@return mono.Class
local function loadClass(name)
    return assert(alce.mono.Class.new({
        assemblyNameOrImage = IMAGE,
        className = name,
        namespace = "Game",
    }))
end

local function throws(callback)
    return not pcall(callback)
end

test.run_and_report(function()
    test.describe("structural Mono class cache", function()
        test.it("should reuse populated ancestors and image indexes", function()
            resetFixture()

            local player = loadClass("Player")
            local sibling = loadClass("Sibling")

            test.expect(player.parent).to_eq(sibling.parent)
            test.expect(fieldCalls[ROOT]).to_eq(1)
            test.expect(fieldCalls[ENTITY]).to_eq(1)
            test.expect(methodCalls[ROOT]).to_eq(1)
            test.expect(methodCalls[ENTITY]).to_eq(1)
            test.expect(imageCalls).to_eq(1)
            test.expect(loadClass("Player")).to_eq(player)
        end)

        test.it("should resolve inherited fields and effective method overloads", function()
            resetFixture()

            local player = loadClass("Player")

            test.expect(player.offset.playerField).to_eq(0x18)
            test.expect(player.offset.entityField).to_eq(0x14)
            test.expect(player.offset.rootField).to_eq(0x10)
            test.expect(player.method["Tick()"]).to_eq(player.method.Tick)
            test.expect(player.method.Tick.id).to_eq(0x301)
            test.expect(player.method["Act(int)"]).to_eq(player.parent.method["Act(int)"])
            test.expect(player.method.Act).to_be_false()
        end)

        test.it("should invalidate class tables and preserve their targets", function()
            resetFixture()
            local classes = alce.mono.ClassTable.new({
                keyPrefixAssembly = false,
                keyPrefixNamespace = false,
            })
            classes:addFromImage({ image = IMAGE, targets = { { "Player", "Game" } } })
            classes:load()
            local staleClass = classes.Player
            local staleMethod = staleClass.method.Tick
            local staleAlias = staleClass:instance({ baseAddress = 0x10000 })

            MainForm.OnProcessOpened(1234, 0, "mock process")

            test.expect(classes:isLoaded()).to_be_false()
            test.expect(classes.Player).to_be_false()
            test.expect(#classes._internal.targetList).to_eq(1)
            test.expect(previousHookCalls > 0).to_be_true()
            test.expect(previousHookArguments[1]).to_eq(1234)
            test.expect(previousHookArguments[2]).to_eq(0)
            test.expect(previousHookArguments[3]).to_eq("mock process")
            test.expect(throws(function()
                staleClass:instance({ baseAddress = 0x10000 })
            end)).to_be_true()
            test.expect(throws(function()
                staleMethod:compile()
            end)).to_be_true()
            test.expect(throws(function()
                local _ = staleAlias.rootField
            end)).to_be_true()

            classes:load()
            test.expect(classes.Player == staleClass).to_be_false()
        end)
    end)
end)
