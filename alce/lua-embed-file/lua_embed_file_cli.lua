local lua_embed_file = require("lua_embed_file")

if not lua_embed_file.run(arg or {}) then
    os.exit(1)
end
