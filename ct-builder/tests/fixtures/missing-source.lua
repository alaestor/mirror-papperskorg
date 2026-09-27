local ct = require("ct")

return ct.table({
  lua_script = ct.file("does-not-exist.lua"),
})
