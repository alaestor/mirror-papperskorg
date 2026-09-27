local test = require("test_utils")
local alce = require("main")

test.run_and_report(function()
    test.describe("env_mock memory simulation", function()
        test.it("should allocate, register, read, write, and unregister symbols", function()
            local memory = alce.memory.AllocateSymbols({
                packets = { { type = vtDword, value = "mock_score" } },
                symbolPrefix = "test_",
            })

            memory.mock_score = 42
            test.expect(memory.mock_score).to_eq(42)
            test.expect(getAddress("test_mock_score")).to_eq(memory.__addresses.mock_score)

            memory:unregister({ "mock_score" })
            test.expect(getAddress("test_mock_score")).to_eq(0)
        end)

        test.it("should release allocations and enumerate registered symbols", function()
            local address = allocateMemory(32)
            registerSymbol("mock_allocation", address, true)

            local symbols = enumRegisteredSymbols()
            test.expect(symbols[1].symbolname).to_eq("mock_allocation")
            test.expect(symbols[1].address).to_eq(address)
            test.expect(symbols[1].allocsize).to_eq(32)
            test.expect(deAlloc(address)).to_eq(true)
            test.expect(deAlloc(address)).to_eq(false)

            unregisterSymbol("mock_allocation")
        end)
    end)

    test.describe("env_mock address list", function()
        test.it("should create, find, attach, and destroy memory records", function()
            local parent = AddressList.createMemoryRecord()
            parent.Description = "mock parent"
            local child = AddressList.createMemoryRecord()
            child.Description = "mock child"

            child:appendToEntry(parent)
            test.expect(parent.Count).to_eq(1)
            test.expect(parent.Child[0]).to_eq(child)
            test.expect(getAddressList():getMemoryRecordByDescription("mock child")).to_eq(child)

            child:destroy()
            test.expect(parent.Count).to_eq(0)
            test.expect(getAddressList():getMemoryRecordByDescription("mock child")).to_eq(nil)
        end)
    end)

    test.describe("env_mock AA and dialog calls", function()
        test.it("should return callable, correctly shaped mock values", function()
            __alce_mock.aob_results["48 8B ??"] = { 0x1234 }

            local scan = AOBScan("48 8B ??")
            test.expect(scan.Count).to_eq(1)
            test.expect(scan.String[0]).to_eq("1234")
            test.expect(AOBScanUnique("48 8B ??")).to_eq(0x1234)
            test.expect(autoAssembleCheck("[ENABLE]")).to_eq(true)
            test.expect(messageDialog("mock")).to_eq(mrOk)
        end)
    end)

    test.describe("env_mock Mono simulation", function()
        test.it("should expose the parameter metadata shape returned by monoscript", function()
            local metadata = assert(mono_method_get_parameters(0x100))
            test.expect(metadata).to_be_type("table")
            test.expect(metadata.parameters).to_be_type("table")
            test.expect(metadata.returnmonotype).to_eq(0)
            test.expect(metadata.returntype).to_eq(MONO_TYPE_VOID)
        end)

        test.it("should expose string-based method signatures", function()
            local parameterTypes, parameterNames, returnType = mono_method_getSignature(0x100)

            test.expect(parameterTypes).to_be_type("string")
            test.expect(parameterNames).to_be_type("table")
            test.expect(returnType).to_eq("void")
        end)
    end)
end)
