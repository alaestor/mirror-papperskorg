local aa = require("ct.aa")
local source = require("ct.source")

---@class CtXmlWriter
---@field lines string[]
local Writer = {}
Writer.__index = Writer

---@return CtXmlWriter
local function new_writer()
  return setmetatable({ lines = {} }, Writer)
end

---@param codepoint integer
---@return boolean
local function legal_xml_codepoint(codepoint)
  return codepoint == 0x09
    or codepoint == 0x0A
    or codepoint == 0x0D
    or (codepoint >= 0x20 and codepoint <= 0xD7FF)
    or (codepoint >= 0xE000 and codepoint <= 0xFFFD)
    or (codepoint >= 0x10000 and codepoint <= 0x10FFFF)
end

---@param value string
---@param label string
---@return string
local function normalize_and_validate(value, label)
  value = value:gsub("\r\n", "\n"):gsub("\r", "\n")
  local ok, invalid = pcall(function()
    for _, codepoint in utf8.codes(value) do
      if not legal_xml_codepoint(codepoint) then
        return codepoint
      end
    end
  end)
  if not ok then
    error(label .. ": invalid UTF-8", 0)
  end
  if invalid ~= nil then
    error(string.format("%s: character U+%04X is not legal in XML 1.0", label, invalid), 0)
  end
  return value
end

---@param value unknown
---@return string
local function escape_text(value)
  return (tostring(value)
    :gsub("&", "&amp;")
    :gsub("<", "&lt;")
    :gsub(">", "&gt;"))
end

---@param value unknown
---@return string
local function escape_attribute(value)
  return (escape_text(value)
    :gsub('"', "&quot;")
    :gsub("'", "&apos;"))
end

