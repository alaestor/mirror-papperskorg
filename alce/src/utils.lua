local validators = require("validators")
local alce = require("globals")
local fnlua = require("fn")
local fn = fnlua.wrap
local Param = fnlua.Param

local utils = {}

---Reads a chain of pointers starting from the given pointer and following the provided offsets.
---@param pointer Address
---@vararg Offset
---@return Address? result: the resulting address resolved from recursive indirection if successful, otherwise nil
function utils.readPointerChain(pointer, ...)
    assert(validators.isAddresslike(pointer), 'alce.readPointerChain(): invalid argument: pointer')
    local currentPtr = readPointer(pointer)
    local offsets = { ... }
    for _, offset in ipairs(offsets) do
        if not validators.isOffsetlike(offset) then
            alce.warn('alce.readPointerChain(): suspicious offset: ' .. tostring(offset))
        end
        currentPtr = readPointer(currentPtr + offset)
        if not validators.isAddresslike(currentPtr) then
            alce.warn('alce.readPointerChain(): invalid address when trying to chain the following:\n' ..
                alce.fmt.table({ tbl = { pointer = pointer, offsets = offsets } }))
            return nil
        end
    end
    return currentPtr
end

---Reads a chain of pointers and asserts the resulting address isAddressLike.
---@param pointer Address
---@vararg Offset
---@return Address address the address resolved from recursive indirection
function utils.safeChain(pointer, ...)
    local info = debug.getinfo(2, "Sl")
    local result = utils.readPointerChain(pointer, ...)
    assert(validators.isAddresslike(result),
        'line ' ..
        tostring(info.currentline) ..
        ': alce.safeChain(): resulting pointer was invalid:\n' ..
        alce.fmt.table({ tbl = { pointer = pointer, offsets = { ... }, result = result } }))
    ---@diagnostic disable-next-line: return-type-mismatch: guarenteed to be non-nil from validators.isAddresslike
    return result
end

---Enumerates values returned from another iterator
---@param iterator fun(): any the underlying iterator
---@param startFrom integer the starting index to be incremented from for each iterations
---@return fun(): index:integer?, value:any iterator the enumerating iterator
function utils.enumerate(iterator, startFrom)
    startFrom = startFrom or 1
    local count = startFrom
    return function()
        local value = iterator()
        if value == nil then return nil end
        local index = count
        count = count + 1
        return index, value
    end
end

---Recursively nils keys with empty tables.
---@param tbl table the table to prune in-place
function utils.prune(tbl)
    local function prune_recursive(t)
        if type(t) ~= "table" then return end
        for k, v in pairs(t) do
            if type(v) == "table" then
                prune_recursive(v)
                if next(v) == nil then tbl[k] = nil end
            end
        end
    end
    prune_recursive(tbl)
end

---Returns the first key associated with a given value, or nil if not found.
---@overload fun(args: { value: any, table: table }): any|nil
utils.keyFromValue = fn({
    params = {
        value = Param.required(),
        table = Param.required():validate(validators.isTable),
    },
    body = function(args)
        for k, v in pairs(args.table) do
            if v == args.value then return k end
        end
    end,
})

---Returns a sorted array of all keys associated with a given value.
---@overload fun(args: { value: any, table: table }): table
utils.keysFromValue = fn({
    params = {
        value = Param.required(),
        table = Param.required():validate(validators.isTable),
    },
    body = function(args)
        local keys = {}
        for k, v in pairs(args.table) do
            if v == args.value then table.insert(keys, k) end
        end
        table.sort(keys)
        return keys
    end,
})

---Assigns k,v pairs from one table to another, silently overwriting duplicate keys.
---@overload fun(args: { dest: table, src: table })
utils.unsafeExtend = fn({
    params = {
        dest = Param.required():validate(validators.isTable),
        src = Param.required():validate(validators.isTable),
    },
    body = function(args)
        for k, v in pairs(args.src) do
            args.dest[k] = v
        end
    end,
})

---Assigns k,v pairs from one table to another, asserting that the keys do not already exist.
---@overload fun(args: { dest: table, src: table })
utils.extend = fn({
    params = {
        dest = Param.required():validate(validators.isTable),
        src = Param.required():validate(validators.isTable),
    },
    body = function(args)
        for k, v in pairs(args.src) do
            assert(not args.dest[k], 'alce.extend(): key already exists: ' .. tostring(k))
            args.dest[k] = v
        end
    end,
})

return utils
