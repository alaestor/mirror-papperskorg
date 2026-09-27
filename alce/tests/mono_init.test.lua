local test = require("test_utils")
require("tools.env_mock")

---@diagnostic disable: lowercase-global: as declared by CE

local alce = require("main")

test.run_and_report(function()
    test.describe("mono.init", function()
        test.it("should initialize on the minimum supported Cheat Engine version", function()
            local versionChecked = false
            getCEVersion = function()
                versionChecked = true
                return 7.7
            end
            alce.isAttached = function() return true end

            alce.mono.init()

            test.expect(versionChecked).to_be_true()
        end)

        test.it("should initialize on a newer Cheat Engine version", function()
            getCEVersion = function() return 8.0 end
            alce.isAttached = function() return true end

            alce.mono.init()
        end)

        test.it_throws("should reject Cheat Engine versions older than 7.7", function()
            getCEVersion = function() return 7.6 end
            alce.mono.init()
        end)

        test.it_throws("should reject an unavailable Cheat Engine version", function()
            getCEVersion = function() return nil end
            alce.mono.init()
        end)
    end)
end)
