local validators = require("validators")
local alce = require("globals")
local T = require("t")
local fnlua = require("fn")
local fn = fnlua.wrap
local Param = fnlua.Param
local mono_cache = require("mono_cache")

local mono_plumbing = {}

-- Internal helper functions

-- is this "type" the acleclass?
---@alias MonoAliasContext {type: mono.Class, baseAddress: Address, generation: integer}

---@param context MonoAliasContext
---@param key string Field name to read
local function aliasRead(context, key)
    mono_cache.assertCurrent(context.generation, 'alce.mono.ObjectAlias read')
    local class = context.type
    local offset = class.offset[key]
    if offset then
        assert(validators.isNonEmptyTable(class.meta),
            'alce.mono.ObjectAlias(): error in aliasRead: metadata table for "' ..
            tostring(class.name) .. '" was invalid; not populated?')
        local meta = class.meta.offset[key]
        local t = T.fromMono({ monoType = meta.monotype })
        return t:read({ address = context.baseAddress + offset })
    end
    local const = class.const[key]
    if const ~= nil then return const end
    local static = class.static[key]
    if static and validators.isAddresslike(static) then return readPointer(static) end
    assert(false, 'alce.mono.ObjectAlias(): error in aliasRead: failed to find datamember ' .. tostring(key))
end

---@param context MonoAliasContext
---@param key string Field name to write
---@param value any Value to write
local function aliasWrite(context, key, value)
    mono_cache.assertCurrent(context.generation, 'alce.mono.ObjectAlias write')
    local class = context.type
    local offset = class.offset[key]
    if offset then
        assert(validators.isNonEmptyTable(class.meta), 'alce.mono.ObjectAlias(): error in aliasWrite: metadata table for "' .. tostring(class.name) .. '" was invalid; not populated?')
        local meta = class.meta.offset[key]
        local t = T.fromMono({ monoType = meta.monotype })
        return t:write({ address = context.baseAddress + offset, value = value })
    end
    assert(not class.const[key], "alce.mono.ObjectAlias(): error in aliasWrite: can't write to constant value: " .. tostring(key))
    assert(not class.static[key], "alce.mono.ObjectAlias(): error in aliasWrite: shouldn't write to static member: " .. tostring(key) .. ". Did you really intend to? You can try doing it manually.")
    assert(false, 'alce.mono.ObjectAlias(): error in aliasWrite: failed to find datamember ' .. tostring(key))
end

-- Public API

--- Initializes mono state
mono_plumbing.init = function()
    assert(alce.isAttached(), 'alce.mono.init(): not attached to process')
    if (monopipe == nil) then
        assert(LaunchMonoDataCollector(), 'alce.mono.init(): LaunchMonoDataCollector() failed.')
    end
end

