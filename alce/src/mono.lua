local fnlua = require("fn")
local fn = fnlua.wrap
local Param = fnlua.Param
local validators = require("validators")
local alce = require("globals")
local mono_cache = require("mono_cache")
local mono_plumbing = require("mono_plumbing")
local mono_t = require("mono_t")

---@class mono
--- Mono porcelain helpers for ergonomic interaction with Mono types.
local mono = {
}

local MINIMUM_CE_VERSION = 7.7

mono_cache.installOpenProcessHook()

mono.T = mono_t

--- Asserts Cheat Engine is supported and the process is attached, then launches the Mono data collector.
mono.init = function()
    local version = getCEVersion()
    assert(type(version) == 'number',
        'alce.mono.init(): failed to determine the Cheat Engine version')
    assert(version >= MINIMUM_CE_VERSION,
        string.format('alce.mono.init(): Cheat Engine %.1f or newer is required; running %.1f',
            MINIMUM_CE_VERSION, version))
    return mono_plumbing.init()
end

---@class Method
---@field id MethodId Method identifier
---@field name string Method name
---@field flags number Method flags
---@field parameters table Method parameter metadata
---@field signature MethodSignature Method signature metadata
---@field address number? Compiled method address
---@field _generation integer Process generation in which the method handle is valid
--- A representation of, and call-abstraction for, mono methods.
mono.Method = {
}

---@class Method_init_arguments
---@field methodID number Method identifier
---@field name string Method name
---@field flags number? Method flags

--- Initializes a mono method.
---@overload fun(self: Method, args: Method_init_arguments): Method
mono.Method.init = fn.method({
    params = {
        methodID = Param.required():validate(validators.isPositiveInteger),
        name = Param.required():validate(validators.isNonBlankString),
        flags = Param.optional(),
    },
    body = function(self, args)
    local methodID = args.methodID
    local name = args.name
    local flags = args.flags

    self.id = methodID
    self._generation = mono_cache.generation
    self.flags = flags or mono_method_getFlags(methodID)

    -- The parameters are returned as a table from the C function
    local parameters = assert(mono_method_get_parameters(methodID),
        'alce.mono.Method.init(): failed to get method parameters')
    for k, v in pairs(parameters) do
        self[k] = v
    end

    self.signature = mono_plumbing.method_getSignature({
        methodID = methodID,
        methodName = name
    })
    self.name = name
    return self
end,
})

---@class Method_new_arguments
---@field methodID MethodId Method identifier
---@field name string Method name
---@field flags number Method flags

--- Creates a new mono method instance.
---@overload fun(args: Method_new_arguments): Method
mono.Method.new = fn({
    params = {
        methodID = Param.required():validate(validators.isPositiveInteger),
        name = Param.required(),
        flags = Param.required(),
    },
    body = function(args)
        local instance = {}
        setmetatable(instance, {
            __index = mono.Method,
            __call = function(obj, ...)
                return obj:call(...)
            end
        })

        if args.methodID then
            instance:init({
                methodID = args.methodID,
                name = args.name,
                flags = args.flags
            })
        end
        return instance
    end
})

--- Returns true if the method is static.
---@param self Method
---@return boolean
mono.Method.isStatic = function(self)
    return validators.hasFlag(METHOD_ATTRIBUTE_STATIC, self.flags)
end

--- Gets the names of global `monoscript.lua` constants representing the method attributes.
---@param self Method
---@return string[]
mono.Method.getAttributes = function(self)
    local t = {}
    for _, flagName in pairs(alce.monoscript.methodAttribute.names) do
        if validators.hasFlag(_G[flagName], self.flags) then
            table.insert(t, flagName)
        end
    end
    return t
end

--- Compiles the method for invocation.
---@param self Method
---@return Address
mono.Method.compile = function(self)
    mono_cache.assertCurrent(self._generation, 'alce.mono.Method.compile()')
    if not self.address then
        assert(validators.isPositiveInteger(self.id), 'alce.mono.Method.compile(): malformed method; not initialized?')
        local r = mono_compile_method(self.id)
        assert(validators.isAddresslike(r), 'alce.mono.Method.compile(): failed to compile method')
        self.address = r
    end
    return self.address
