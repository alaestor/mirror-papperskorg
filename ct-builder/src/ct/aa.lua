local source = require("ct.source")

---@class CtAa
local aa = {}

---@param value string
---@return string
local function normalize(value)
  return (value:gsub("\r\n", "\n"):gsub("\r", "\n"):gsub("\n+$", ""))
end

---@param blocks string[]
---@param value string
local function append(blocks, value)
  value = normalize(value)
  if value ~= "" then
    blocks[#blocks + 1] = value
  end
end

---@param blocks string[]
---@param stage CtAaStage|nil
---@param base_dir string
local function append_stage(blocks, stage, base_dir)
  if not stage then
    return
  end
  if stage.lua then
    local lua_source = normalize(source.resolve(stage.lua, base_dir))
    if lua_source ~= "" then
      append(blocks, "{$lua}\n" .. lua_source .. "\n{$asm}")
    end
  end
  if stage.asm then
    append(blocks, source.resolve(stage.asm, base_dir))
  end
end

---@param sections CtAaSections
---@param base_dir string
---@return string
function aa.compose(sections, base_dir)
  local blocks = {}
  if sections.comment then
    local comment = normalize(source.resolve(sections.comment, base_dir))
    if comment ~= "" then
      append(blocks, "{\n" .. comment .. "\n}")
    end
  end

  append_stage(blocks, sections.header, base_dir)
  blocks[#blocks + 1] = "[ENABLE]"
  append_stage(blocks, sections.enable, base_dir)
  blocks[#blocks + 1] = "[DISABLE]"
  append_stage(blocks, sections.disable, base_dir)
  return table.concat(blocks, "\n\n") .. "\n"
end

return aa