---@param attributes { [1]: string, [2]: unknown }[]|nil
---@return string
local function render_attributes(attributes)
  local output = {}
  for _, attribute in ipairs(attributes or {}) do
    if attribute[2] ~= nil then
      output[#output + 1] = string.format(' %s="%s"', attribute[1], escape_attribute(attribute[2]))
    end
  end
  return table.concat(output)
end

---@param depth integer
---@param value string
function Writer:line(depth, value)
  self.lines[#self.lines + 1] = string.rep("  ", depth) .. value
end

---@param depth integer
---@param name string
---@param value unknown
---@param attributes? { [1]: string, [2]: unknown }[]
function Writer:element(depth, name, value, attributes)
  self:line(depth, string.format(
    "<%s%s>%s</%s>",
    name,
    render_attributes(attributes),
    escape_text(value),
    name
  ))
end

---@param depth integer
---@param name string
---@param attributes? { [1]: string, [2]: unknown }[]
function Writer:empty(depth, name, attributes)
  self:line(depth, string.format("<%s%s/>", name, render_attributes(attributes)))
end

local value_type_names = {
  binary = "Binary",
  byte = "Byte",
  ["2_bytes"] = "2 Bytes",
  ["4_bytes"] = "4 Bytes",
  ["8_bytes"] = "8 Bytes",
  float = "Float",
  double = "Double",
  string = "String",
  byte_array = "Array of byte",
}

local option_names = {
  { "hide_children", "moHideChildren" },
  { "default_collapsed", "moDefaultCollapsed" },
  { "recursive_set_value", "moRecursiveSetValue" },
  { "manual_expand_collapse", "moManualExpandCollapse" },
  { "activate_children", "moActivateChildrenAsWell" },
  { "deactivate_children", "moDeactivateChildrenAsWell" },
  { "allow_manual_collapse", "moAllowManualCollapseAndExpand" },
  { "always_hide_children", "moAlwaysHideChildren" },
}

---@param writer CtXmlWriter
---@param options CtEntryOptions|nil
---@param depth integer
local function render_options(writer, options, depth)
  if options == nil then
    return
  end
  local attributes = {}
  for _, option in ipairs(option_names) do
    if options[option[1]] then
      attributes[#attributes + 1] = { option[2], "1" }
    end
  end
  if #attributes > 0 then
    writer:empty(depth, "Options", attributes)
  end
end

local action_names = {
  toggle = "Toggle Activation",
  toggle_allow_increase = "Toggle Activation Allow Increase",
  toggle_allow_decrease = "Toggle Activation Allow Decrease",
  activate = "Activate",
  deactivate = "Deactivate",
  set_value = "Set Value",
  decrease_value = "Decrease Value",
  increase_value = "Increase Value",
}

---@param value string|CtSound
---@return string, string|nil
local function sound_parts(value)
  if type(value) == "string" then
    return value, nil
  end
  return value.text, value.tts
end

---@param writer CtXmlWriter
---@param hotkeys CtHotkey[]
---@param allocation CtIdAllocation
---@param depth integer
local function render_hotkeys(writer, hotkeys, allocation, depth)
  if #hotkeys == 0 then
    return
  end
  writer:line(depth, "<Hotkeys>")
  for _, hotkey in ipairs(hotkeys) do
    local attributes = {}
    if hotkey.only_while_down then
      attributes[#attributes + 1] = { "OnlyWhileDown", "1" }
    end
    writer:line(depth + 1, "<Hotkey" .. render_attributes(attributes) .. ">")
    writer:element(depth + 2, "Action", action_names[hotkey.action])
    writer:line(depth + 2, "<Keys>")
    for _, key in ipairs(hotkey.keys) do
      writer:element(depth + 3, "Key", key)
    end
    writer:line(depth + 2, "</Keys>")
    if hotkey.value ~= nil then
      writer:element(depth + 2, "Value", hotkey.value)
    end
    if hotkey.description ~= nil then
      writer:element(depth + 2, "Description", hotkey.description)
    end
    writer:element(depth + 2, "ID", allocation.hotkeys[hotkey])
    for _, sound_key in ipairs({ "activate_sound", "deactivate_sound" }) do
      local sound = hotkey[sound_key]
      if sound ~= nil then
        local text, tts = sound_parts(sound)
        local element_name = sound_key == "activate_sound" and "ActivateSound" or "DeactivateSound"
        local sound_attributes = {}
        if tts then
          sound_attributes[#sound_attributes + 1] = { "TTS", tts }
        end
        writer:element(depth + 2, element_name, text, sound_attributes)
      end
    end
    writer:line(depth + 1, "</Hotkey>")
  end
  writer:line(depth, "</Hotkeys>")
end

---@param writer CtXmlWriter
---@param dropdown CtDropdown
---@param depth integer
local function render_dropdown(writer, dropdown, depth)
  local attributes = {}
  if dropdown.read_only then
    attributes[#attributes + 1] = { "ReadOnly", "1" }
  end
  if dropdown.description_only then
    attributes[#attributes + 1] = { "DescriptionOnly", "1" }
  end
  if dropdown.display_value_as_item then
    attributes[#attributes + 1] = { "DisplayValueAsItem", "1" }
  end
  local lines = {}
  for _, item in ipairs(dropdown.items) do
    local line = tostring(item.value)
    if item.description ~= nil then
      line = line .. ":" .. item.description
    end
    lines[#lines + 1] = line
  end
  local content = table.concat(lines, "\n")
  if #lines > 0 then
    content = content .. "\n"
  end
  writer:element(depth, "DropDownList", content, attributes)
end

---@param writer CtXmlWriter
---@param entry CtEntry
---@param allocation CtIdAllocation
---@param base_dir string
---@param emit_defaults boolean
---@param depth integer
local function render_entry(writer, entry, allocation, base_dir, emit_defaults, depth)
  local attributes = {}
  if entry.no_checkbox then
    attributes[#attributes + 1] = { "NoCheckbox", "1" }
  end
  if entry.value_on_activate ~= nil then
    attributes[#attributes + 1] = { "ValueOnActivate", entry.value_on_activate }
  end
  if entry.value_on_deactivate ~= nil then
    attributes[#attributes + 1] = { "ValueOnDeactivate", entry.value_on_deactivate }
  end
  if entry.restore_on_deactivate then
    attributes[#attributes + 1] = { "RestoreOnDeactivate", "1" }
  end
  if entry.save_value then
    attributes[#attributes + 1] = { "SaveValueOnTableStateSave", "1" }
  end

  writer:line(depth, "<CheatEntry" .. render_attributes(attributes) .. ">")
  writer:element(depth + 1, "ID", allocation.entries[entry])
  writer:element(depth + 1, "Description", '"' .. entry.description .. '"')
  if entry.color then
    writer:element(depth + 1, "Color", entry.color)
  end
  render_options(writer, entry.options, depth + 1)

  if entry.kind == "aa" then
    writer:element(depth + 1, "VariableType", "Auto Assembler Script")
    local script
    if entry.script then
      script = source.resolve(entry.script, base_dir)
    else
      script = aa.compose(entry.sections, base_dir)
    end
    script = normalize_and_validate(script, "AssemblerScript")
    local script_attributes = entry.async and { { "Async", "1" } } or nil
    writer:element(depth + 1, "AssemblerScript", script, script_attributes)
  elseif entry.kind == "group" then
    writer:element(depth + 1, "GroupHeader", "1")
    if entry.address ~= nil then
      if entry.address == "" then
        writer:empty(depth + 1, "Address")
      else
        writer:element(depth + 1, "Address", entry.address)
      end
    end
  else
    if entry.dropdown then
      render_dropdown(writer, entry.dropdown, depth + 1)
    end
    if entry.show_as_hex then
      writer:element(depth + 1, "ShowAsHex", "1")
    end
    if emit_defaults or entry.show_as_signed then
      writer:element(depth + 1, "ShowAsSigned", entry.show_as_signed and "1" or "0")
    end
    writer:element(depth + 1, "VariableType", value_type_names[entry.value_type])
    if entry.value_type == "binary" then
      if emit_defaults or entry.bit_start ~= 0 then
        writer:element(depth + 1, "BitStart", entry.bit_start)
      end
      writer:element(depth + 1, "BitLength", entry.bit_length)
      if emit_defaults or entry.show_as_binary then
        writer:element(depth + 1, "ShowAsBinary", entry.show_as_binary and "1" or "0")
      end
    elseif entry.value_type == "string" then
      writer:element(depth + 1, "Length", entry.length)
      if emit_defaults or entry.unicode then
        writer:element(depth + 1, "Unicode", entry.unicode and "1" or "0")
      end
      if emit_defaults or entry.code_page ~= 0 then
        writer:element(depth + 1, "CodePage", entry.code_page)
      end
      if emit_defaults or not entry.zero_terminate then
        writer:element(depth + 1, "ZeroTerminate", entry.zero_terminate and "1" or "0")
      end
    elseif entry.value_type == "byte_array" then
      writer:element(depth + 1, "ByteLength", entry.byte_length)
    end

    if entry.address == "" then
      if emit_defaults then
        writer:empty(depth + 1, "Address")
      end
    else
      writer:element(depth + 1, "Address", entry.address)
    end
    if #entry.offsets > 0 then
      writer:line(depth + 1, "<Offsets>")
      for _, offset in ipairs(entry.offsets) do
        writer:element(depth + 2, "Offset", offset)
      end
      writer:line(depth + 1, "</Offsets>")
    end
  end

  render_hotkeys(writer, entry.hotkeys, allocation, depth + 1)
  if #entry.children > 0 then
    writer:line(depth + 1, "<CheatEntries>")
    for _, child in ipairs(entry.children) do
      render_entry(writer, child, allocation, base_dir, emit_defaults, depth + 2)
    end
    writer:line(depth + 1, "</CheatEntries>")
  end
  writer:line(depth, "</CheatEntry>")
end

---@param definition CtTable
---@param allocation CtIdAllocation
---@param base_dir string
---@return string
local function render(definition, allocation, base_dir)
  local writer = new_writer()
  writer.lines[#writer.lines + 1] = '<?xml version="1.0" encoding="utf-8"?>'
  local root_attributes = {}
  if definition.uses_mono then
    root_attributes[#root_attributes + 1] = { "UsesMono", "1" }
  end
  if definition.auto_attach ~= nil then
    root_attributes[#root_attributes + 1] = { "AutoAttach", definition.auto_attach }
  end
  if definition.show_script_errors then
    root_attributes[#root_attributes + 1] = { "ShowScriptErrors", "1" }
  end
  if definition.read_only_address_list then
    root_attributes[#root_attributes + 1] = { "ReadOnlyAddressList", "1" }
  end
  if definition.full_client_address_list then
    root_attributes[#root_attributes + 1] = { "FullClientAddressList", "1" }
  end
  root_attributes[#root_attributes + 1] = { "CheatEngineTableVersion", definition.version }
  writer:line(0, "<CheatTable" .. render_attributes(root_attributes) .. ">")

  if definition.custom_script_error_message ~= nil then
    local text = source.resolve(definition.custom_script_error_message, base_dir)
    writer:element(1, "CustomScriptErrorMessage", normalize_and_validate(text, "CustomScriptErrorMessage"))
  end
  if #definition.entries == 0 then
    writer:empty(1, "CheatEntries")
  else
    writer:line(1, "<CheatEntries>")
    for _, entry in ipairs(definition.entries) do
      render_entry(writer, entry, allocation, base_dir, definition.emit_defaults, 2)
    end
    writer:line(1, "</CheatEntries>")
  end
  writer:empty(1, "UserdefinedSymbols")
  if definition.comments ~= nil then
    local text = source.resolve(definition.comments, base_dir)
    writer:element(1, "Comments", normalize_and_validate(text, "Comments"))
  end
  if definition.lua_script ~= nil then
    local text = source.resolve(definition.lua_script, base_dir)
    writer:element(1, "LuaScript", normalize_and_validate(text, "LuaScript"))
  end
  writer:line(0, "</CheatTable>")
  return table.concat(writer.lines, "\n") .. "\n"
end

return {
  render = render,
}
