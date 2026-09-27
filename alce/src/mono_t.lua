local validators = require("validators")
local alce = require("globals")
local fnlua = require("fn")
local fn = fnlua.wrap
local Param = fnlua.Param

---@class MonoT
local T = {}

---Represents a managed heap list with configurable index offsets and item/size pointers.
---@class T.List
---@field baseAddress Address
---@field indexFrom Offset
---@field indexBy integer
---@field offset table<string, Offset>
T.List = {
    indexFrom = 0x20,
    indexBy = 0x8,
    offset = {
        items = 0x10,
        size = 0x18
    },

    --- Creates a new T.List representation at the given baseAddress.
    ---@overload fun(self: T.List, args: { baseAddress: Address, indexFrom?: Offset, indexBy?: Offset, offsetItems?: Offset, offsetSize?: number }): T.List
    new = fn.method({
        params = {
            baseAddress = Param.required():validate(validators.isAddresslike),
            indexFrom = Param.optional(),
            indexBy = Param.optional(),
            offsetItems = Param.optional(),
            offsetSize = Param.optional(),
        },
        body = function(self, args)
        local instance = {
            baseAddress = args.baseAddress,
            indexFrom = args.indexFrom or self.indexFrom,
            indexBy = args.indexBy or self.indexBy,
            offset = {
                items = args.offsetItems or self.offset.items,
                size = args.offsetSize or self.offset.size
            }
        }
        setmetatable(instance, { __index = self, __call = self.at })
        return instance
    end,
    }),

    --- Convenience constructor that returns new T.List that aliases the result from `readPointerChain(...)`
    ---@param self T.List
    ---@param pointer Pointer
    ---@vararg Offset
    ---@return T.List
    newFromChain = function(self, pointer, ...)
        return self:new({ baseAddress = alce.utils.safeChain(pointer, ...) })
    end,

    --- Returns the number of items in the list.
    ---@param self T.List
    ---@return integer
    size = function(self)
        assert(validators.isAddresslike(self.baseAddress), 'alce.mono.T.List.size(): invalid state: baseAddress: ' .. tostring(self.baseAddress))
        return readInteger(self.baseAddress + self.offset.size)
    end,

    --- Returns address of the Nth element at index (starting from zero) without bounds checking.
    ---@param self T.List
    ---@param index Offset
    ---@return Address
    atUnsafe = function(self, index)
        local itemBase = readPointer(self.baseAddress + self.offset.items)
        return readPointer(itemBase + self.indexFrom + (index * self.indexBy))
    end,

    --- Returns address of the Nth element at index (starting from zero) with bounds checking.
    ---@param self T.List
    ---@param index integer
    ---@return Address
    at = function(self, index)
        assert(validators.isAddresslike(self.baseAddress), 'alce.mono.T.List.at(): invalid state: baseAddress: ' .. tostring(self.baseAddress))
        assert(index < self:size(), 'alce.mono.T.List.at(): invalid argument: index is out of bounds')
        return self:atUnsafe(index)
    end,

    --- Returns an iterator which returns the value of the list item from first to end.
    ---@overload fun(self: T.List, args: { first?: number, last?: number }): fun(): (integer, number)?
    iterator = fn.method({
        params = {
            first = Param.default({ value = 0 }):validate(validators.isNonNegativeInteger),
            last = Param.optional(),
        },
        body = function(self, args)
        local i = args.first
        local size = self:size()
        local last = args.last
        local e = last or size
        assert(e <= size, 'alce.mono.T.List.iterator(): invalid argument: last was out of bounds: ' .. tostring(last))

        local info = debug.getinfo(2, "Sl")
        return function()
            if i < e then
                assert(i < self:size(),
                    'line ' ..
                    tostring(info.currentline) ..
                    ': alce.mono.T.List.iterator(): iterator went out of bounds during iteration... Size changed?')
                i = i + 1
                return i, self:atUnsafe({ index = i - 1 })
            end
        end
    end,
    }),

    --- Convenience method wraps the result of the iterator in `alceClass:instance`, returning object instance aliases rather than addresses.
    ---@overload fun(self: T.List, args: { alceClass: table, first?: number, last?: number }): (fun(): (integer, any)?)
    instanceIterator = fn.method({
        params = {
            alceClass = Param.required():validate(validators.isTable),
            first = Param.optional(),
            last = Param.optional(),
        },
        body = function(self, args)
        local alceClass = args.alceClass
        local iter = self:iterator({ first = args.first, last = args.last })
        return function()
            local i, r = iter()
            if r then
                return i, alceClass:instance(r)
            else
                return nil
            end
        end
    end,
    }),
}

return T
