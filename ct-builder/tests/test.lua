local ct = require("ct")
local compiler = require("ct.compiler")

---@param condition unknown
---@param message string
local function assert_true(condition, message)
  if not condition then
    error(message, 2)
  end
end

---@param haystack string
---@param needle string
---@param message? string
local function assert_contains(haystack, needle, message)
  assert_true(
    haystack:find(needle, 1, true) ~= nil,
    message or string.format("expected output to contain %q", needle)
  )
end

---@param haystack string
---@param needle string
---@param message? string
local function assert_not_contains(haystack, needle, message)
  assert_true(
    haystack:find(needle, 1, true) == nil,
    message or string.format("expected output not to contain %q", needle)
  )
end

---@param callback fun()
---@param expected string
local function assert_error(callback, expected)
  local ok, message = pcall(callback)
  assert_true(not ok, "expected call to fail")
  assert_contains(tostring(message), expected)
end

local empty = compiler.compile(ct.table({}))
assert_contains(empty, '<CheatTable CheatEngineTableVersion="52">')
assert_contains(empty, "<CheatEntries/>")
assert_not_contains(empty, "CheatCodes")

local ids = compiler.compile(ct.table({
  entries = {
    ct.group({ description = "auto one" }),
    ct.group({ id = 1, description = "explicit" }),
    ct.group({
      description = "auto two",
      hotkeys = {
        { id = 1, action = "toggle", keys = { 65 } },
        { action = "activate", keys = { 66 } },
      },
    }),
  },
}))
assert_contains(ids, "<ID>0</ID>")
assert_contains(ids, "<ID>1</ID>")
assert_contains(ids, "<ID>2</ID>")

local compact = compiler.compile(ct.table({
  emit_defaults = false,
  entries = {
    ct.value({
      description = "compact",
      value_type = ct.types.string,
      length = 10,
    }),
  },
}))
assert_not_contains(compact, "ShowAsSigned")
assert_not_contains(compact, "Unicode")
assert_not_contains(compact, "CodePage")
assert_not_contains(compact, "ZeroTerminate")
assert_not_contains(compact, "<Address")
assert_contains(compact, "<Length>10</Length>")

local structured = compiler.compile(ct.table({
  entries = {
    ct.aa({
      description = "structured",
      sections = {
        comment = "comment",
        header = { lua = "header lua", asm = "header asm" },
        enable = { lua = "enable lua", asm = "enable asm" },
        disable = { asm = "disable asm" },
      },
    }),
  },
}))
assert_contains(structured, "{$lua}\nheader lua\n{$asm}")
assert_contains(structured, "[ENABLE]")
assert_contains(structured, "{$lua}\nenable lua\n{$asm}")
assert_contains(structured, "[DISABLE]")
assert_not_contains(structured, "[DISABLE]\n\n{$lua}")

local features = compiler.compile(ct.table({
  uses_mono = true,
  auto_attach = 'game"&<.exe',
  entries = {
    ct.value({
      description = "dropdown",
      value_type = ct.types.int32,
      dropdown = {
        read_only = true,
        items = {
          { value = 0, description = "zero & less < more" },
        },
      },
      hotkeys = {
        {
          action = "set_value",
          keys = { 17, 65 },
          value = -10,
          activate_sound = { text = "active", tts = "EN" },
        },
      },
    }),
  },
}))
assert_contains(features, 'AutoAttach="game&quot;&amp;&lt;.exe"')
assert_contains(features, '<DropDownList ReadOnly="1">0:zero &amp; less &lt; more')
assert_contains(features, '<ActivateSound TTS="EN">active</ActivateSound>')

local entry_options = compiler.compile(ct.table({
  entries = {
    ct.aa({
      description = "async script",
      script = "[ENABLE]\n[DISABLE]",
      async = true,
      options = {
        hide_children = true,
        deactivate_children = true,
      },
    }),
    ct.value({
      description = "configured value",
      value_type = ct.types.int32,
      options = { allow_manual_collapse = true },
    }),
  },
}))
assert_contains(entry_options, '<Options moHideChildren="1" moDeactivateChildrenAsWell="1"/>')
assert_contains(entry_options, '<AssemblerScript Async="1">[ENABLE]')
assert_contains(entry_options, '<Options moAllowManualCollapseAndExpand="1"/>')

assert_error(function()
  ct.group({ description = "bad", typo = true })
end, "unknown key")

assert_error(function()
  ct.group({ description = "bad color", color = "red" })
end, "color")

assert_error(function()
  ct.value({
    description = "wrong subtype fields",
    value_type = ct.types.float,
    length = 10,
  })
end, "does not accept")

assert_error(function()
  compiler.compile({ entries = {} })
end, "ct.table")

assert_error(function()
  compiler.compile(ct.table({
    entries = {
      ct.group({ id = 7, description = "one" }),
      ct.group({ id = 7, description = "two" }),
    },
  }))
end, "duplicate entry ID")

assert_error(function()
  ct.group({
    description = "bad options",
    options = {
      always_hide_children = true,
      hide_children = true,
    },
  })
end, "conflicts")

assert_error(function()
  ct.aa({ description = "bad async", script = "", async = "yes" })
end, "async")

assert_error(function()
  ct.value({
    description = "bad value options",
    value_type = ct.types.int32,
    options = { unknown = true },
  })
end, "unknown key")

assert_error(function()
  compiler.compile(ct.table({ comments = "bad\0xml" }))
end, "not legal in XML")

print("Lua tests passed")
