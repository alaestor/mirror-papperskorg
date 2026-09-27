local validators = require("validators")
local alce = require("globals")
local fnlua = require("fn")
local fn = fnlua.wrap
local Schema = fnlua.Schema
local Param = fnlua.Param

--[[
CE type helpers for reading, writing, and inspecting vartypes. They back the
user-facing `VTypeHelper` instances available through `alce.T`.

- `typeStrings` lists CE vartype names.
- `size`, `read`, and `write` map vartypes to their respective operations.
]]
local vt = {
    ---@type BasicTypeString[]
    basicTypeStrings = {
        'byte',
        'word',
        'dword',
        'qword',
        'single',
        'double',
        'pointer',
    },
}

---@type VirtualTypeString[]
vt.typeStrings = {}
for _, v in ipairs(vt.basicTypeStrings) do table.insert(vt.typeStrings, 'vt' .. v:sub(1, 1):upper() .. v:sub(2)) end

---@type table<VirtualType, integer>
vt.size = {
    [vtUnicodeString] = 1,
    [vtByte]          = 1,
    [vtWord]          = 2,
    [vtDword]         = 4,
    [vtQword]         = 8,
    [vtSingle]        = 4,
    [vtDouble]        = 8,
    [vtString]        = targetIs64Bit() and 8 or 4,
    [vtPointer]       = targetIs64Bit() and 8 or 4,
}

---@type table<VirtualType, fun(address: Address<any>): any>
vt.read = {
    [vtUnicodeString] = function(addr) return readBytes(addr, 1, false) end,
    [vtByte]          = function(addr) return readBytes(addr, 1, false) end,
    [vtWord]          = readSmallInteger,
    [vtDword]         = readInteger,
    [vtQword]         = readQword,
    [vtSingle]        = readFloat,
    [vtDouble]        = readDouble,
    [vtString]        = readString,
    [vtPointer]       = readPointer,
}

---@type table<VirtualType, fun(address: Address<any>, value: any): boolean>
vt.write = {
    [vtUnicodeString] = function(addr, val) return writeBytes(addr, { val & 0xFF }) end,
    [vtByte]          = function(addr, val) return writeBytes(addr, { val & 0xFF }) end,
    [vtWord]          = writeSmallInteger,
    [vtDword]         = writeInteger,
    [vtQword]         = writeQword,
    [vtSingle]        = writeFloat,
    [vtDouble]        = writeDouble,
    [vtString]        = writeString,
    [vtPointer]       = function(addr, val) return (targetIs64Bit() and writeQword or writeInteger)(addr, val) end,
}



--- Helper for inspecting, reading, writing, and formatting a specific CE vartype.
---@class VTypeHelper
---@field name string
---@field vtName string
---@field vType number
---@field size number
---@field readUnsafe fun(address: number): any
---@field writeUnsafe fun(address: number, value: any): any
--[[
`VTypeHelper` finds related monotypes and performs type-appropriate reads,
writes, and invoke-argument formatting for one CE vartype.
--]]
vt.VTypeHelper = {}

--- Creates a new VType helper.
---@overload fun(self: VTypeHelper, args: { basicTypeString: string }): VTypeHelper
vt.VTypeHelper.new = fn.method({
    params = {
        basicTypeString = Param.required():validate(validators.isNonBlankString),
    },
    body = function(self, args)
    local basicTypeString = args.basicTypeString
    local vts = 'vt' .. basicTypeString:sub(1, 1):upper() .. basicTypeString:sub(2)
    local vt_global = _G[vts]
    assert(validators.isInteger(vt_global),
        'alce.T.VType(): invalid argument: basicTypeString: no global variable named ' .. tostring(vts))
    local instance = {
        name = basicTypeString,
        vtName = vts,
        vType = vt_global,
        size = vt.size[vt_global],
        readUnsafe = vt.read[vt_global],
        writeUnsafe = vt.write[vt_global],
    }
    setmetatable(instance,
        {
            __index = self,
            __call = function(helper, value)
                return helper:asInvokeArgument({ value = value })
            end,
            __name = 'VTypeHelper: ' .. basicTypeString,
        })
    return instance
end,
})

--- Returns the monotypes associated with the VType.
---@param self VTypeHelper
---@return MonoType[]
vt.VTypeHelper.getMonotypes = function(self)
    return alce.utils.keysFromValue({ value = self.vType, tbl = monoTypeToVartypeLookup })
end

--- Returns a sorted array of strings representing the monotypes.
---@overload fun(self: VTypeHelper): string[]|nil
vt.VTypeHelper.getMonotypesAsStrings = fn.method({
    schema = Schema.empty(),
    body = function(self)
    local keys = alce.utils.keysFromValue({ value = self.vType, tbl = monoTypeToVartypeLookup })
    local r = {}
    for _, v in ipairs(keys) do table.insert(r, alce.monoscript.monotype.nameLookup[v]) end
    table.sort(r)
    return next(r) and r or nil
end,
})

--- Formats the VType and a value for invoking methods.
---@overload fun(self: VTypeHelper, args: { value: any }): { type: number, value: any }
vt.VTypeHelper.asInvokeArgument = fn.method({
    params = { value = Param.required() },
    body = function(self, args)
    local value = args.value
    return { type = self.vType, value = value }
end,
})

--- Reads a value from the specified address using the VType.
---@overload fun(self: VTypeHelper, args: { address: number }): any
vt.VTypeHelper.read = fn.method({
    params = { address = Param.required():validate(validators.isAddresslike) },
    body = function(self, args)
    local address = args.address
    assert(self.readUnsafe, 'alce.T.VType.read(): no read function for type ' .. self.name)
    return self.readUnsafe(address)
end,
})

--- Writes a value to the specified address using the VType.
---@overload fun(self: VTypeHelper, args: { address: number, value: any }): boolean
vt.VTypeHelper.write = fn.method({
    params = {
        address = Param.required():validate(validators.isAddresslike),
        value = Param.required(),
    },
    body = function(self, args)
    local address = args.address
    local value = args.value
    assert(self.readUnsafe, 'alce.T.VType.write(): no read function for type ' .. self.name)
    return self.writeUnsafe(address, value)
end,
})

return vt