end

---@class Method_callUnsafe_arguments
---@field instance number? Mono object instance address
---@field arguments table? Arguments in Cheat Engine's invoke format

--- Invokes the method without safety checks. Arguments must be in Cheat Engine's invoke format.
---@overload fun(self: Method, args: Method_callUnsafe_arguments): Result?, ExceptionString?, Type?
mono.Method.callUnsafe = fn.method({
    params = {
        instance = Param.optional(),
        arguments = Param.optional(),
    },
    body = function(self, args)
    mono_cache.assertCurrent(self._generation, 'alce.mono.Method.callUnsafe()')
    local instance = args.instance
    local arguments = args.arguments
    if alce.cfg.debug_print then
        alce.debug(string.format(
            "alce.mono.Method.callUnsafe(): attempting to invoke call '%s' on instance %s with...\n%s",
            self.signature.full, alce.fmt.address(instance),
            alce.fmt.table({ tbl = { wants = self.parameters, giving = arguments or 'Nothing' } })))
    end
    return mono_invoke_method(nil, self.id, instance or 0, arguments or {})
end,
})

--- Invokes the method after safety checks and argument processing.
---@param self Method
---@param maybe_instance number?
---@param ... any
---@return any
function mono.Method.call(self, maybe_instance, ...)
    mono_cache.assertCurrent(self._generation, 'alce.mono.Method.call()')
    local raw_args = { ... }
    assert(self.id and self.parameters, 'alce.mono.Method.call(): malformed method; not initialized?')

    local pcount = #raw_args
    assert(pcount == #(self.parameters), 'alce.mono.Method.call(): called with the wrong number of parameters')
    assert(self:isStatic() or validators.isAddresslike(maybe_instance),
        'alce.mono.Method.call(): non-static method was called without an instance')

    local args_list = {}
    for i = 1, pcount do
        local arg = raw_args[i]
        assert(arg ~= nil,
            'alce.mono.Method.call(): argument ' .. tostring(i) .. ' was nil... Did you mean `0` or `false`?')
        if type(arg) == 'table' then
            assert(arg.type ~= nil and alce.T[arg.type] and arg.value ~= nil,
                'alce.mono.Method.call(): argument ' ..
                tostring(i) .. ' was a malformed table; tables must be {type=,value=}')
            args_list[i] = arg
        else
            args_list[i] = alce.T.fromMono({ monoType = self.parameters[i].type })(arg)
        end
    end

    local result, exception, _ = self:callUnsafe({ instance = maybe_instance, arguments = args_list })
    assert(not exception, 'alce.mono.Method.call(): exception: ' .. tostring(exception))
    return result
end

---@class mono.Class
---@field id number Class identifier
---@field namespace string Class namespace
---@field name string Class name
---@field parent mono.Class? Cached immediate parent class
---@field method table<string, Method> Processed methods indexed by name and signature
---@field const table<string, any> Literal field values indexed by field name
---@field static table<string, any> Static field values indexed by field name
---@field offset table<string, integer> Instance-field offsets indexed by field name
---@field constToKey table<any, string> Reverse lookup from literal value to field name
---@field meta MonoClassFieldMetadata Raw metadata for processed fields
---@field fields MonoFieldInfo[] Raw fields declared by this class
---@field startOffset integer Lowest instance-field offset in this class hierarchy
---@field _generation integer Process generation in which the class handles are valid
---@field _methodResolution table<string, Method|false> Cached effective-method lookups
---@field _declaredMethods ProcessedMethods Methods declared directly by this class
--- A process-scoped representation of a Mono class and its declared members.
mono.Class = {
}

local classFromID

---@class Class_new_arguments
---@field assemblyNameOrImage number|string Assembly name or image handle
---@field className string Class name
---@field namespace string? Optional namespace

--- Creates a new mono class instance.
---@overload fun(args: Class_new_arguments): mono.Class?
mono.Class.new = fn({
    params = {
        assemblyNameOrImage = Param.required():validate(function(v)
            return validators.isPositiveInteger(v) or validators.isNonBlankString(v)
        end),
        className = Param.required():validate(validators.isNonBlankString),
        namespace = Param.optional(),
    },
    body = function(args)
        local assemblyNameOrImage = args.assemblyNameOrImage
        local className = args.className
        local namespace = args.namespace
        alce.debug('alce.mono.Class.new(): attempting to get "', className, '"')

        local id = mono_plumbing.getClass({
            assemblyNameOrImage = assemblyNameOrImage,
            className = className,
            namespace = namespace
        })

        if not validators.isPositiveInteger(id) then
            alce.warn('alce.mono.Class.new(): failed to find class: ' .. tostring(className))
            return nil
        end
        ---@cast id ClassId: guarenteed to be non-nil

        return classFromID({ classID = id, name = className, namespace = namespace })
    end
})

---@param class mono.Class
---@param key string
---@return Method?
local function resolveMethod(class, key)
    local cached = class._methodResolution[key]
    if cached ~= nil then return cached ~= false and cached or nil end

    local current = class
    while current do
        local exact = current._declaredMethods.bySignature[key]
        if exact then
            class._methodResolution[key] = exact
            return exact
        end
        current = current.parent
    end

    ---@type table<string, Method>
    local effective = {}
    current = class
    while current do
        local signatures = current._declaredMethods.signaturesByName[key]
        if signatures then
            for signature in pairs(signatures) do
                if not effective[signature] then
                    effective[signature] = current._declaredMethods.bySignature[signature]
                end
            end
        end
        current = current.parent
    end

    local result
    for _, method in pairs(effective) do
        if result then
            class._methodResolution[key] = false
            return nil
        end
        result = method
    end
    class._methodResolution[key] = result or false
    return result
end

---@param declared table
---@param parent table?
---@return table
local function inheritMap(declared, parent)
    -- Only keyed lookup inherits. Iteration intentionally has no inherited-member guarantee.
    return setmetatable(declared, { __index = parent })
end

---@param args {classID: ClassId, name: string?, namespace: string?}
---@return mono.Class?
classFromID = function(args)
    local cached = mono_cache.classesByID[args.classID]
    if cached then return cached end

    local instance = {
        id = args.classID,
        namespace = args.namespace or assert(mono_class_getNamespace(args.classID),
            'alce.mono.Class.new(): failed to get class namespace'),
        name = args.name or assert(mono_class_getName(args.classID),
            'alce.mono.Class.new(): failed to get class name'),
        _generation = mono_cache.generation,
        _methodResolution = {},
    }
    setmetatable(instance, { __index = mono.Class })
    ---@cast instance mono.Class
    -- Publish the shell first so malformed cyclic metadata cannot recurse forever.
    mono_cache.classesByID[args.classID] = instance

    local parentID = mono_class_getParent(args.classID)
    if parentID and parentID ~= 0 then
        instance.parent = classFromID({ classID = parentID })
        if not instance.parent then
            mono_cache.classesByID[args.classID] = nil
            return nil
        end
    end

    local methods = mono_plumbing.getProcessedMethods({ classID = args.classID })
    if not methods then
        alce.warn('alce.mono.Class.new(): failed to get methods: ' .. tostring(instance.name))
        mono_cache.classesByID[args.classID] = nil
        return nil
    end
    instance._declaredMethods = methods
    instance.method = setmetatable({}, {
        __index = function(_, key)
            if type(key) ~= 'string' then return nil end
            return resolveMethod(instance, key)
        end,
    })

    local fields = mono_plumbing.getProcessedFields({
        classID = args.classID,
        keepMetadata = true,
        keepFields = true,
    })
    if not fields then
        alce.warn('alce.mono.Class.new(): failed to get fields: ' .. tostring(instance.name))
        mono_cache.classesByID[args.classID] = nil
        return nil
    end

    local parent = instance.parent
    instance.const = inheritMap(fields.const, parent and parent.const)
    instance.static = inheritMap(fields.static, parent and parent.static)
    instance.offset = inheritMap(fields.offset, parent and parent.offset)
    instance.constToKey = inheritMap(fields.constToKey, parent and parent.constToKey)
    instance.meta = {
        const = inheritMap(fields.meta.const, parent and parent.meta.const),
        static = inheritMap(fields.meta.static, parent and parent.meta.static),
        offset = inheritMap(fields.meta.offset, parent and parent.meta.offset),
    }
    instance.fields = fields.fields
    instance.startOffset = fields.startOffset
    if parent and parent.startOffset then
        instance.startOffset = math.min(instance.startOffset, parent.startOffset)
    end
    return instance
end

---@class Class_instance_arguments
---@field baseAddress number Base address of the object instance

--- Creates an `ObjectAlias` proxy for an instance of this class.
---@overload fun(self: mono.Class, args: Class_instance_arguments): table
mono.Class.instance = fn.method({
    params = { baseAddress = Param.required():validate(validators.isAddresslike) },
    body = function(self, args)
        mono_cache.assertCurrent(self._generation, 'alce.mono.Class.instance()')
        return mono_plumbing.ObjectAlias({
            alceClass = self,
            baseAddress = args.baseAddress
        })
    end,
})

--- Convenient shorthand for self:instance({ baseAddress = alce.safeChain(...) }).
---@param self mono.Class
---@param pointer Pointer
---@vararg Offset
---@return table
function mono.Class.instanceFrom(self, pointer, ...)
    return self:instance({ baseAddress = alce.utils.safeChain(pointer, ...) })
end

---@class ClassTable
---@field _internal ClassTableInternal Internal loading state
---@field [string] any Dynamically loaded classes and ClassTable methods
--- A table that loads and holds mono classes by name.
mono.ClassTable = {
}

---@class ClassTableInternal
---@field targetList ClassTableTarget[]
---@field isLoaded boolean
---@field keyPrefixAssembly boolean
---@field keyPrefixNamespace boolean
---@field loadedGeneration integer?

---@alias ClassTableTarget { [1]: string|AssemblyImage, [2]: string, [3]: string? }

---@class ClassTable_new_arguments
---@field keyPrefixAssembly boolean Prefix keys with assembly names
---@field keyPrefixNamespace boolean Prefix keys with namespaces

--- Creates a new mono class table instance.
---@overload fun(args: ClassTable_new_arguments): ClassTable
mono.ClassTable.new = fn({
    params = {
        keyPrefixAssembly = Param.default({value = false, skipPipeline = true}):validate(validators.isBoolean),
        keyPrefixNamespace = Param.default({value = false, skipPipeline = true}):validate(validators.isBoolean),
    },
    body = function(args)
        local instance = {
            _internal = {
                targetList = {},
                isLoaded = false,
                keyPrefixAssembly = args.keyPrefixAssembly,
                keyPrefixNamespace = args.keyPrefixNamespace,
                loadedGeneration = nil,
            }
        }
        setmetatable(instance, { __index = mono.ClassTable })
        mono_cache.registerClassTable(instance)
        return instance
    end
})

---@class ClassTable_add_arguments
---@field targets table? Entries shaped as `{ image, className, namespace? }`

--- Adds explicit targets shaped as `{ {image, className, namespace?}, ... }`.
---@overload fun(self: ClassTable, args: ClassTable_add_arguments): ClassTable
mono.ClassTable.add = fn.method({
    params = { targets = Param.default({ value = {} }):validate(validators.isTable) },
    body = function(self, args)
        assert(not self._internal.isLoaded,
            'alce.mono.ClassTable.add(): cannot add while table is loaded. Try calling unload() first?')
        local entries = args.targets
        alce.debug('alce.mono.ClassTable.add(): added targets: ' .. alce.fmt.table({ tbl = entries }))
        for _, entry in ipairs(entries) do
            table.insert(self._internal.targetList, entry)
        end
        return self
    end,
})

---@class ClassTable_addFromImage_arguments
---@field image number|string Assembly image handle or name
---@field targets table? Class names or `{ className, namespace? }` pairs

--- Adds class targets from a single assembly image.
---@overload fun(self: ClassTable, args: ClassTable_addFromImage_arguments): ClassTable
mono.ClassTable.addFromImage = fn.method({
    params = {
        image = Param.required(),
        targets = Param.default({ value = {} }):validate(validators.isTable),
    },
    body = function(self, args)
        assert(not self._internal.isLoaded, 'alce.mono.ClassTable.addFromImage(): cannot add while table is loaded. Try calling unload() first?')
        local image = args.image
        local items = args.targets
        for _, item in ipairs(items) do
            if type(item) == 'table' then
                table.insert(self._internal.targetList, { image, item[1], item[2] })
            else
                table.insert(self._internal.targetList, { image, item, nil })
            end
        end
        return self
    end,
})

--- Returns true if the class table is loaded.
---@param self ClassTable
---@return boolean
mono.ClassTable.isLoaded = function(self)
    return self._internal.isLoaded == true and self._internal.loadedGeneration == mono_cache.generation
end

--- Loads the classes specified in the target list.
---@param self ClassTable
---@return ClassTable self
mono.ClassTable.load = function(self)
    alce.debug('alce.mono.ClassTable.load(): called.')
    assert(not self:isLoaded(), 'alce.mono.ClassTable.load(): already loaded. Forget to call :unload()?')
    self._internal.isLoaded = true
    self._internal.loadedGeneration = mono_cache.generation
    local assemblies = assert(mono_enumAssemblies(), 'alce.mono.ClassTable.load(): failed to enumerate assemblies')
    ---@cast assemblies table<integer|string, AssemblyId|AssemblyImage>
    local r = {}
    for _, target in ipairs(self._internal.targetList) do
        local assemblyNameOrImage, className, namespace = table.unpack(target)
        local assemblyName
        if type(assemblyNameOrImage) == 'string' then
            assemblyName = assemblyNameOrImage
        else
            assemblyName = assert(mono_image_get_name(assemblyNameOrImage),
                'alce.mono.ClassTable.load(): failed to get assembly image name')
        end
        if not assemblies[assemblyName] then
            assemblies[assemblyName] = mono_plumbing.getImage({ assemblyName = assemblyName, enumeratedAssemblies = assemblies })
        end
        local image = assert(assemblies[assemblyName],
            'alce.mono.ClassTable.load(): failed to resolve assembly image')
        ---@cast image AssemblyImage
        local c = mono.Class.new({
            assemblyNameOrImage = image,
            className = className,
            namespace = namespace
        })
        if c then
            local keyPrefixA = self._internal.keyPrefixAssembly and assemblyName .. '.' or ''
            local keyPrefixB = self._internal.keyPrefixNamespace and c.namespace .. '.' or ''
            local key = keyPrefixA .. keyPrefixB .. className
            if r[key] then
                alce.warn('alce.mono.ClassTable.load(): Overwriting duplicate key: "' ..
                    key .. '" (try enabling assembly or namespace prefixing?)')
            end
            r[key] = c
        end
    end
    for k, v in pairs(r) do
        self[k] = v
    end
    return self
end

--- Unloads the class table.
---@param self ClassTable
---@return ClassTable self
mono.ClassTable.unload = function(self)
    alce.debug('alce.mono.ClassTable.unload(): called.')
    if not self:isLoaded() then
        alce.warn('alce.mono.ClassTable.unload() called but nothing was loaded?... Doing it anyways.')
    end
    local internal = self._internal
    -- We need to clear the keys that were added to the instance
    for k, v in pairs(self) do
        if type(v) == 'function' then
            -- This is one of our methods, don't clear it
        elseif k ~= '_internal' then
            self[k] = nil
        end
    end
    self._internal = internal
    self._internal.isLoaded = false
    self._internal.loadedGeneration = nil
    return self
end

--- Clears the target list and unloads if necessary.
---@param self ClassTable
---@return ClassTable self
mono.ClassTable.clear = function(self)
    alce.debug('alce.mono.ClassTable.clear(): called.')
    if self:isLoaded() then
        self:unload()
    end
    self._internal.targetList = {}
    return self
end

---Invalidates loaded representations while preserving configured targets.
---@param self ClassTable
function mono.ClassTable._invalidateCache(self)
    local internal = self._internal
    for key in pairs(self) do
        if key ~= '_internal' then self[key] = nil end
    end
    self._internal = internal
    self._internal.isLoaded = false
    self._internal.loadedGeneration = nil
end

return mono
