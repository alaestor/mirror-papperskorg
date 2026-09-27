local test = require("test_utils")
require("tools.env_mock")

test.run_and_report(function()
    local generated_path = arg and arg[1]
    if not generated_path then
        error("Usage: lua tests/test-table-builder.lua <generated.lua>")
    end

    package.path = ""
    local roots = dofile(generated_path)

    test.describe("Generated table builder script", function()
        test.it("should execute without ALCE or runtime module dependencies", function()
            test.expect(#roots).to_eq(1)
            test.expect(_G.alce).to_eq(nil)
        end)

        test.it("should preserve the embedded AA script", function()
            local record = getAddressList():getMemoryRecordByDescription("Embedded script")
            test.expect(record ~= nil).to_be_true()
            test.expect(record.Script).to_eq(
                "[ENABLE]\ndefine(embeddedValue, 42)\n\n[DISABLE]\nunregisterSymbol(embeddedValue)\n"
            )
        end)
    end)
end)
