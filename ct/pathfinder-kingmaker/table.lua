local ct = require("ct")

return ct.table({
  version = 52,
  lua_script = ct.file("global.lua"),
  entries = {
    ct.group({
      description = "",
      options = { hide_children = true, allow_manual_collapse = true },
      children = {
        ct.aa({
          description = "debug: print monoclasses",
          script = ct.file("debug-print-monoclasses.cea"),
         }),
        ct.aa({
          description = "debug: print hello world",
          script = ct.file("debug-print-hello-world.cea"),
        }),
        ct.group({
          description = "header template",
          options = { hide_children = true, manual_expand_collapse = true, allow_manual_collapse = true },
        }),
      },
    }),
    ct.aa({
      options = { hide_children = true, allow_manual_collapse = true },
      description = "Enable",
      script = ct.file("enable.cea"),
      children = {
        ct.aa({
          description = "expr",
          script = ct.file("expr.cea")
        }),
      },
    }),
  },
})
