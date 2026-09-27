local fnlua = require("fn")
local private = require("ct.private")
local source = require("ct.source")
local validation = require("ct.validation")

local Param = fnlua.Param
local wrap = fnlua.wrap

---@alias CtText string|CtSourceRef
---@alias CtValueType "binary"|"byte"|"2_bytes"|"4_bytes"|"8_bytes"|"float"|"double"|"string"|"byte_array"

---@class CtAaStage
---@field lua? CtText
---@field asm? CtText

---@class CtAaSections
---@field comment? CtText
---@field header? CtAaStage
---@field enable? CtAaStage
---@field disable? CtAaStage

---@class CtEntryOptions
---@field hide_children? boolean
---@field default_collapsed? boolean
---@field recursive_set_value? boolean
---@field manual_expand_collapse? boolean
---@field activate_children? boolean
---@field deactivate_children? boolean
---@field allow_manual_collapse? boolean
---@field always_hide_children? boolean

---@class CtSound
---@field text string
---@field tts? string

---@class CtHotkey
---@field id? integer
---@field action string
---@field keys integer[]
---@field description? string
---@field only_while_down? boolean
---@field value? string|number
---@field activate_sound? string|CtSound
---@field deactivate_sound? string|CtSound

---@class CtDropdownItem
---@field value string|number
---@field description? string

---@class CtDropdown
---@field items CtDropdownItem[]
---@field read_only? boolean
---@field description_only? boolean
---@field display_value_as_item? boolean

---@class CtEntryCommon
---@field id? integer
---@field description string
---@field color? string
---@field no_checkbox? boolean
---@field save_value? boolean
---@field value_on_activate? string|number
---@field value_on_deactivate? string|number
---@field restore_on_deactivate? boolean
---@field options? CtEntryOptions
---@field hotkeys? CtHotkey[]
---@field children? CtEntry[]

---@class CtGroupArgs: CtEntryCommon
---@field address? string

---@class CtValueArgs: CtEntryCommon
---@field value_type CtValueType
---@field address? string
---@field offsets? string[]
---@field show_as_signed? boolean
---@field show_as_hex? boolean
---@field bit_start? integer
---@field bit_length? integer
---@field show_as_binary? boolean
---@field length? integer
---@field unicode? boolean
---@field code_page? integer
---@field zero_terminate? boolean
---@field byte_length? integer
---@field dropdown? CtDropdown

---@class CtAaArgs: CtEntryCommon
---@field script? CtText
---@field sections? CtAaSections
---@field async? boolean

---@class CtTableArgs
---@field version? integer
---@field emit_defaults? boolean
---@field uses_mono? boolean
---@field auto_attach? string
---@field show_script_errors? boolean
---@field read_only_address_list? boolean
---@field full_client_address_list? boolean
---@field custom_script_error_message? CtText
---@field comments? CtText
---@field lua_script? CtText
---@field entries? CtEntry[]

---@class CtEntry
---@field kind "group"|"value"|"aa"
---@field id? integer
---@field description string
---@field color? string
---@field no_checkbox? boolean
---@field save_value? boolean
---@field value_on_activate? string|number
---@field value_on_deactivate? string|number
---@field restore_on_deactivate? boolean
---@field options? CtEntryOptions
---@field hotkeys CtHotkey[]
---@field children CtEntry[]
---@field address? string
---@field script? CtText
---@field sections? CtAaSections
---@field async? boolean
---@field value_type? CtValueType
---@field offsets string[]
---@field show_as_signed? boolean
---@field show_as_hex? boolean
---@field bit_start? integer
---@field bit_length? integer
---@field show_as_binary? boolean
---@field length? integer
---@field unicode? boolean
---@field code_page? integer
---@field zero_terminate? boolean
---@field byte_length? integer
---@field dropdown? CtDropdown

---@class CtTable
---@field version integer
---@field emit_defaults boolean
---@field uses_mono? boolean
---@field auto_attach? string
---@field show_script_errors? boolean
---@field read_only_address_list? boolean
---@field full_client_address_list? boolean
---@field custom_script_error_message? CtText
---@field comments? CtText
---@field lua_script? CtText
---@field entries CtEntry[]

