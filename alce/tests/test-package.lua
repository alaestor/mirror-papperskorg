local test = require("test_utils")
require("tools.env_mock")

test.run_and_report(function()
    local package_path = arg and arg[1]
    if not package_path then
        error("Usage: lua tests/test-package.lua <path-to-alce.lua>")
    end

    -- The package must provide every Lua module it needs, including fnlua.
    package.loaded.fn = nil
    package.preload.fn = nil
    package.path = ""

    local previous_pre_execute = function() end
    local previous_post_execute = function() end
    _G.onMemRecPreExecute = previous_pre_execute
    _G.onMemRecPostExecute = previous_post_execute

    local packaged_alce = dofile(package_path)

    test.describe("Packaged ALCE namespace", function()
        test.it("should return and publish the alce namespace", function()
            test.expect(packaged_alce).to_be_type("table")
            test.expect(_G.alce).to_eq(packaged_alce)
            test.expect(packaged_alce.globals).to_eq(packaged_alce)
        end)

        test.it("should expose every module assembled by src/main.lua", function()
            local expected_modules = {
                "cheat_table",
                "fmt",
                "memory",
                "mono",
                "mono_plumbing",
                "mono_t",
                "monoscript",
                "printers",
                "T",
                "utils",
                "validators",
                "vt",
            }

            for _, module_name in ipairs(expected_modules) do
                test.expect(packaged_alce[module_name]).to_be_type("table")
            end
        end)

        test.it("should promote printer functions to the top-level namespace", function()
            for name, printer in pairs(packaged_alce.printers) do
                if type(printer) == "function" then
                    test.expect(packaged_alce[name]).to_eq(printer)
                end
            end
        end)
    end)

    test.describe("Cheat Engine callback globals", function()
        test.it("should expose the pre-execute callback", function()
            test.expect(_G.onMemRecPreExecute).to_be_type("function")
            test.expect(_G.onMemRecPreExecute == previous_pre_execute).to_be_false()
        end)

        test.it("should expose the post-execute callback", function()
            test.expect(_G.onMemRecPostExecute).to_be_type("function")
            test.expect(_G.onMemRecPostExecute == previous_post_execute).to_be_false()
        end)
    end)
end)
