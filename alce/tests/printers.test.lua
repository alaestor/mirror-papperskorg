local test = require("test_utils")
local alce = require("main")

test.run_and_report(function()
    test.describe("warning diagnostics", function()
        alce.cfg.warn_print = false

        test.it("should capture warning arguments and caller information", function()
            alce.warnings.clear()

            alce.warn("address=", 42)

            local warning = alce.warnings.last
            test.expect(warning.message).to_eq("address=42")
            test.expect(warning.arguments.n).to_eq(2)
            test.expect(warning.arguments[1]).to_eq("address=")
            test.expect(warning.arguments[2]).to_eq(42)
            test.expect(warning.short_source).to_be_type("string")
            test.expect(warning.line).to_be_type("number")
            test.expect(warning.traceback).to_be_type("string")
        end)

        test.it("should retain only the five most recent warnings by default", function()
            alce.warnings.clear()
            alce.cfg.warning_history_size = 5

            for index = 1, 6 do alce.warn("warning ", index) end

            test.expect(#alce.warnings.history).to_eq(5)
            test.expect(alce.warnings.history[1].message).to_eq("warning 2")
            test.expect(alce.warnings.history[5].message).to_eq("warning 6")
        end)

        test.it("should omit caller locals by default", function()
            alce.warnings.clear()
            alce.cfg.warning_collect_locals = false

            alce.warn("shallow")

            test.expect(alce.warnings.last.locals).to_be_false()
        end)

        test.it("should capture caller locals when enabled", function()
            alce.warnings.clear()
            alce.cfg.warning_collect_locals = true

            local expected_state = { value = 42 }
            alce.warn("deep")

            local captured_state
            for _, captured in ipairs(alce.warnings.last.locals) do
                if captured.name == "expected_state" then captured_state = captured.value end
            end
            test.expect(captured_state).to_eq(expected_state)
            alce.cfg.warning_collect_locals = false
        end)

        test.it("should clear retained warning diagnostics", function()
            alce.warn("clear me")

            alce.warnings.clear()

            test.expect(alce.warnings.last).to_be_false()
            test.expect(#alce.warnings.history).to_eq(0)
        end)
    end)
end)