local ct = {
  types = {
    binary = "binary",
    byte = "byte",
    int16 = "2_bytes",
    int32 = "4_bytes",
    int64 = "8_bytes",
    float = "float",
    double = "double",
    string = "string",
    byte_array = "byte_array",
  },
}

local text_param = Param.optional():validate(source.is_text)
local boolean_param = Param.optional():validate(validation.is_boolean)
local scalar_param = Param.optional():validate(validation.is_scalar)
local id_param = Param.optional():validate(validation.is_non_negative_integer)
local table_param = Param.optional():validate(validation.is_table)
local color_param = Param.optional():validate(function(value)
  return type(value) == "string" and value:match("^%x%x%x%x%x%x$") ~= nil
end)

---@param value unknown
---@param label string
local function validate_text(value, label)
  if value ~= nil and not source.is_text(value) then
    error(label .. ": expected a string or ct.file(...)", 3)
  end
end

---@param value unknown
---@param label string
local function validate_boolean(value, label)
  if value ~= nil and type(value) ~= "boolean" then
    error(label .. ": expected a boolean", 3)
  end
end

---@param children unknown
---@param label string
---@return CtEntry[]
local function validate_children(children, label)
  if children == nil then
    return {}
  end
  validation.array(children, label)
  for index, child in ipairs(children) do
    if type(child) ~= "table" or child[private.node_marker] == nil then
      error(string.format("%s[%d]: expected a ct entry constructor result", label, index), 3)
    end
  end
  return children
end

local option_keys = {
  hide_children = true,
  default_collapsed = true,
  recursive_set_value = true,
  manual_expand_collapse = true,
  activate_children = true,
  deactivate_children = true,
  allow_manual_collapse = true,
  always_hide_children = true,
}

---@param options CtEntryOptions|nil
local function validate_options(options)
  if options == nil then
    return
  end
  validation.known_keys(options, option_keys, "options")
  for key, value in pairs(options) do
    validate_boolean(value, "options." .. key)
  end
  if options.always_hide_children and (
      options.hide_children
      or options.default_collapsed
      or options.manual_expand_collapse
      or options.allow_manual_collapse
    )
  then
    error("options.always_hide_children conflicts with manual hide/collapse options", 3)
  end
end

local sound_keys = { text = true, tts = true }

---@param value string|CtSound|nil
---@param label string
local function validate_sound(value, label)
  if value == nil or type(value) == "string" then
    return
  end
  if type(value) ~= "table" then
    error(label .. ": expected a string or sound table", 3)
  end
  validation.known_keys(value, sound_keys, label)
  validation.string(value.text, label .. ".text")
  if value.tts ~= nil then
    validation.string(value.tts, label .. ".tts")
  end
end

local hotkey_keys = {
  id = true,
  action = true,
  keys = true,
  description = true,
  only_while_down = true,
  value = true,
  activate_sound = true,
  deactivate_sound = true,
}

local hotkey_actions = {
  toggle = true,
  toggle_allow_increase = true,
  toggle_allow_decrease = true,
  activate = true,
  deactivate = true,
  set_value = true,
  decrease_value = true,
  increase_value = true,
}

---@param hotkeys CtHotkey[]|nil
---@return CtHotkey[]
local function validate_hotkeys(hotkeys)
  if hotkeys == nil then
    return {}
  end
  validation.array(hotkeys, "hotkeys")
  for index, hotkey in ipairs(hotkeys) do
    local label = string.format("hotkeys[%d]", index)
    if type(hotkey) ~= "table" then
      error(label .. ": expected a table", 3)
    end
    validation.known_keys(hotkey, hotkey_keys, label)
    if hotkey.id ~= nil then
      validation.non_negative_integer(hotkey.id, label .. ".id")
    end
    validation.string(hotkey.action, label .. ".action")
    if not hotkey_actions[hotkey.action] then
      error(label .. ".action: unsupported action: " .. hotkey.action, 3)
    end
    validation.array(hotkey.keys, label .. ".keys")
    if #hotkey.keys == 0 then
      error(label .. ".keys: expected at least one key", 3)
    end
    for key_index, key in ipairs(hotkey.keys) do
      validation.non_negative_integer(key, string.format("%s.keys[%d]", label, key_index))
    end
    if hotkey.description ~= nil then
      validation.string(hotkey.description, label .. ".description")
    end
    validate_boolean(hotkey.only_while_down, label .. ".only_while_down")
    if hotkey.value ~= nil and not validation.is_scalar(hotkey.value) then
      error(label .. ".value: expected a string or number", 3)
    end
    local value_action = hotkey.action == "set_value"
      or hotkey.action == "decrease_value"
      or hotkey.action == "increase_value"
    if value_action and hotkey.value == nil then
      error(label .. ".value: required for value actions", 3)
    end
    validate_sound(hotkey.activate_sound, label .. ".activate_sound")
    validate_sound(hotkey.deactivate_sound, label .. ".deactivate_sound")
  end
  return hotkeys
