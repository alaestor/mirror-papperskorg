local validators = require("validators")
local alce = require("globals")
local vt = require("vt")
local fnlua = require("fn")
local fn = fnlua.wrap
local Param = fnlua.Param

--- Lookup table for `VTypeHelper` instances by CE vartype ID, basic type string, or vartype string.
---@class T
--[[
Index by a CE vartype (`vtDword`), basic type string (`"dword"`), or vartype
name (`"vtDword"`) to get a `VTypeHelper` for type-aware reads, writes, and
invoke arguments.
]]
local T = {}

-- Populate T with VTypeHelpers
for _, v in ipairs(vt.basicTypeStrings) do
    local t = vt.VTypeHelper:new({ basicTypeString = v })
    for _, lookupKey in pairs({ t.name, t.vtName, t.vType }) do
        assert(not T[lookupKey], 'alce.T: key already exists: ' .. tostring(lookupKey))
        T[lookupKey] = t
    end
end

-- Calculate the maximum monotype key and store it in config
local max_key = -math.huge
if monoTypeToVartypeLookup then
    for k, _ in pairs(monoTypeToVartypeLookup) do
        if type(k) == 'number' and k > max_key then
            max_key = k
        end
    end
end
alce.cfg.monotype_max_key = max_key

-- Export T back to the global alce table for backward compatibility and general access
alce.T = T

---Returns a VTypeHelper from a monoType, with a warning if it exceeds the lookup key limit.
---@overload fun(args: { monoType: number }): VTypeHelper
T.unsafeFromMono = fn({
    params = { monoType = Param.required():validate(validators.isInteger) },
    body = function(args)
    local monoType = args.monoType
    if monoType > alce.cfg.monotype_max_key then
        alce.warn('alce.T.unsafeFromMono(): monoType (' ..
            tostring(monoType) ..
            ') is bigger than the largest monoTypeToVartypeLookup key. Monoscript defaults to vtDword.')
    end
    return alce.T[monoTypeToVartypeLookup[monoType]]
end,
})

---Returns a VTypeHelper from a monoType with a bounds-checking assertion.
---@overload fun(args: { monoType: number }): VTypeHelper
T.fromMono = fn({
    params = { monoType = Param.required():validate(validators.isInteger) },
    body = function(args)
    local monoType = args.monoType
    assert(monoType <= alce.cfg.monotype_max_key,
        'alce.T.fromMono(): monoType (' ..
        tostring(monoType) .. ') is bigger than the largest monoTypeToVartypeLookup key.')
    return alce.T[monoTypeToVartypeLookup[monoType]]
end,
})

return T
