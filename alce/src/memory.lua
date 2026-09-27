local validators = require("validators")
local alce = require("globals")
local fnlua = require("fn")
local fn = fnlua.wrap
local Param = fnlua.Param

local memory = {}

---Registers symbols defined in a context, optionally filtered by a list of names.
---@overload fun(args: { context: memory.AllocateSymbolsContext, names?: string[] })
memory.AllocateSymbols_register = fn({
    params = {
        context = Param.required():validate(validators.isTable),
        names = Param.optional(),
    },
    body = function(args)
    local context = args.context
    local names = args.names
    local namesToRegister = names or context.names

    for _, name in ipairs(namesToRegister) do
        assert(context.addresses[name],
            "alce.memory.AllocateSymbols.register(): invalid argument: invalid name: " .. tostring(name))
        if not context.registered[name] then
            local symbol = context.symbolNames[name]
            local address = context.addresses[name]
            registerSymbol(symbol, address, true)
            context.registered[name] = symbol
        end
    end
end,
})

---Unregisters symbols defined in a context, optionally filtered by a list of names.
---@overload fun(args: { context: memory.AllocateSymbolsContext, names?: string[] })
memory.AllocateSymbols_unregister = fn({
    params = {
        context = Param.required():validate(validators.isTable),
        names = Param.optional(),
    },
    body = function(args)
    local context = args.context
    local names = args.names
    local namesToUnregister = {}

    if names == nil then
        for name, _ in pairs(context.registered) do
            table.insert(namesToUnregister, name)
        end
    else
        namesToUnregister = names
    end

    for _, name in ipairs(namesToUnregister) do
        assert(context.registered[name],
            "alce.memory.AllocateSymbols.unregister(): invalid argument: symbol not registered: " .. tostring(name))
        local symbol = context.registered[name]
        unregisterSymbol(symbol)
        context.registered[name] = nil
    end
end,
})

---Allocates contiguous memory aliased by name and provides read/write access.
---@class memory.AllocateSymbolsContext
---@field size number
---@field memory Address
---@field names string[]
---@field symbolPrefix string
---@field symbolNames table<string, string>
---@field addresses table<string, Address>
---@field types table<string, table>
---@field registered table<string, string>
---
---@class memory.AllocateSymbolsProxy
---
---@overload fun(args: { packets: table, doNotRegister?: boolean, symbolPrefix?: string, baseAddress?: number, protection?: boolean }): table
memory.AllocateSymbols = fn({
    params = {
        packets = Param.required():validate(validators.isNonEmptyTable),
        doNotRegister = Param.default({ value = false }):validate(function(value) return type(value) == "boolean" end),
        symbolPrefix = Param.default({ value = "" }):validate(function(value) return type(value) == "string" end),
        baseAddress = Param.optional(),
        protection = Param.optional(),
    },
    body = function(args)
    local packets = args.packets
    local doNotRegister = args.doNotRegister
    local symbolPrefix = args.symbolPrefix or ""
    local baseAddress = args.baseAddress

    ---@type memory.AllocateSymbolsContext
    local internal = {
        size = 0,
        ---@diagnostic disable-next-line: assign-type-mismatch
        memory = nil,
        names = {},
        symbolPrefix = symbolPrefix,
        symbolNames = {},
        addresses = {},
        types = {},
        registered = {},
    }

    for i, packet in ipairs(packets) do
        assert(validators.isNonEmptyTable(packet), 'alce.memory.AllocateSymbols(): invalid argument: packets[' .. tostring(i) .. '] must be a table of {type=,value=}')
        local name = packet.value
        assert(validators.isNonBlankString(name), 'alce.memory.AllocateSymbols(): invalid argument: packets[' .. tostring(i) .. '] name must be a non-blank string')
        table.insert(internal.names, name)
        local t = alce.T[packet.type]
        assert(t, 'alce.memory.AllocateSymbols(): invalid argument: packets[' .. tostring(i) .. '] type must be a key compatible with alce.T (e.g. a CE vartype like `vtSingle`)')
        internal.types[name] = t
        internal.symbolNames[name] = alce.fmt.sanitizeSymbolName(internal.symbolPrefix .. name)
        internal.size = internal.size + t.size
    end

    local allocated = allocateMemory(internal.size, baseAddress)
    assert(validators.isAddresslike(allocated), 'alce.memory.AllocateSymbols(): failed to allocate memory...')
    internal.memory = allocated --[[@as Address]]

    local cursor = internal.memory
    for _, name in ipairs(internal.names) do
        internal.addresses[name] = cursor
        cursor = cursor + internal.types[name].size
    end

    if not doNotRegister then memory.AllocateSymbols_register({ context = internal }) end

    local proxy = {}
    setmetatable(proxy, {
        __name = 'AllocateSymbolsProxy: ' .. alce.fmt.address(internal.memory),

        __pairs = function(_) error("alce.memory.AllocateSymbols.__pairs(): doesn't support iteration") end,

        __eq = function(a, b)
            local ameta = getmetatable(a)
            local bmeta = getmetatable(b)
            return ameta == bmeta and rawequal(ameta.__index, bmeta.__index)
        end,

        -- __index handles reads and internal access
        __index = function(tbl, key)
            if key:sub(1, 2) == "__" then
                return internal[key:sub(3)]
            elseif key == 'register' then
                return function(optional_self, ...) -- self is unnecessary: workaround to allow both `:` and `.` calling
                    if optional_self == tbl then
                        return memory.AllocateSymbols_register({ context = internal, names = ... })
                    else
                        return memory.AllocateSymbols_register({ context = internal, names = optional_self, ... })
                    end
                end
            elseif key == 'unregister' then
                return function(optional_self, ...) -- self is unnecessary: workaround to allow both `:` and `.` calling
                    if optional_self == tbl then
                        return memory.AllocateSymbols_unregister({ context = internal, names = ... })
                    else
                        return memory.AllocateSymbols_unregister({ context = internal, names = optional_self, ... })
                    end
                end
            else
                local t = internal.types[key]
                assert(t, "alce.memory.AllocateSymbols.__index(): couldn't find type for key: " .. tostring(key))
                local a = internal.addresses[key]
                assert(a, "alce.memory.AllocateSymbols.__index(): couldn't find address for key: " .. tostring(key))
                return t:read({ address = a })
            end
        end,

        -- __newindex handles writes
        __newindex = function(_, key, value)
            local t = internal.types[key]
            assert(t, "alce.memory.AllocateSymbols.__newindex(): couldn't find type for key: " .. tostring(key))
            local a = internal.addresses[key]
            assert(a, "alce.memory.AllocateSymbols.__newindex(): couldn't find address for key: " .. tostring(key))
            return t:write({ address = a, value = value })
        end,
    })
    return proxy
end,
})

return memory