end

local dropdown_keys = {
  items = true,
  read_only = true,
  description_only = true,
  display_value_as_item = true,
}
local dropdown_item_keys = { value = true, description = true }

---@param dropdown CtDropdown|nil
local function validate_dropdown(dropdown)
  if dropdown == nil then
    return
  end
  validation.known_keys(dropdown, dropdown_keys, "dropdown")
  validation.array(dropdown.items, "dropdown.items")
  for index, item in ipairs(dropdown.items) do
    local label = string.format("dropdown.items[%d]", index)
    if type(item) ~= "table" then
      error(label .. ": expected a table", 3)
    end
    validation.known_keys(item, dropdown_item_keys, label)
    if not validation.is_scalar(item.value) then
      error(label .. ".value: expected a string or number", 3)
    end
    local value = tostring(item.value)
    if value:find("[\r\n]") then
      error(label .. ".value: newlines are not supported", 3)
    end
    if item.description ~= nil then
      validation.string(item.description, label .. ".description")
      if item.description:find("[\r\n]") then
        error(label .. ".description: newlines are not supported", 3)
      end
    end
  end
  validate_boolean(dropdown.read_only, "dropdown.read_only")
  validate_boolean(dropdown.description_only, "dropdown.description_only")
  validate_boolean(dropdown.display_value_as_item, "dropdown.display_value_as_item")
end

local stage_keys = { lua = true, asm = true }
local section_keys = { comment = true, header = true, enable = true, disable = true }

---@param sections CtAaSections|nil
local function validate_sections(sections)
  if sections == nil then
    return
  end
  validation.known_keys(sections, section_keys, "sections")
  validate_text(sections.comment, "sections.comment")
  for _, stage_name in ipairs({ "header", "enable", "disable" }) do
    local stage = sections[stage_name]
    if stage ~= nil then
      if type(stage) ~= "table" then
        error("sections." .. stage_name .. ": expected a table", 3)
      end
      validation.known_keys(stage, stage_keys, "sections." .. stage_name)
      validate_text(stage.lua, "sections." .. stage_name .. ".lua")
      validate_text(stage.asm, "sections." .. stage_name .. ".asm")
    end
  end
end

---@param args table
---@param kind "group"|"value"|"aa"
---@return CtEntry
local function finish_entry(args, kind)
  args.kind = kind
  args.children = validate_children(args.children, "children")
  args.hotkeys = validate_hotkeys(args.hotkeys)
  args[private.node_marker] = kind
  return args
end

local common_params = {
  id = id_param,
  description = Param.required():validate(validation.is_string),
  color = color_param,
  no_checkbox = boolean_param,
  save_value = boolean_param,
  value_on_activate = scalar_param,
  value_on_deactivate = scalar_param,
  restore_on_deactivate = boolean_param,
  options = table_param,
  hotkeys = table_param,
  children = table_param,
}

---@param extra table<string, any>
---@return table<string, any>
local function with_common(extra)
  local params = {}
  for key, value in pairs(common_params) do
    params[key] = value
  end
  for key, value in pairs(extra) do
    params[key] = value
  end
  return params
end

---@overload fun(args: CtGroupArgs): CtEntry
ct.group = wrap({
  params = with_common({
    address = Param.optional():validate(validation.is_string),
  }),
  body = function(args)
    validate_options(args.options)
    return finish_entry(args, "group")
  end,
})

local value_types = {
  binary = true,
  byte = true,
  ["2_bytes"] = true,
  ["4_bytes"] = true,
  ["8_bytes"] = true,
  float = true,
  double = true,
  string = true,
  byte_array = true,
}

