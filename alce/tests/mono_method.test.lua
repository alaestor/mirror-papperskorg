local test = require("test_utils")
require("tools.env_mock")

---@diagnostic disable: lowercase-global: as declared by CE's monoscript.lua

local alce = require("main")

local METHOD_ID = 0x1234

---@param parameters MonoMethodParameter[]
---@param flags MethodAttribute?
---@return Method
local function newMethod(parameters, flags)
    mono_method_get_parameters = function()
        return {
            parameters = parameters,
            returnmonotype = 0,
            returntype = MONO_TYPE_VOID,
        }
    end
    mono_method_getSignature = function() return "", {}, "void" end
    mono_method_getFullName = function() return "Fixture.Method ()" end

    return alce.mono.Method.new({
        methodID = METHOD_ID,
        name = "Method",
        flags = flags or METHOD_ATTRIBUTE_STATIC,
    })
end

local function integerParameter()
    return { name = "value", monotype = 0x100, type = MONO_TYPE_I4 }
end

local function valueTypeParameter()
    return { name = "value", monotype = 0x200, type = MONO_TYPE_VALUETYPE }
end

test.run_and_report(function()
    test.describe("mono.Method.callUnsafe", function()
        test.it("should delegate the invocation and preserve all return values", function()
            local method = newMethod({ integerParameter() })
            local arguments = { { type = vtDword, value = 7 } }
            local captured = {}

            mono_invoke_method = function(domain, methodID, instance, invokeArguments)
                captured.domain = domain
                captured.methodID = methodID
                captured.instance = instance
                captured.arguments = invokeArguments
                return "result", "exception", MONO_TYPE_I4
            end

            local result, exception, resultType = method:callUnsafe({
                instance = 0xfeed,
                arguments = arguments,
            })

            test.expect(captured.domain).to_be_false()
            test.expect(captured.methodID).to_eq(METHOD_ID)
            test.expect(captured.instance).to_eq(0xfeed)
            test.expect(captured.arguments).to_eq(arguments)
            test.expect(result).to_eq("result")
            test.expect(exception).to_eq("exception")
            test.expect(resultType).to_eq(MONO_TYPE_I4)
        end)

        test.it("should default an absent instance and arguments at the CE boundary", function()
            local method = newMethod({})
            local captured = {}

            mono_invoke_method = function(_, _, instance, arguments)
                captured.instance = instance
                captured.arguments = arguments
            end

            method:callUnsafe({})

            test.expect(captured.instance).to_eq(0)
            test.expect(captured.arguments).to_be_type("table")
            test.expect(#captured.arguments).to_eq(0)
        end)
    end)

    test.describe("mono.Method.call", function()
        test.it_throws("should reject an incorrect argument count", function()
            local method = newMethod({ integerParameter() })
            method:call(nil)
        end)

        test.it_throws("should reject a non-static call without an instance", function()
            local method = newMethod({}, 0)
            method:call(nil)
        end)

        test.it_throws("should reject nil arguments", function()
            local method = newMethod({ integerParameter(), integerParameter() })
            method:call(nil, nil, 2)
        end)

        test.it_throws("should reject malformed explicit invoke arguments", function()
            local method = newMethod({ integerParameter() })
            method:call(nil, { type = vtDword })
        end)

        test.it("should preserve explicit invoke arguments", function()
            local method = newMethod({ integerParameter() })
            local argument = { type = vtByte, value = 11 }
            local captured

            mono_invoke_method = function(_, _, _, arguments)
                captured = arguments[1]
                return 22, nil, MONO_TYPE_I4
            end

            local result = method:call(nil, argument)

            test.expect(captured).to_eq(argument)
            test.expect(result).to_eq(22)
        end)

        test.it("should convert scalar arguments to predictable transport types", function()
            local method = newMethod({ integerParameter() })
            local captured

            mono_invoke_method = function(_, _, _, arguments)
                captured = arguments[1]
                return 42, nil, MONO_TYPE_I4
            end

            local result = method:call(nil, 21)

            test.expect(captured.type).to_eq(vtDword)
            test.expect(captured.value).to_eq(21)
            test.expect(result).to_eq(42)
        end)

        test.it_throws("should assert managed exceptions", function()
            local method = newMethod({})
            mono_invoke_method = function()
                return nil, "managed failure", MONO_TYPE_VOID
            end

            method:call(nil)
        end)

        test.it("should leave enum normalization and result formatting to CE", function()
            local method = newMethod({ valueTypeParameter() })
            local receivedTypes = {}
            local callCount = 0

            mono_invoke_method = function(_, _, _, arguments)
                callCount = callCount + 1
                receivedTypes[callCount] = arguments[1].type
                arguments[1].type = vtDword
                return "Enum Fixture.Kind: One (1)", nil, MONO_TYPE_VALUETYPE
            end

            local scalarResult = method:call(nil, 1)
            local explicitResult = method:call(nil, { type = vtPointer, value = 1 })

            test.expect(receivedTypes[1]).to_eq(vtPointer)
            test.expect(receivedTypes[2]).to_eq(vtPointer)
            test.expect(scalarResult).to_eq("Enum Fixture.Kind: One (1)")
            test.expect(explicitResult).to_eq("Enum Fixture.Kind: One (1)")
        end)

        test.it("should delegate value-type receivers and decoded results without compiling", function()
            local method = newMethod({}, 0)
            local compileCalls = 0
            local capturedInstance
            local decoded = { x = 3, y = 4 }

            mono_compile_method = function()
                compileCalls = compileCalls + 1
                return 0xdeadbeef
            end
            mono_invoke_method = function(_, _, instance)
                capturedInstance = instance
                return decoded, nil, MONO_TYPE_VALUETYPE
            end

            local result = method:call(0x10000)

            test.expect(capturedInstance).to_eq(0x10000)
            test.expect(result).to_eq(decoded)
            test.expect(compileCalls).to_eq(0)
        end)
    end)
end)
