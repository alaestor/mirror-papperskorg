local alce = require("globals")

local validators = {}

---Checks if a value is a boolean
function validators.isBoolean(value)
    return type(value) == "boolean"
end

---Checks if a value is an integer.
function validators.isInteger(value)
    return type(value) == 'number' and math.type(value) == 'integer'
end

---Checks if a value is a positive integer.
function validators.isPositiveInteger(value)
    return type(value) == 'number' and math.type(value) == 'integer' and value > 0
end

---Checks if a value is a non-negative integer.
function validators.isNonNegativeInteger(value)
    return type(value) == 'number' and math.type(value) == 'integer' and value >= 0
end

---Checks if a value is a float.
function validators.isFloat(value)
    return type(value) == 'number' and math.type(value) == 'float'
end

---Checks if a value is a non-negative float.
function validators.isNonNegativeFloat(value)
    return type(value) == 'number' and math.type(value) == 'float' and value >= 0.0
end

---Checks if a value is a finite number.
function validators.isFiniteNumber(value)
    return type(value) == 'number' and value == value and value ~= math.huge and value ~= -math.huge
end

---Checks if a value is a non-empty string.
function validators.isNonEmptyString(value)
    return type(value) == 'string' and value ~= ''
end

---Checks if a value is a non-blank string.
function validators.isNonBlankString(value)
    return type(value) == 'string' and string.find(value, '%S') ~= nil
end

---Checks if a value is a table.
function validators.isTable(value)
    return type(value) == 'table'
end

---Checks if a value is an empty table.
function validators.isEmptyTable(value)
    return type(value) == 'table' and next(value) == nil
end

---Checks if a value is a non-empty table.
function validators.isNonEmptyTable(value)
    return type(value) == 'table' and next(value) ~= nil
end

---Checks if a value is zero, empty table, blank string, or nil.
function validators.isZeroEmptyOrNil(value)
    local t = type(value)
    return t == nil or (t == 'number' and value == 0) or validators.isEmptyTable(value) or
        (not validators.isNonBlankString(value))
end

---Checks if a value is callable.
function validators.isCallable(value)
    if type(value) == 'function' then return true end
    local mt = getmetatable(value)
    return mt ~= nil and type(mt.__call) == 'function'
end

---Checks if a value is between a minimum and maximum.
---@param value any
---@param minimum number
---@param maximum number
---@return boolean
function validators.isBetween(value, minimum, maximum)
    return value > minimum and value < maximum
end

---Checks that a value is an integer within a signed boundary.
---@param value any
---@param tooFarBoundary? number
---@return boolean
function validators.isSignedOffsetlike(value, tooFarBoundary)
    local boundary = (tooFarBoundary or alce.cfg.isOffset_tooFarBoundary)
    return validators.isInteger(value) and value > -boundary and value < boundary
end

---Checks that a value is a non-negative integer below a boundary.
---@param value any
---@param tooFarBoundary? number
---@return boolean
function validators.isOffsetlike(value, tooFarBoundary)
    return validators.isNonNegativeInteger(value) and value < (tooFarBoundary or alce.cfg.isOffset_tooFarBoundary)
end

---Checks that a value is an integer within valid address bounds.
---@param value any
---@param nearNullBoundary? number
---@param userspaceBoundary? number
---@return boolean
function validators.isAddresslike(value, nearNullBoundary, userspaceBoundary)
    return validators.isInteger(value) and value > (nearNullBoundary or alce.cfg.isAddress_nearNullBoundary) and
        value <
        (userspaceBoundary or (targetIs64Bit() and alce.cfg.isAddress_userspaceBoundary64 or alce.cfg.isAddress_userspaceBoundary32))
end

---Checks if a flag bit is set in a flags value.
---@param flag number
---@param flags number
---@return boolean
function validators.hasFlag(flag, flags)
    return (flags & flag) == flag
end

---Asserts a value is truthy, optionally via a checker function.
---@param value any
---@param checker? function
---@return any
function validators.check(value, checker)
    local info = debug.getinfo(2, "Sl")
    assert(checker == nil or validators.isCallable(checker),
        'line ' .. tostring(info.currentline) .. ': alce.check: invalid argument: checker not callable.')
    assert(checker and checker(value) or value,
        'line ' .. tostring(info.currentline) .. ': alce.check( ' .. tostring(value) .. ' )')
    return value
end

---Asserts a value is falsy, optionally via a checker function.
---@param value any
---@param checker? function
---@return any
function validators.ncheck(value, checker)
    local info = debug.getinfo(2, "Sl")
    assert(checker == nil or validators.isCallable(checker),
        'line ' .. tostring(info.currentline) .. ': alce.check: invalid argument: checker not callable.')
    assert(not (checker and checker(value) or value),
        'line ' .. tostring(info.currentline) .. ': alce.ncheck( ' .. tostring(value) .. ' )')
    return value
end

return validators