--- Returns an array of members sorted by parent hierarchy
---@overload fun(args: {array: table, hierarchy: table}): table
mono_plumbing.sortByHierarchy = fn({
    params = {
        array = Param.required():validate(validators.isTable),
        hierarchy = Param.required():validate(validators.isNonEmptyTable),
    },
    body = function(args)
        local array = args.array
        local hierarchy = args.hierarchy
        if not array then return nil end
        local ClassID = hierarchy[#hierarchy]
        local classOrder = {}
        for i, classId in ipairs(hierarchy) do classOrder[classId] = i end
        table.sort(array,
            function(a, b)
                local orderA = classOrder[a.parent or ClassID]
                local orderB = classOrder[b.parent or ClassID]
                if orderA ~= orderB then return orderA < orderB end
                    if a.name ~= b.name then return a.name < b.name end
                        return (a.field or a.method) < (b.field or b.method)
            end
        )
        return array
    end,
})

---@class getImage_arguments
---@field assemblyName string Assembly name to find
---@field enumeratedAssemblies table? Pre-enumerated assemblies

--- Gets an image by assembly name
---@overload fun(args: getImage_arguments): AssemblyImage?
mono_plumbing.getImage = fn({
    params = {
        assemblyName = Param.required():validate(validators.isNonBlankString),
        enumeratedAssemblies = Param.optional(),
    },
    body = function(args)
        local assemblyName = args.assemblyName
        local enumeratedAssemblies = args.enumeratedAssemblies
        local assemblies = enumeratedAssemblies or mono_enumAssemblies()
        assert(validators.isNonEmptyTable(assemblies), "alce.mono.getImage(): Couldn't enumerate assemblies. Mono features not active?")
        ---@cast assemblies AssemblyId[]
        for _, assembly in ipairs(assemblies) do
            local image = mono_getImageFromAssembly(assembly)
            if image and mono_image_get_name(image) == assemblyName then return image end
            end
            alce.warn("alce.mono.getImage(): Failed to find " .. tostring(assemblyName))
            return nil
    end,
})

---@param existencePolicy fn.Param
---@return fn.Param
local assemblyNameOrImageParam = function(existencePolicy)
    return existencePolicy
        :transform(function(v)
            if validators.isPositiveInteger(v) then
                return v
            end
            if validators.isNonBlankString(v) then
                local image = mono_plumbing.getImage({ assemblyName = v })
                if validators.isPositiveInteger(image) then
                    return image
                else
                    return nil, "Failed to get image from name: " .. tostring(v)
                end
            end
            return nil, "Must be an assembly name or an Image. Got:" .. tostring(v)
        end)
end

---@class getClass_arguments
---@field assemblyNameOrImage string|AssemblyImage Assembly name or image handle
---@field className string Class name
---@field namespace string? Optional namespace

--- Finds a class by name in a specific assembly and namespace
---@overload fun(args: getClass_arguments): table?
mono_plumbing.getClassEx = fn({
    params = {
        assemblyNameOrImage = assemblyNameOrImageParam(Param.required()),
        className = Param.required():validate(validators.isNonBlankString),
        namespace = Param.optional(),
    },
    body = function(args)
        local assemblyNameOrImage = args.assemblyNameOrImage
        local className = args.className
        local namespace = args.namespace
        local imageIndex = mono_cache.classesByImage[assemblyNameOrImage]
        if not imageIndex then
            imageIndex = { byFullName = {}, byNamespace = {} }
            local classes = assert(mono_image_enumClassesEx(assemblyNameOrImage),
                'alce.mono.getClassEx(): failed to enumerate image classes')
            for _, class in ipairs(classes) do
                if not imageIndex.byFullName[class.FullName] then
                    imageIndex.byFullName[class.FullName] = class
                end
                imageIndex.byNamespace[(class.NameSpace or "") .. "\0" .. class.FullName] = class
            end
            mono_cache.classesByImage[assemblyNameOrImage] = imageIndex
        end

        local class = namespace
            and imageIndex.byNamespace[namespace .. "\0" .. className]
            or imageIndex.byFullName[className]
        if class then return class end

        alce.warn("alce.mono.getClassEx(): No results for getClassEx: assembly=" .. tostring(assemblyNameOrImage) .. ", classname=" .. className .. ((namespace and ', namespace="' .. namespace .. '"') or '') .. ")")
        return nil
    end,
})

--- Returns the class handle for a given class
---@overload fun(args: getClass_arguments): ClassId?
mono_plumbing.getClass = fn({
    params = {
        assemblyNameOrImage = assemblyNameOrImageParam(Param.required()),
        className = Param.required():validate(validators.isNonBlankString),
        namespace = Param.optional(),
    },
    body = function(args)
        local r = mono_plumbing.getClassEx(args)
        return r and r.Handle or nil
    end,
})

---@class method_getSignature_arguments
---@field methodID MethodId Method identifier
---@field methodName string? Optional method name override

---@class MethodSignature
---@field types string[]
---@field paramnames string[]
---@field returntype string
---@field initials string
---@field full string

--- Returns the signature of a method
---@overload fun(args: method_getSignature_arguments): MethodSignature
mono_plumbing.method_getSignature = fn({
    params = {
        methodID = Param.required():validate(validators.isPositiveInteger),
        methodName = Param.optional(),
    },
    body = function(args)
        local methodID = args.methodID
        local name = validators.isNonBlankString(args.methodName) and args.methodName or mono_method_getName(methodID)
        local paramtypes_raw, paramnames, returntype = mono_method_getSignature(methodID)
        assert(paramtypes_raw and paramnames and returntype,
            'alce.mono.method_getSignature(): failed to get method signature')
        local paramtypes = string.gsub(paramtypes_raw, '/', '+')
        local t = { types = paramtypes, paramnames = paramnames, returntype = returntype }
        t.initials = string.format('%s(%s)', name, paramtypes or '')
        local typenames = mono_splitParameters(paramtypes)
        local params = ''
        if typenames and #typenames == #paramnames then
            local rr = {}
            for i = 1, #typenames do rr[i] = typenames[i] .. ' ' .. paramnames[i] end
                params = table.concat(rr, ",")
        end
        -- derive the qualifier (e.g. "Namespace+Class:method") robustly
        local front = ''
        local fullname = mono_method_getFullName(methodID)
        if fullname and fullname ~= '' then
            local m = string.match(fullname, '^(.-) %(')
            if m then front = string.gsub(m, '/', '+') end
        end
        -- fallback 1: build it from the declaring type
        if front == '' then
            local cls = mono_method_getClass(methodID)
            if cls and cls ~= 0 then
                local ns = mono_class_getNamespace(cls)
                local cn = mono_class_getName(cls)
                local qual = (ns and ns ~= '' and (ns .. '.' .. cn)) or (cn or '')
                front = string.gsub(qual, '/', '+')
                -- append the method name so the qualifier stays in the same shape
            end
        end
        -- fallback 2: just the method name, last resort
        if front == '' then front = name or '' end
        t.full = string.format('%s(%s)', front, params)
        return t
    end,
})

--- Returns an array of class IDs ordered from parent to child
---@overload fun(args: { classID: ClassId }): table<integer,ClassId>
mono_plumbing.class_getParentHierarchy = fn({
    params = { classID = Param.required():validate(validators.isPositiveInteger) },
    body = function(args)
        local hierarchy = {}
        local current_class = args.classID
        while current_class and current_class ~= 0 do
            table.insert(hierarchy, 1, current_class)
            current_class = mono_class_getParent(current_class)
        end
        return hierarchy
    end,
})

---@class getProcessedFields_arguments
---@field classID ClassId Class
---@field keepMetadata boolean? Keep metadata in output
---@field keepFields boolean? Keep raw field data in output

--- Enumerates and processes fields for a class
---@overload fun(args: getProcessedFields_arguments): table?
mono_plumbing.getProcessedFields = fn({
    params = {
        classID = Param.required():validate(validators.isPositiveInteger),
        keepMetadata = Param.optional():validate(validators.isBoolean),
        keepFields = Param.optional():validate(validators.isBoolean)
    },
    body = function(args)
        local classID = args.classID
        local keepMetadata = args.keepMetadata
        local keepFields = args.keepFields
        local fields = mono_class_enumFields(classID, false)
        if type(fields) ~= 'table' then
            alce.warn('alce.mono.getProcessedFields(): failed to lookup fields')
            return nil
        end
        local startOffset = mono_structfields_getStartOffset(fields)
        if not startOffset then startOffset = targetIs64Bit() and 0x10 or 0x8 end
            local t = {
                startOffset = startOffset,
                const = {},
                static = {},
                offset = {},
                meta = keepMetadata == true and {
                    const = {},
                    static = {},
                    offset = {},
                } or nil,
                constToKey = {},
                fields = keepFields == true and fields or nil,
            }
            ---@param subtable string Subtable key ("const", "static", or "offset")
            ---@param f table Field data with `name` field
            local function warnIfCollides(subtable, f)
                if not t[subtable][f.name] then return nil end
                    local dstr = ((not alce.cfg.debug_print and "Set 'alce.cfg.debug_print=true` for more information.")
                        or string.format('\n[WARN-DEBUG]>\nReplacing...\n---\n%s\n---\nwith data from...\n---\n%s\n---\n',
                            alce.fmt.table({ tbl = t[subtable][f.name] }), alce.fmt.table({ tbl = f })))
                    alce.debug(string.format("alce.mono.getProcessedFields(): duplicate %s field named '%s' detected. %s",
                        subtable, f.name, dstr))
            end
            ---@param ts string Subtable key
            ---@param f table Field data
            ---@param val any Value to store
            local function store(ts, f, val)
                warnIfCollides(ts, f)
                t[ts][f.name] = val
                if keepMetadata == true then
                    t.meta[ts][f.name] = {}
                    local meta = t.meta[ts][f.name]
                    for _, k in pairs({ 'typename', 'type', 'monotype', 'flags' }) do meta[k] = f[k] end
                        meta.vtype = monoTypeToVartypeLookup[meta.monotype] or nil
                end
            end
            for _, f in pairs(fields) do
                if f.staticAddress then
                    store('static', f, f.staticAddress)
                elseif f.isConst then
                    store('const', f, mono_class_getStaticFieldValue(classID, f.field))
                elseif f.offset then
                    store('offset', f, f.offset)
                else
                    alce.warn("alce.mono.getProcessedFields(): uncategorized field; what is this?\n??:" .. alce.fmt.table({ tbl = f }))
                end
            end
            t.const.enumSeperator = nil
            t.static.enumSeparatorCharArray = nil
            if next(t.const) then for k, v in pairs(t.const) do t.constToKey[v] = k end end
                return t or nil
    end,
})

---@class ProcessedMethods
---@field bySignature table<string, Method>
---@field signaturesByName table<string, table<string, true>>

---@class getProcessedMethods_arguments
---@field classID ClassId Class identifier

--- Enumerates and processes methods for a class
---@overload fun(args: getProcessedMethods_arguments): ProcessedMethods?
mono_plumbing.getProcessedMethods = fn({
    params = {
        classID = Param.required(),
    },
    body = function(args)
        local classID = args.classID
        local methods = mono_class_enumMethods(classID, false)
        if type(methods) ~= 'table' then
            alce.warn('alce.mono.getProcessedMethods(): Failed to enumerate methods')
            return nil
        end
        ---@type ProcessedMethods
        local t = { bySignature = {}, signaturesByName = {} }
        for _, mpack in ipairs(methods) do
            local m = alce.mono.Method.new({ methodID = mpack.method, name = mpack.name, flags = mpack.flags })
            t.bySignature[m.signature.initials] = m
            t.signaturesByName[mpack.name] = t.signaturesByName[mpack.name] or {}
            t.signaturesByName[mpack.name][m.signature.initials] = true
        end
        return t
    end,
})

--- Creates a proxy object for a mono object instance at a given address
---@overload fun(args: {alceClass: mono.Class, baseAddress: Address}): table
mono_plumbing.ObjectAlias = fn({
    params = {
        alceClass = Param.required():validate(validators.isTable),
        baseAddress = Param.required():validate(validators.isAddresslike),
    },
    body = function(args)
        local alceClass = args.alceClass
        local baseAddress = args.baseAddress
        local internal = {
            baseAddress = baseAddress,
            type = alceClass,
            generation = alceClass._generation,
        }
        local proxy = {}
        setmetatable(proxy, {
            __tostring = function()
                return string.format("%s@%s", alceClass.name,
                    alce.fmt.address(internal.baseAddress))
            end,
            __pairs = function(_)
                error("alce.mono.ObjectAlias.__pairs(): Aliases don't support iteration; not sure what behavior was expected. Perhaps you want the alce.mono.Class; try `.__type`?")
            end,
            __eq = function(a, b)
                local ameta = getmetatable(a)
                local bmeta = getmetatable(b)
                return ameta == bmeta and rawequal(ameta.__index, bmeta.__index) and a.__baseAddress == b.__baseAddress
            end,
            __index = function(tbl, key)
                mono_cache.assertCurrent(internal.generation, 'alce.mono.ObjectAlias lookup')
                if not validators.isNonBlankString(key) then
                    alce.warn('alce.mono.ObjectAlias.__index(): invalid argument: key should be the name of a data-member or method. Got: ' .. tostring(key))
                    return nil
                end
                local m = internal.type.method[key]
                if m then
                    return function(optional_self, ...)
                        if optional_self == tbl then
                            return m:call(internal.baseAddress, ...)
                        else
                            return m:call(internal.baseAddress, optional_self, ...)
                        end
                    end
                elseif key:sub(1, 2) == "__" then
                    return internal[key:sub(3)]
                else
                    return aliasRead(internal, key)
                end
            end,
            __newindex = function(_, key, value) aliasWrite(internal, key, value) end
                })
        return proxy
    end,
})

return mono_plumbing
