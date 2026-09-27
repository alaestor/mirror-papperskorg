local validators = require("validators")
local alce = require("globals")
local fnlua = require("fn")
local fn = fnlua.wrap
local Param = fnlua.Param

---@class alce
---@field THIS MemoryRecord
---@field LAST_SUCCESS MemoryRecord
---@field LAST_FAILURE MemoryRecord
---@field executionCallback function

---@diagnostic disable: lowercase-global: as defined by CE

---Hook called before a memory record script executes.
---@param memrec MemoryRecord
---@param newState any
---@diagnostic disable-next-line
_G.onMemRecPreExecute = function(memrec, newState)
    if memrec.Type == vtAutoAssembler then
        alce.debug('Trying to run script: ', memrec.Description)
        alce.THIS = memrec
    end
end

---Hook called after a memory record script executes.
---@param memrec MemoryRecord
---@param newState any
---@param succeeded boolean
_G.onMemRecPostExecute = function(memrec, newState, succeeded)
    alce.THIS = nil
    alce.debug('Script ', memrec.Description, succeeded and ' succeeded' or ' failed')
    if memrec.Type == vtAutoAssembler then
        if succeeded then
            alce.LAST_SUCCESS = memrec
        else
            alce.LAST_FAILURE = memrec
        end
    end
    if validators.isCallable(alce.executionCallback) then
        alce.executionCallback(memrec, newState, succeeded)
        alce.executionCallback = nil
    end
end

---Cheat table utilities for hooking script execution and managing memory records.
---@class cheattable
--[[
Hooks `onMemRecPreExecute` and `onMemRecPostExecute`.

- `alce.THIS` is the MemoryRecord currently executing.
- `alce.LAST_SUCCESS` and `alce.LAST_FAILURE` record the most recent script
  outcome for cheat-table execution (not the Execute button or syntax checks).
]]
local cheattable = {
}

---Makes disableWithoutExecute() be called on the next MemoryRecord script that runs successfully.
---Can be used at the bottom of an [ENABLE] section to turn a script into a momentary button rather than toggle.
---@overload fun(args: { disableBeep?: boolean })
cheattable.disableAfterSuccess = fn({
    params = { disableBeep = Param.default({ value = false }):validate(function(value) return type(value) == "boolean" end) },
    body = function(args)
    local disableBeep = args.disableBeep
    alce.executionCallback = function(this, _, succeeded)
        if succeeded then
            this:disableWithoutExecute()
            if not disableBeep then beep() end
        end
    end
end,
})

cheattable.disableAfterSuccess({ disableBeep = false })

---Destroys all children of the given memory record.
---@overload fun(memoryRecord: MemoryRecord)
function cheattable.clearChildren(memoryRecord)
    local count = memoryRecord.Count
    for i = count - 1, 0, -1 do
        memoryRecord.Child[i]:destroy()
    end
end

---Finds a memory record by description and destroys its children.
---@overload fun(args: { desc: string, addressList?: AddressList })
cheattable.clearChildrenByDesc = fn({
    params = {
        desc = Param.required():validate(validators.isNonBlankString),
        addressList = Param.optional(),
    },
    body = function(args)
    local desc = args.desc
    local addressList = args.addressList
    local al = addressList or getAddressList()
    local parent = al:getMemoryRecordByDescription(desc)
    if parent then
        cheattable.clearChildren(parent)
    end
end,
})

---Creates a new MemoryRecord attached to a parent.
---@class CreateRecordParams
---@field parent MemoryRecord?
---@field description string?
---@field vtype number?
---@field address number|string?
---@field offsets integer[]?
---@field dropDownSettings table?
---@field saveToTable boolean?
---@overload fun(args: CreateRecordParams): MemoryRecord
cheattable.createRecord = fn({
    params = {
        parent = Param.optional(),
        description = Param.optional(),
        vtype = Param.optional(),
        address = Param.optional(),
        offsets = Param.optional(),
        dropDownSettings = Param.optional(),
        saveToTable = Param.default({ value = false }):validate(function(value) return type(value) == "boolean" end),
    },
    body = function(args)
    local mr = AddressList.createMemoryRecord()
    if validators.isNonBlankString(args.description) then
        mr.Description = args.description
    end
    mr.Type = args.vtype or vtDword
    mr.DontSave = args.saveToTable ~= true
    if validators.isAddresslike(args.address) then
        mr.Address = string.format("%X", args.address)
    elseif validators.isNonBlankString(args.address) then
        mr.Address = args.address
    end
    if args.offsets then
        mr.OffsetCount = #args.offsets
        for i, offset in ipairs(args.offsets) do
            mr.Offset[i - 1] = offset
        end
    end
    if args.dropDownSettings then
        if validators.isNonBlankString(args.dropDownSettings.options) then
            mr.DropDownList.Text = args.dropDownSettings.options
        elseif validators.isNonBlankString(args.dropDownSettings.optionsFrom) then
            mr.DropDownLinkedMemrec = args.dropDownSettings.optionsFrom
        end
        mr.DropDownDescriptionOnly = args.dropDownSettings.hideNumbers == true
        mr.DropDownReadOnly = args.dropDownSettings.noManualInput == true
        mr.DisplayAsDropDownListItem = args.dropDownSettings.dontDisplayAsString ~= true
    end
    mr.Options = '[moAllowManualCollapseAndExpand]'
    if args.parent then
        mr.appendToEntry(args.parent)
    end
    return mr
end,
})

---Creates a new group header MemoryRecord.
---@class CreateHeaderParams
---@field parent MemoryRecord?
---@field description string?
---@field showCollapseButtons boolean?
---@field saveToTable boolean?
---@overload fun(args: CreateHeaderParams): MemoryRecord
cheattable.createHeader = fn({
    params = {
        parent = Param.optional(),
        description = Param.optional(),
        showCollapseButtons = Param.default({ value = false }):validate(function(value) return type(value) == "boolean" end),
        saveToTable = Param.default({ value = false }):validate(function(value) return type(value) == "boolean" end),
    },
    body = function(args)
    local mr = AddressList.createMemoryRecord()
    if validators.isNonBlankString(args.description) then
        mr.Description = args.description
    end
    if args.parent then
        mr.appendToEntry(args.parent)
    end
    mr.DontSave = args.saveToTable ~= true
    mr.IsGroupHeader = true
    mr.Options = args.showCollapseButtons and
        '[moHideChildren,moAllowManualCollapseAndExpand,moManualExpandCollapse]' or
        '[moHideChildren,moAllowManualCollapseAndExpand]'
    return mr
end,
})

return cheattable
