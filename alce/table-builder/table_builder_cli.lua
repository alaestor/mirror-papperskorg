local compiler = require("table_builder_compiler")

if not compiler.run(arg or {}) then
    os.exit(1)
end
