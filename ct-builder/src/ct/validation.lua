---@class CtValidation
local validation = {}

---@param value unknown
---@return boolean
function validation.is_string(value)
  return type(value) == "string"
end

---@param value unknown
---@return boolean
function validation.is_boolean(value)
  return type(value) == "boolean"
end

---@param value unknown
---@return boolean
function validation.is_number(value)
  return type(value) == "number"
end

---@param value unknown
---@return boolean
function validation.is_integer(value)
  return type(value) == "number" and value % 1 == 0
end

---@param value unknown
---@return boolean
function validation.is_non_negative_integer(value)
  return validation.is_integer(value) and value >= 0
end

---@param value unknown
---@return boolean
function validation.is_positive_integer(value)
  return validation.is_integer(value) and value > 0
end

---@param value unknown
---@return boolean
function validation.is_table(value)
  return type(value) == "table"
end

---@param value unknown
---@return boolean
function validation.is_scalar(value)
  local value_type = type(value)
  return value_type == "string" or value_type == "number"
end

---@param value unknown
---@param label string
---@return table
function validation.array(value, label)
  if type(value) ~= "table" then
    error(label .. ": expected an array", 3)
  end

  local count = 0
  for key in pairs(value) do
    if not validation.is_positive_integer(key) then
      error(label .. ": expected an array", 3)
    end
    count = count + 1
  end
  if count ~= #value then
    error(label .. ": array contains holes", 3)
  end
  return value
end

---@param value table
---@param allowed table<string, boolean>
---@param label string
function validation.known_keys(value, allowed, label)
  for key in pairs(value) do
    if not allowed[key] then
      error(string.format("%s: unknown key: %s", label, tostring(key)), 3)
    end
  end
end

---@param value unknown
---@param label string
---@return string
function validation.string(value, label)
  if type(value) ~= "string" then
    error(label .. ": expected a string", 3)
  end
  return value
end

---@param value unknown
---@param label string
---@return boolean
function validation.boolean(value, label)
  if type(value) ~= "boolean" then
    error(label .. ": expected a boolean", 3)
  end
  return value
end

---@param value unknown
---@param label string
---@return integer
function validation.non_negative_integer(value, label)
  if not validation.is_non_negative_integer(value) then
    error(label .. ": expected a non-negative integer", 3)
  end
  return value
end

return validation
