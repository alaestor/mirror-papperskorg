local private = require("ct.private")

---@class CtSourceRef
---@field path string
local SourceRef = {}
SourceRef.__index = SourceRef

---@class CtSource
local source = {}

---@param path string
---@return CtSourceRef
function source.new(path)
  return setmetatable({
    [private.source_marker] = true,
    path = path,
  }, SourceRef)
end

---@param value unknown
---@return boolean
function source.is(value)
  return type(value) == "table" and value[private.source_marker] == true
end

---@param value unknown
---@return boolean
function source.is_text(value)
  return type(value) == "string" or source.is(value)
end

---@param path string
---@return string
local function read_file(path)
  local handle, open_error = io.open(path, "rb")
  if not handle then
    error(string.format("cannot read source %q: %s", path, open_error), 0)
  end
  local content, read_error = handle:read("*a")
  handle:close()
  if not content then
    error(string.format("cannot read source %q: %s", path, read_error), 0)
  end
  return content
end

---@param base_dir string
---@param path string
---@return string
local function join(base_dir, path)
  if path:sub(1, 1) == "/" then
    return path
  end
  if base_dir == "." or base_dir == "" then
    return path
  end
  return base_dir:gsub("/+$", "") .. "/" .. path
end

---@param value string|CtSourceRef
---@param base_dir string
---@return string
function source.resolve(value, base_dir)
  if type(value) == "string" then
    return value
  end
  if not source.is(value) then
    error("expected text or ct.file(...) source", 0)
  end
  return read_file(join(base_dir, value.path))
end

return source
