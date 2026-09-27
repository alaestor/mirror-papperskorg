local ct = require("ct")

return ct.table({
  auto_attach = "game<&>.exe",
  uses_mono = true,
  comments = "table comments",
  lua_script = ct.file("global.lua"),
  entries = {
    ct.aa({
      id = 10,
      description = "Mixed Lua & ASM",
      sections = {
        comment = "generated script",
        header = {
          lua = ct.file("header.lua"),
          asm = "define(VALUE,10)",
        },
        enable = {
          lua = 'registerSymbol("demo")',
          asm = "alloc(newmem,1000)",
        },
        disable = {
          lua = 'unregisterSymbol("demo")',
          asm = "dealloc(newmem)",
        },
      },
      children = {
        ct.value({
          description = "Pointer",
          value_type = ct.types.int32,
          address = "base",
          offsets = { "1F", "10", "4" },
          dropdown = {
            read_only = true,
            description_only = true,
            display_value_as_item = true,
            items = {
              { value = 0, description = "zero" },
              { value = 10, description = "ten" },
            },
          },
          hotkeys = {
            {
              action = "set_value",
              keys = { 17, 65 },
              value = 10,
              only_while_down = true,
              activate_sound = {
                text = "{MRDescription} Activated",
                tts = "EN",
              },
            },
          },
        }),
      },
    }),
    ct.group({
      description = "Options",
      color = "0000FF",
      options = {
        hide_children = true,
        default_collapsed = true,
        recursive_set_value = true,
        manual_expand_collapse = true,
        activate_children = true,
        deactivate_children = true,
        allow_manual_collapse = true,
      },
      children = {
        ct.value({
          description = "Binary",
          value_type = ct.types.binary,
          bit_start = 3,
          bit_length = 15,
        }),
        ct.value({
          description = "Unicode",
          value_type = ct.types.string,
          length = 20,
          unicode = true,
        }),
        ct.value({
          description = "Bytes",
          value_type = ct.types.byte_array,
          byte_length = 8,
          show_as_hex = true,
        }),
      },
    }),
  },
})