---@overload fun(args: CtValueArgs): CtEntry
ct.value = wrap({
  params = with_common({
    value_type = Param.required():validate(function(value)
      return type(value) == "string" and value_types[value] == true
    end),
    address = Param.optional():validate(validation.is_string),
    offsets = table_param,
    show_as_signed = boolean_param,
    show_as_hex = boolean_param,
    bit_start = id_param,
    bit_length = id_param,
    show_as_binary = boolean_param,
    length = id_param,
    unicode = boolean_param,
    code_page = id_param,
    zero_terminate = boolean_param,
    byte_length = id_param,
    dropdown = table_param,
  }),
  body = function(args)
    validate_options(args.options)
    args.address = args.address or ""
    args.show_as_signed = args.show_as_signed == true
    args.show_as_hex = args.show_as_hex == true
    args.offsets = args.offsets or {}
    validation.array(args.offsets, "offsets")
    for index, offset in ipairs(args.offsets) do
      validation.string(offset, string.format("offsets[%d]", index))
    end
    validate_dropdown(args.dropdown)

    if args.value_type == "binary" then
      if args.length ~= nil or args.unicode ~= nil or args.code_page ~= nil
        or args.zero_terminate ~= nil or args.byte_length ~= nil
      then
        error("binary values do not accept string or byte-array fields", 2)
      end
      args.bit_start = args.bit_start or 0
      if args.bit_length == nil or args.bit_length <= 0 then
        error("bit_length: required and must be positive for binary values", 2)
      end
      args.show_as_binary = args.show_as_binary == true
    elseif args.value_type == "string" then
      if args.bit_start ~= nil or args.bit_length ~= nil
        or args.show_as_binary ~= nil or args.byte_length ~= nil
      then
        error("string values do not accept binary or byte-array fields", 2)
      end
      if args.length == nil or args.length <= 0 then
        error("length: required and must be positive for string values", 2)
      end
      args.unicode = args.unicode == true
      args.code_page = args.code_page or 0
      if args.zero_terminate == nil then
        args.zero_terminate = true
      end
    elseif args.value_type == "byte_array" then
      if args.bit_start ~= nil or args.bit_length ~= nil or args.show_as_binary ~= nil
        or args.length ~= nil or args.unicode ~= nil or args.code_page ~= nil
        or args.zero_terminate ~= nil
      then
        error("byte arrays do not accept binary or string fields", 2)
      end
      if args.byte_length == nil or args.byte_length <= 0 then
        error("byte_length: required and must be positive for byte arrays", 2)
      end
    elseif args.bit_start ~= nil or args.bit_length ~= nil or args.show_as_binary ~= nil
      or args.length ~= nil or args.unicode ~= nil or args.code_page ~= nil
      or args.zero_terminate ~= nil or args.byte_length ~= nil
    then
      error("this value type does not accept binary, string, or byte-array fields", 2)
    end
    return finish_entry(args, "value")
  end,
})

---@overload fun(args: CtAaArgs): CtEntry
ct.aa = wrap({
  params = with_common({
    script = text_param,
    sections = table_param,
    async = boolean_param,
  }),
  body = function(args)
    if (args.script == nil) == (args.sections == nil) then
      error("exactly one of script or sections is required", 2)
    end
    validate_sections(args.sections)
    validate_options(args.options)
    return finish_entry(args, "aa")
  end,
})

---@overload fun(path: string): CtSourceRef
ct.file = function(path)
  validation.string(path, "path")
  if path == "" then
    error("path: must not be empty", 2)
  end
  return source.new(path)
end

---@overload fun(args: CtTableArgs): CtTable
ct.table = wrap({
  params = {
    version = Param.optional():validate(validation.is_positive_integer),
    emit_defaults = boolean_param,
    uses_mono = boolean_param,
    auto_attach = Param.optional():validate(validation.is_string),
    show_script_errors = boolean_param,
    read_only_address_list = boolean_param,
    full_client_address_list = boolean_param,
    custom_script_error_message = text_param,
    comments = text_param,
    lua_script = text_param,
    entries = table_param,
  },
  body = function(args)
    args.version = args.version or 52
    if args.emit_defaults == nil then
      args.emit_defaults = true
    end
    args.entries = validate_children(args.entries, "entries")
    args[private.root_marker] = true
    return args
  end,
})

return ct
