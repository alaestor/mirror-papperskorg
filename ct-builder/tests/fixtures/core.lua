local ct = require("ct")

return ct.table({
  entries = {
    ct.value({
      description = "No description",
      value_type = ct.types.int32,
    }),
  },
})
