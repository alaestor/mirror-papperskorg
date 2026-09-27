local test = require("test_utils")
require("tools.env_mock")
local table_builder = require("table_builder_runtime")

local function expect_error_contains(callback, expected)
    local ok, message = pcall(callback)
    test.expect(ok).to_be_false()
    test.expect(tostring(message):find(expected, 1, true) ~= nil).to_be_true()
end

local function record(description)
    return getAddressList():getMemoryRecordByDescription(description)
end

test.run_and_report(function()
    test.describe("table builder runtime", function()
        test.it("should replace the address list with an ordered memory-record hierarchy", function()
            AddressList.createMemoryRecord().Description = "old record"

            local roots = table_builder.build({
                records = {
                    {
                        kind = "header",
                        description = "Player",
                        properties = {
                            Color = 0x112233,
                            Collapsed = true,
                        },
                        children = {
                            {
                                kind = "aa",
                                description = "Enable",
                                script = "[ENABLE]\nalloc(test, 8)\n\n[DISABLE]\ndealloc(test)\n",
                                properties = { Async = true },
                            },
                            {
                                kind = "record",
                                description = "Health",
                                address = 0x1234,
                                vtype = vtDword,
                                offsets = { 0x10, "module.symbolicOffset" },
                                dropdown = {
                                    items = "0:Dead\n1:Alive",
                                    read_only = true,
                                    description_only = true,
                                },
                                properties = { ShowAsHex = true },
                            },
                        },
                    },
                },
            })

            test.expect(#roots).to_eq(1)
            test.expect(record("old record")).to_eq(nil)
            local player = record("Player")
            test.expect(player.IsGroupHeader).to_be_true()
            test.expect(player.DontSave).to_be_false()
            test.expect(player.Color).to_eq(0x112233)
            test.expect(player.Count).to_eq(2)
            test.expect(player.Child[0].Description).to_eq("Enable")
            test.expect(player.Child[1].Description).to_eq("Health")

            local aa = record("Enable")
            test.expect(aa.Type).to_eq(vtAutoAssembler)
            test.expect(aa.Script).to_eq("[ENABLE]\nalloc(test, 8)\n\n[DISABLE]\ndealloc(test)\n")
            test.expect(aa.Async).to_be_true()

            local health = record("Health")
            test.expect(health.Address).to_eq("1234")
            test.expect(health.OffsetCount).to_eq(2)
            test.expect(health.Offset[0]).to_eq(0x10)
            test.expect(health.OffsetText[1]).to_eq("module.symbolicOffset")
            test.expect(health.DropDownList.Text).to_eq("0:Dead\n1:Alive")
            test.expect(health.DropDownReadOnly).to_be_true()
            test.expect(health.DropDownDescriptionOnly).to_be_true()
            test.expect(health.DisplayAsDropDownListItem).to_be_true()
            test.expect(health.ShowAsHex).to_be_true()
        end)

        test.it("should allow blank descriptions", function()
            local roots = table_builder.build({
                records = {
                    { kind = "header", description = "" },
                    { kind = "record", description = "   " },
                },
            })

            test.expect(roots[1].Description).to_eq("")
            test.expect(roots[2].Description).to_eq("   ")
        end)

        test.it("should apply linked dropdowns and type-specific properties", function()
            table_builder.build({
                records = {
                    {
                        kind = "record",
                        description = "Name",
                        vtype = vtString,
                        dropdown = { linked_record = "Name choices", display_as_item = false },
                        properties = {
                            ["String.Size"] = 32,
                            ["String.Unicode"] = true,
                        },
                    },
                },
            })

            local name = record("Name")
            test.expect(name.DropDownLinkedMemrec).to_eq("Name choices")
            test.expect(name.DisplayAsDropDownListItem).to_be_false()
            test.expect(name.String.Size).to_eq(32)
            test.expect(name.String.Unicode).to_be_true()
        end)

        test.it("should validate the whole definition before replacing existing records", function()
            table_builder.build({
                records = { { kind = "header", description = "preserved" } },
            })

            expect_error_contains(function()
                table_builder.build({
                    records = {
                        {
                            kind = "header",
                            description = "invalid",
                            children = {
                                { kind = "aa", description = "broken", script = "" },
                            },
                        },
                    },
                })
            end, "records[1].children[1].script")

            test.expect(record("preserved") ~= nil).to_be_true()
            test.expect(record("invalid")).to_eq(nil)
        end)

        test.it("should roll back new records when Cheat Engine construction fails", function()
            table_builder.build({
                records = { { kind = "header", description = "preserved after CE error" } },
            })
            local original_create = AddressList.createMemoryRecord
            local calls = 0
            AddressList.createMemoryRecord = function(...)
                calls = calls + 1
                if calls == 2 then
                    error("simulated CE failure")
                end
                return original_create(...)
            end

            local ok, message = pcall(function()
                table_builder.build({
                    records = {
                        { kind = "record", description = "partial one" },
                        { kind = "record", description = "partial two" },
                    },
                })
            end)
            AddressList.createMemoryRecord = original_create

            test.expect(ok).to_be_false()
            test.expect(tostring(message):find("simulated CE failure", 1, true) ~= nil).to_be_true()
            test.expect(record("preserved after CE error") ~= nil).to_be_true()
            test.expect(record("partial one")).to_eq(nil)
        end)

        test.it("should reject cycles, sparse arrays, conflicts, and unsupported properties", function()
            local cyclic = { kind = "header", description = "cycle" }
            cyclic.children = { cyclic }
            expect_error_contains(function()
                table_builder.build({ records = { cyclic } })
            end, "must not reuse or cycle")

            expect_error_contains(function()
                table_builder.build({
                    records = {
                        {
                            kind = "record",
                            description = "sparse",
                            offsets = { [1] = 1, [3] = 3 },
                        },
                    },
                })
            end, "must be a dense array")

            expect_error_contains(function()
                table_builder.build({
                    records = {
                        {
                            kind = "record",
                            description = "invalid offset",
                            offsets = { true },
                        },
                    },
                })
            end, "must be a number or string")

            expect_error_contains(function()
                table_builder.build({
                    records = {
                        {
                            kind = "record",
                            description = "dropdown",
                            dropdown = { items = "1:One", linked_record = "other" },
                        },
                    },
                })
            end, "mutually exclusive")

            expect_error_contains(function()
                table_builder.build({
                    records = {
                        {
                            kind = "record",
                            description = "active",
                            properties = { Active = true },
                        },
                    },
                })
            end, "not a supported writable property")
        end)
    end)
end)
