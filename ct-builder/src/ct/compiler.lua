local ids = require("ct.ids")
local private = require("ct.private")
local xml = require("ct.xml")

---@class CtCompileOptions
---@field base_dir? string

---@class CtCompiler
local compiler = {}

---@param definition unknown
---@param options? CtCompileOptions
---@return string
function compiler.compile(definition, options)
  if type(definition) ~= "table" or definition[private.root_marker] ~= true then
    error("definition must return ct.table { ... }", 0)
  end
  options = options or {}
  local allocation = ids.allocate(definition.entries)
  return xml.render(definition, allocation, options.base_dir or ".")
end

return compiler
