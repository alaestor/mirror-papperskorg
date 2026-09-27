local test = require("test_utils")
require("tools.env_mock")
local compiler = require("table_builder_compiler")

local function read_file(path)
    local file = assert(io.open(path, "rb"))
    local content = assert(file:read("*a"))
    assert(file:close())
    return content
end

local runtime_source = read_file("table-builder/table_builder_runtime.lua")

test.run_and_report(function()
    test.describe("table builder compiler", function()
        test.it("should emit a standalone CE script", function()
            local generated = compiler.compile({
                source_name = "definition.lua",
                source = [[
return {
    records = {
        {
            kind = "aa",
            description = "Compiled AA",
            script = "[ENABLE]\n// exact\n[DISABLE]\n",
        },
    },
}
]],
                runtime_source = runtime_source,
            })
            local previous_path = package.path
            package.path = ""
            local chunk, load_error = load(generated, "@generated.lua", "t", _G)
            test.expect(chunk ~= nil).to_be_true()
            if chunk == nil then
                error(load_error)
            end
            chunk()
            package.path = previous_path

            local aa = getAddressList():getMemoryRecordByDescription("Compiled AA")
            test.expect(aa.Script).to_eq("[ENABLE]\n// exact\n[DISABLE]\n")
        end)

        test.it("should report definition syntax errors using the input name", function()
            local ok, message = pcall(function()
                compiler.compile({
                    source_name = "broken-definition.lua",
                    source = "return {",
                    runtime_source = runtime_source,
                })
            end)

            test.expect(ok).to_be_false()
            test.expect(tostring(message):find("broken-definition.lua", 1, true) ~= nil).to_be_true()
        end)
    end)
end)
