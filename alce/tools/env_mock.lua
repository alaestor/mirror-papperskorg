-- Mocking layer for Cheat Engine API to allow standalone Lua execution of ALCE.
-- This file should be required before any ALCE modules.

--# GUI events from CE's MainForm

_G.MainForm = _G.MainForm or {
    OnProcessOpened = function() end,
}

--# Variable types from CE's defines.lua

---@type VirtualType
_G.vtByte=0
---@type VirtualType
_G.vtWord=1
---@type VirtualType
_G.vtDword=2
---@type VirtualType
_G.vtQword=3
---@type VirtualType
_G.vtSingle=4
---@type VirtualType
_G.vtDouble=5
---@type VirtualType
_G.vtString=6
---@type VirtualType
_G.vtUnicodeString=7 --Only used by autoguess
---@type VirtualType
_G.vtWideString=7
---@type VirtualType
_G.vtByteArray=8
---@type VirtualType
_G.vtBinary=9
---@type VirtualType
_G.vtAll=10
---@type VirtualType
_G.vtAutoAssembler=11
---@type VirtualType
_G.vtPointer=12 --Only used by autoguess and structures
---@type VirtualType
_G.vtCustom=13
---@type VirtualType
_G.vtGrouped=14

--# Global constants from CE's monoscript.lua

---@type MonoType
_G.MONO_TYPE_END        = 0x00       -- End of List
---@type MonoType
_G.MONO_TYPE_VOID       = 0x01
---@type MonoType
_G.MONO_TYPE_BOOLEAN    = 0x02
---@type MonoType
_G.MONO_TYPE_CHAR       = 0x03
---@type MonoType
_G.MONO_TYPE_I1         = 0x04
---@type MonoType
_G.MONO_TYPE_U1         = 0x05
---@type MonoType
_G.MONO_TYPE_I2         = 0x06
---@type MonoType
_G.MONO_TYPE_U2         = 0x07
---@type MonoType
_G.MONO_TYPE_I4         = 0x08
---@type MonoType
_G.MONO_TYPE_U4         = 0x09
---@type MonoType
_G.MONO_TYPE_I8         = 0x0a
---@type MonoType
_G.MONO_TYPE_U8         = 0x0b
---@type MonoType
_G.MONO_TYPE_R4         = 0x0c
---@type MonoType
_G.MONO_TYPE_R8         = 0x0d
---@type MonoType
_G.MONO_TYPE_STRING     = 0x0e
---@type MonoType
_G.MONO_TYPE_PTR        = 0x0f       -- arg: <type> token
---@type MonoType
_G.MONO_TYPE_BYREF      = 0x10       -- arg: <type> token
---@type MonoType
_G.MONO_TYPE_VALUETYPE  = 0x11       -- arg: <type> token
---@type MonoType
_G.MONO_TYPE_CLASS      = 0x12       -- arg: <type> token
---@type MonoType
_G.MONO_TYPE_VAR        = 0x13       -- number
---@type MonoType
_G.MONO_TYPE_ARRAY      = 0x14       -- type, rank, boundsCount, bound1, loCount, lo1
---@type MonoType
_G.MONO_TYPE_GENERICINST= 0x15       -- <type> <type-arg-count> <type-1> \x{2026} <type-n> */
---@type MonoType
_G.MONO_TYPE_TYPEDBYREF = 0x16
---@type MonoType
_G.MONO_TYPE_I          = 0x18
---@type MonoType
_G.MONO_TYPE_U          = 0x19
---@type MonoType
_G.MONO_TYPE_FNPTR      = 0x1b       -- arg: full method signature */
---@type MonoType
_G.MONO_TYPE_OBJECT     = 0x1c
---@type MonoType
_G.MONO_TYPE_SZARRAY    = 0x1d       -- 0-based one-dim-array */
---@type MonoType
_G.MONO_TYPE_MVAR       = 0x1e       -- number */
---@type MonoType
_G.MONO_TYPE_CMOD_REQD  = 0x1f       -- arg: typedef or typeref token */
---@type MonoType
_G.MONO_TYPE_CMOD_OPT   = 0x20       -- optional arg: typedef or typref token */
---@type MonoType
_G.MONO_TYPE_INTERNAL   = 0x21       -- CLR internal type */
---@type MonoType
_G.MONO_TYPE_MODIFIER   = 0x40       -- Or with the following types */
---@type MonoType
_G.MONO_TYPE_SENTINEL   = 0x41       -- Sentinel for varargs method signature */
---@type MonoType
_G.MONO_TYPE_PINNED     = 0x45       -- Local var that points to pinned object */
---@type MonoType
_G.MONO_TYPE_ENUM       = 0x55 -- an enumeration */

---@type table<MonoType, VirtualType> a lookup table for converting MonoType constants to VirtualType constants
_G.monoTypeToVartypeLookup={} --for dissect data
_G.monoTypeToVartypeLookup[MONO_TYPE_BOOLEAN]=vtByte
_G.monoTypeToVartypeLookup[MONO_TYPE_CHAR]=vtUnicodeString --the actual chars...
_G.monoTypeToVartypeLookup[MONO_TYPE_I1]=vtByte
_G.monoTypeToVartypeLookup[MONO_TYPE_U1]=vtByte
_G.monoTypeToVartypeLookup[MONO_TYPE_I2]=vtWord
_G.monoTypeToVartypeLookup[MONO_TYPE_U2]=vtWord
_G.monoTypeToVartypeLookup[MONO_TYPE_I4]=vtDword
_G.monoTypeToVartypeLookup[MONO_TYPE_U4]=vtDword
_G.monoTypeToVartypeLookup[MONO_TYPE_I8]=vtQword
_G.monoTypeToVartypeLookup[MONO_TYPE_U8]=vtQword
_G.monoTypeToVartypeLookup[MONO_TYPE_R4]=vtSingle
_G.monoTypeToVartypeLookup[MONO_TYPE_R8]=vtDouble
_G.monoTypeToVartypeLookup[MONO_TYPE_STRING]=vtPointer --pointer to a string object
_G.monoTypeToVartypeLookup[MONO_TYPE_PTR]=vtPointer
_G.monoTypeToVartypeLookup[MONO_TYPE_I]=vtPointer --IntPtr
_G.monoTypeToVartypeLookup[MONO_TYPE_U]=vtPointer
_G.monoTypeToVartypeLookup[MONO_TYPE_OBJECT]=vtPointer --object
_G.monoTypeToVartypeLookup[MONO_TYPE_BYREF]=vtPointer
_G.monoTypeToVartypeLookup[MONO_TYPE_CLASS]=vtPointer
_G.monoTypeToVartypeLookup[MONO_TYPE_FNPTR]=vtPointer
_G.monoTypeToVartypeLookup[MONO_TYPE_GENERICINST]=vtPointer
_G.monoTypeToVartypeLookup[MONO_TYPE_ARRAY]=vtPointer
_G.monoTypeToVartypeLookup[MONO_TYPE_SZARRAY]=vtPointer
_G.monoTypeToVartypeLookup[MONO_TYPE_VALUETYPE]=vtPointer --needed for structs when returned by invoking a method( even though they are not qwords)

---@type table<MonoType, string> lookup table for converting MonoType to a human-readable type string
_G.monoTypeToCStringLookup={}
_G.monoTypeToCStringLookup[MONO_TYPE_END]='void'
_G.monoTypeToCStringLookup[MONO_TYPE_BOOLEAN]='boolean'
_G.monoTypeToCStringLookup[MONO_TYPE_CHAR]='char'
_G.monoTypeToCStringLookup[MONO_TYPE_I1]='char'
_G.monoTypeToCStringLookup[MONO_TYPE_U1]='unsigned char'
_G.monoTypeToCStringLookup[MONO_TYPE_I2]='short'
_G.monoTypeToCStringLookup[MONO_TYPE_U2]='unsigned short'
_G.monoTypeToCStringLookup[MONO_TYPE_I4]='int'
_G.monoTypeToCStringLookup[MONO_TYPE_U4]='unsigned int'
_G.monoTypeToCStringLookup[MONO_TYPE_I8]='int64'
_G.monoTypeToCStringLookup[MONO_TYPE_U8]='unsigned int 64'
_G.monoTypeToCStringLookup[MONO_TYPE_R4]='single'
_G.monoTypeToCStringLookup[MONO_TYPE_R8]='double'
_G.monoTypeToCStringLookup[MONO_TYPE_STRING]='String'
_G.monoTypeToCStringLookup[MONO_TYPE_PTR]='Pointer'
_G.monoTypeToCStringLookup[MONO_TYPE_BYREF]='Object'
_G.monoTypeToCStringLookup[MONO_TYPE_CLASS]='Object'
_G.monoTypeToCStringLookup[MONO_TYPE_FNPTR]='Function'
_G.monoTypeToCStringLookup[MONO_TYPE_GENERICINST]='<Generic>'
_G.monoTypeToCStringLookup[MONO_TYPE_ARRAY]='Array[]'
_G.monoTypeToCStringLookup[MONO_TYPE_SZARRAY]='String[]'

---@type FieldAttribute
_G.FIELD_ATTRIBUTE_FIELD_ACCESS_MASK=0x0007
---@type FieldAttribute
_G.FIELD_ATTRIBUTE_COMPILER_CONTROLLED=0x0000
---@type FieldAttribute
_G.FIELD_ATTRIBUTE_PRIVATE=0x0001
---@type FieldAttribute
_G.FIELD_ATTRIBUTE_FAM_AND_ASSEM=0x0002
---@type FieldAttribute
_G.FIELD_ATTRIBUTE_ASSEMBLY=0x0003
---@type FieldAttribute
_G.FIELD_ATTRIBUTE_FAMILY=0x0004
---@type FieldAttribute
_G.FIELD_ATTRIBUTE_FAM_OR_ASSEM=0x0005
---@type FieldAttribute
_G.FIELD_ATTRIBUTE_PUBLIC=0x0006
---@type FieldAttribute
_G.FIELD_ATTRIBUTE_STATIC=0x0010
---@type FieldAttribute
_G.FIELD_ATTRIBUTE_INIT_ONLY=0x0020
---@type FieldAttribute
_G.FIELD_ATTRIBUTE_LITERAL=0x0040
---@type FieldAttribute
_G.FIELD_ATTRIBUTE_NOT_SERIALIZED=0x0080
---@type FieldAttribute
_G.FIELD_ATTRIBUTE_SPECIAL_NAME=0x0200
---@type FieldAttribute
_G.FIELD_ATTRIBUTE_PINVOKE_IMPL=0x2000
---@type FieldAttribute
_G.FIELD_ATTRIBUTE_RESERVED_MASK=0x9500
---@type FieldAttribute
_G.FIELD_ATTRIBUTE_RT_SPECIAL_NAME=0x0400
---@type FieldAttribute
_G.FIELD_ATTRIBUTE_HAS_FIELD_MARSHAL=0x1000
---@type FieldAttribute
_G.FIELD_ATTRIBUTE_HAS_DEFAULT=0x8000
---@type FieldAttribute
_G.FIELD_ATTRIBUTE_HAS_FIELD_RVA=0x0100

---@type FieldAttribute
_G.METHOD_ATTRIBUTE_MEMBER_ACCESS_MASK      =0x0007
---@type FieldAttribute
_G.METHOD_ATTRIBUTE_COMPILER_CONTROLLED     =0x0000
---@type FieldAttribute
_G.METHOD_ATTRIBUTE_PRIVATE                 =0x0001
---@type FieldAttribute
_G.METHOD_ATTRIBUTE_FAM_AND_ASSEM           =0x0002
---@type FieldAttribute
_G.METHOD_ATTRIBUTE_ASSEM                   =0x0003
---@type FieldAttribute
_G.METHOD_ATTRIBUTE_FAMILY                  =0x0004
---@type FieldAttribute
_G.METHOD_ATTRIBUTE_FAM_OR_ASSEM            =0x0005
---@type FieldAttribute
_G.METHOD_ATTRIBUTE_PUBLIC                  =0x0006
---@type FieldAttribute
_G.METHOD_ATTRIBUTE_STATIC                  =0x0010
---@type FieldAttribute
_G.METHOD_ATTRIBUTE_FINAL                   =0x0020
---@type FieldAttribute
_G.METHOD_ATTRIBUTE_VIRTUAL                 =0x0040
---@type FieldAttribute
_G.METHOD_ATTRIBUTE_HIDE_BY_SIG             =0x0080
---@type FieldAttribute
_G.METHOD_ATTRIBUTE_VTABLE_LAYOUT_MASK      =0x0100
---@type FieldAttribute
_G.METHOD_ATTRIBUTE_REUSE_SLOT              =0x0000
---@type FieldAttribute
_G.METHOD_ATTRIBUTE_NEW_SLOT                =0x0100
---@type FieldAttribute
_G.METHOD_ATTRIBUTE_STRICT                  =0x0200
---@type FieldAttribute
_G.METHOD_ATTRIBUTE_ABSTRACT                =0x0400
---@type FieldAttribute
_G.METHOD_ATTRIBUTE_SPECIAL_NAME            =0x0800
---@type FieldAttribute
_G.METHOD_ATTRIBUTE_PINVOKE_IMPL            =0x2000
---@type FieldAttribute
_G.METHOD_ATTRIBUTE_UNMANAGED_EXPORT        =0x0008

_G.process = 0x12345678


--# Mocking functions


---@diagnostic disable: unused-local

-- Stateful simulation for ALCE's memory and symbol APIs. It deliberately does
-- not emulate process memory: typed reads and writes share one value per address.
local state = {
    memory = {},
    allocations = {},
    symbols = {},
    symbol_options = {},
    records = {},
    aob_results = {},
    next_address = 0x100000,
    next_record_id = 1,
    beep_count = 0,
    messages = {},
    input_result = nil,
    dialog_result = nil,
}
_G.__alce_mock = state

local function address_of(value)
    if type(value) == "number" then return value end
    if type(value) == "string" then
        local hex = value:match("^0x(.+)$")
        return state.symbols[value] or tonumber(value) or (hex and tonumber(hex, 16)) or 0
    end
    return 0
end

local function read_value(addr, fallback)
    local value = state.memory[address_of(addr)]
    return value == nil and fallback or value
end

local function write_value(addr, value)
    state.memory[address_of(addr)] = value
    return true
end

_G.readInteger = function(addr) return read_value(addr, 0) end
_G.readFloat = function(addr) return read_value(addr, 0.0) end
_G.readByte = function(addr) return read_value(addr, 0) end
_G.writeInteger = write_value
_G.writeFloat = write_value
_G.writeByte = write_value
_G.getAddress = address_of
_G.getPointerAddress = address_of
_G.readPointer = function(addr) return read_value(addr, 0) end
_G.writePointer = write_value
_G.readString = function(addr) return read_value(addr, "") end
_G.writeString = write_value
_G.readMemory = function(addr, size) return read_value(addr, string.rep("\0", size or 0)) end
_G.writeMemory = write_value
_G.readSmallInteger = function(addr) return read_value(addr, 0) end
_G.readQword = function(addr) return read_value(addr, 0) end
_G.readDouble = function(addr) return read_value(addr, 0.0) end
_G.readBytes = function(addr, size, returnAsTable)
    local value = read_value(addr, nil)
    if type(value) == "table" then return value end
    local bytes = {}
    for i = 1, size or 1 do bytes[i] = 0 end
    return bytes
end
_G.writeSmallInteger = write_value
_G.writeQword = write_value
_G.writeDouble = write_value
_G.writeBytes = write_value
_G.allocateMemory = function(size, baseAddress)
    local address = baseAddress or state.next_address
    state.allocations[address] = size
    state.next_address = math.max(state.next_address, address + size + 0x10)
    return address
end

---@param address number|string
---@param size integer?
---@return boolean success
_G.deAlloc = function(address, size)
    address = address_of(address)
    if state.allocations[address] == nil then return false end
    state.allocations[address] = nil
    state.memory[address] = nil
    return true
end

-- System mocks
_G.target64Bit = function() return true end
_G.targetIs64Bit = function() return true end
_G.isAttached = function() return true end
_G.getCEVersion = function() return 7.7 end
local function create_string_list(values)
    local list = { Count = 0, String = {}, Strings = {}, destroyed = false }
    local function sync_text()
        local lines = {}
        for i = 0, list.Count - 1 do lines[#lines + 1] = list.String[i] end
        rawset(list, "_text", table.concat(lines, "\n"))
    end
    function list.add(self_or_value, optional_value)
        local value = optional_value == nil and self_or_value or optional_value
        local index = list.Count
        list.String[index], list.Strings[index] = tostring(value), tostring(value)
        list.Count = index + 1
        sync_text()
        return index
    end
    function list.clear()
        list.Count, list.String, list.Strings = 0, {}, {}
        sync_text()
    end
    function list.delete(self_or_index, optional_index)
        local index = optional_index == nil and self_or_index or optional_index
        for i = index, list.Count - 2 do
            list.String[i], list.Strings[i] = list.String[i + 1], list.Strings[i + 1]
        end
        list.String[list.Count - 1], list.Strings[list.Count - 1] = nil, nil
        list.Count = math.max(0, list.Count - 1)
        sync_text()
    end
    function list.destroy() list.destroyed = true end
    setmetatable(list, {
        __index = function(_, key)
            if key == "Text" then return rawget(list, "_text") or "" end
        end,
        __newindex = function(_, key, value)
            if key ~= "Text" then rawset(list, key, value); return end
            list.clear()
            for line in tostring(value):gmatch("[^\r\n]+") do list.add(line) end
        end,
    })
    for _, value in ipairs(values or {}) do list.add(value) end
    return list --[[@as StringList]]
end

local address_list = { Count = 0, SelCount = 0, MemoryRecord = {} }
local function refresh_address_list()
    local index = 0
    address_list.MemoryRecord = {}
    for _, record in ipairs(state.records) do
        if not record.destroyed then
            record.Index = index
            address_list.MemoryRecord[index] = record
            address_list[index] = record
            index = index + 1
        end
    end
    address_list.Count = index
end
function address_list.createMemoryRecord()
    local record = {
        ID = state.next_record_id, Index = 0, Description = "", Address = "0",
        AddressString = "0", CurrentAddress = 0, OffsetCount = 0, Offset = {},
        OffsetText = {}, Type = vtDword, Script = "", Value = "0", Active = false,
        Child = {}, Count = 0, DropDownList = create_string_list(), destroyed = false,
        String = { Size = 0, Unicode = false, Codepage = false },
        Binary = { Startbit = 0, Size = 0 },
        Aob = { Size = 0 },
    }
    state.next_record_id = state.next_record_id + 1
    function record.appendToEntry(self_or_parent, optional_parent)
        local parent = optional_parent or self_or_parent
        parent.Child[parent.Count] = record
        parent.Count = parent.Count + 1
        record.Parent = parent
    end
    function record.destroy()
        for i = record.Count - 1, 0, -1 do record.Child[i]:destroy() end
        if record.Parent then
            local parent = record.Parent
            for i = 0, parent.Count - 1 do
                if parent.Child[i] == record then
                    for j = i, parent.Count - 2 do parent.Child[j] = parent.Child[j + 1] end
                    parent.Child[parent.Count - 1] = nil
                    parent.Count = parent.Count - 1
                    break
                end
            end
        end
        record.destroyed = true
        refresh_address_list()
    end
    function record.disableWithoutExecute() record.Active = false; record.disabled = true end
    function record.getDescription() return record.Description end
    function record.setDescription(self_or_value, optional_value) record.Description = optional_value or self_or_value end
    function record.getAddress()
        if record.OffsetCount > 0 then return record.Address, record.Offset end
        return record.Address
    end
    function record.setAddress(self_or_value, optional_value, optional_offsets)
        local value = optional_value or self_or_value
        local offsets = optional_value and optional_offsets or nil
        record.Address, record.AddressString = value, value
        record.CurrentAddress = address_of(value)
        if offsets then
            record.OffsetCount = #offsets
            for i, offset in ipairs(offsets) do record.Offset[i - 1] = offset end
        end
    end
    function record.getOffsetCount() return record.OffsetCount end
    function record.setOffsetCount(self_or_count, optional_count) record.OffsetCount = optional_count or self_or_count end
    function record.getOffset(self_or_index, optional_index) return record.Offset[optional_index or self_or_index] end
    function record.setOffset(self_or_index, index_or_value, optional_value)
        local index, value = optional_value and index_or_value or self_or_index, optional_value or index_or_value
        record.Offset[index] = value
    end
    function record.getCurrentAddress() return address_of(record.Address) end
    function record.beginEdit() record.editing = true end
    function record.endEdit() record.editing = false end
    table.insert(state.records, record)
    refresh_address_list()
    return record
end
function address_list.getCount() return address_list.Count end
function address_list.getMemoryRecord(self_or_index, optional_index)
    return address_list.MemoryRecord[optional_index or self_or_index]
end
function address_list.getMemoryRecordByDescription(self_or_description, optional_description)
    local description = optional_description or self_or_description
    for _, record in ipairs(state.records) do
        if record.Description == description and not record.destroyed then return record end
    end
end
function address_list.getMemoryRecordsWithDescription(self_or_description, optional_description)
    local description, matches = optional_description or self_or_description, {}
    for _, record in ipairs(state.records) do
        if record.Description == description and not record.destroyed then matches[#matches + 1] = record end
    end
    return matches
end
function address_list.getMemoryRecordByID(self_or_id, optional_id)
    local id = optional_id or self_or_id
    for _, record in ipairs(state.records) do
        if record.ID == id and not record.destroyed then return record end
    end
end
function address_list.getSelectedRecords()
    local selected = {}
    for _, record in ipairs(state.records) do
        if record.Selected and not record.destroyed then selected[#selected + 1] = record end
    end
    return selected
end
function address_list.getSelectedRecord() return address_list.SelectedRecord end
function address_list.setSelectedRecord(self_or_record, optional_record)
    local selected = optional_record or (self_or_record ~= address_list and self_or_record or nil)
    for _, record in ipairs(state.records) do record.Selected = record == selected end
    address_list.SelectedRecord = selected
    address_list.SelCount = selected and 1 or 0
end
function address_list.disableAllWithoutExecute()
    for _, record in ipairs(state.records) do
        if not record.destroyed then record:disableWithoutExecute() end
    end
end
function address_list.rebuildDescriptionCache() end
_G.AddressList = address_list
_G.getAddressList = function() return address_list end
_G.beep = function() state.beep_count = state.beep_count + 1 end

_G.showMessage = function(text) state.messages[#state.messages + 1] = tostring(text) end
_G.inputQuery = function(caption, prompt, initialstring) return state.input_result end
_G.messageDialog = function(...)
    state.messages[#state.messages + 1] = { ... }
    return state.dialog_result or mrOk
end

---Starts Cheat Engine's Mono data collector.
---@return boolean success
_G.LaunchMonoDataCollector = function() return true end

-- Mono-specific mocks

---Reports whether the Mono collector is connected to the current process.
---@return boolean valid
_G.mono_isValid = function() return true end

---Reads an invocation result from the Mono pipe.
---@return any result
---@return MonoType type
_G.mono_readObject = function() return 0, 0 end

---@param method MethodId
---@return MethodAttribute flags
_G.mono_method_getFlags = function(method) return 0 end

---Returns the raw parameter and return-type metadata for a method.
---@param method MethodId
---@return MonoMethodParameters?
_G.mono_method_get_parameters = function(method)
    return { parameters = {}, returnmonotype = 0, returntype = MONO_TYPE_VOID }
end

---@param method MethodId
---@return Address
_G.mono_compile_method = function(method) return 0xdeadbeef end

---Invokes a Mono method using Cheat Engine's public invocation boundary.
---@param domain DomainId?
---@param method MethodId
---@param object MonoObject
---@param args table
---@return any result
---@return string? exception
---@return MonoType? type
_G.mono_invoke_method = function(domain, method, object, args) return nil, nil, nil end

---@param method MethodId
---@return ClassId
_G.mono_method_getClass = function(method) return 0 end

---@param class ClassId
---@return Address monotype
_G.mono_class_get_type = function(class) return class end

---@param object MonoObject
---@return MonoObject unboxedObject
_G.mono_object_unbox = function(object) return object end

---@param object MonoObject
---@return table<string, any>?
_G.mono_object_enumValues = function(object) return nil end

---@param assembly AssemblyId
---@return AssemblyImage?
_G.mono_getImageFromAssembly = function(assembly) return 0 end

---@param image AssemblyImage
---@return string? name
---@return string? error
_G.mono_image_get_name = function(image) return "mock_image" end

---@param image AssemblyImage
---@return MonoClassInfo[]
_G.mono_image_enumClassesEx = function(image) return {} end

---@return AssemblyId[] assemblies
_G.mono_enumAssemblies = function() return {} end

---@param method MethodId
---@return string name
_G.mono_method_getName = function(method) return "mock_method" end

---@param method MethodId
---@return string parameterTypes Comma-separated parameter type names
---@return string[] parameterNames
---@return string returnType
_G.mono_method_getSignature = function(method) return "", {}, "void" end

---@param method MethodId
---@return string name
_G.mono_method_getFullName = function(method) return "mock_class.mock_method()" end

---@param types string Comma-separated Mono type names
---@return string[] types
_G.mono_splitParameters = function(types) return {} end

---@param class ClassId
---@return ClassId parentId
_G.mono_class_getParent = function(class) return 0 end

---@param class ClassId
---@return string className
_G.mono_class_getName = function(class) return "mock_class" end

---@param class ClassId
---@return string namespace
_G.mono_class_getNamespace = function(class) return "mock_namespace" end

---@param class ClassId
---@param includeParents boolean?
---@param expandedStructs boolean?
---@return MonoFieldInfo[]
_G.mono_class_enumFields = function(class, includeParents, expandedStructs) return {} end

---@param fields MonoFieldInfo[]
---@return integer startOffset
_G.mono_structfields_getStartOffset = function(fields) return 0 end

---@param class ClassId
---@param field FieldId
---@return any value
_G.mono_class_getStaticFieldValue = function(class, field) return 0 end

---@param class ClassId
---@param includeParents boolean?
---@return MonoMethodInfo[]
_G.mono_class_enumMethods = function(class, includeParents) return {} end

---@param vartype VirtualType
---@param value any
---@return boolean success
_G.mono_writeObject = function(vartype, value) return true end

---@param monotype MonoTypeId
---@return MonoType type
_G.mono_type_get_type = function(monotype) return MONO_TYPE_END end

--# CE table mocks

---@param symbol string
---@param address Address
---@param donotsave boolean?
_G.registerSymbol = function(symbol, address, donotsave)
    state.symbols[symbol] = address_of(address)
    state.symbol_options[symbol] = { donotsave = donotsave == true }
end

---@param symbol string
_G.unregisterSymbol = function(symbol)
    state.symbols[symbol], state.symbol_options[symbol] = nil, nil
end

---@return RegisteredSymbol[]
_G.enumRegisteredSymbols = function()
    local symbols = {}
    for symbol, address in pairs(state.symbols) do
        symbols[#symbols + 1] = {
            symbolname = symbol, address = address,
            donotsave = state.symbol_options[symbol].donotsave,
            allocsize = state.allocations[address],
        }
    end
    table.sort(symbols, function(a, b) return a.symbolname < b.symbolname end)
    return symbols
end

_G.deleteAllRegisteredSymbols = function()
    state.symbols, state.symbol_options = {}, {}
end

---@return StringList
_G.AOBScan = function(...)
    local args = { ... }
    local pattern = type(args[1]) == "string" and args[1] or table.concat(args, " ")
    local results = state.aob_results[pattern] or {}
    local strings = {}
    for _, address in ipairs(results) do strings[#strings + 1] = string.format("%X", address) end
    return create_string_list(strings)
end

_G.AOBScanUnique = function(pattern, protectionflags, alignmenttype, alignmentparam)
    local results = state.aob_results[pattern]
    return results and results[1] or nil
end

_G.AOBScanModuleUnique = function(module, pattern, protectionflags, alignmenttype, alignmentparam)
    local results = state.aob_results[module .. ":" .. pattern] or state.aob_results[pattern]
    return results and results[1] or nil
end

local function parse_auto_assembler(text)
    local info = { allocs = {}, registeredsymbols = {}, exceptionlist = {}, symbols = {} }
    for name, size, preferred in text:gmatch("[Aa][Ll][Ll][Oo][Cc]%s*%(%s*([%w_.$]+)%s*,%s*([^,%s%)]+)%s*,?%s*([^%s%)]*)") do
        local parsed_size = tonumber(size) or tonumber(size, 16) or 0x1000
        local parsed_preferred = preferred ~= "" and address_of(preferred) or nil
        local address = allocateMemory(parsed_size, parsed_preferred)
        info.allocs[name] = { address = address, size = parsed_size, prefered = parsed_preferred }
        info.symbols[name] = address
    end
    for name in text:gmatch("[Rr][Ee][Gg][Ii][Ss][Tt][Ee][Rr][Ss][Yy][Mm][Bb][Oo][Ll]%s*%(%s*([%w_.$]+)%s*%)") do
        local address = info.symbols[name] or state.symbols[name] or 0
        registerSymbol(name, address)
        info.registeredsymbols[#info.registeredsymbols + 1] = name
    end
    return info
end

---@return boolean success
---@return AutoAssemblerDisableInfo
_G.autoAssemble = function(...)
    local text, targetself_or_disableinfo, disableinfo = ...
    local old = type(targetself_or_disableinfo) == "table" and targetself_or_disableinfo or disableinfo
    if old then
        for _, symbol in ipairs(old.registeredsymbols or {}) do unregisterSymbol(symbol) end
        for _, allocation in pairs(old.allocs or {}) do deAlloc(allocation.address, allocation.size) end
        return true, old --[[@as AutoAssemblerDisableInfo]]
    end
    return true, parse_auto_assembler(text)
end

_G.autoAssembleCheck = function(text, enable, targetself)
    if type(text) ~= "string" then return false, "script must be a string" end
    return true
end

--# Constants
_G.MONOCMD_INVOKEMETHOD = 1

---@type MessageDialogType
_G.mtWarning = 0
---@type MessageDialogType
_G.mtError = 1
---@type MessageDialogType
_G.mtInformation = 2
---@type MessageDialogType
_G.mtConfirmation = 3
---@type MessageDialogButton
_G.mbYes = 0
---@type MessageDialogButton
_G.mbNo = 1
---@type MessageDialogButton
_G.mbOK = 2
---@type MessageDialogButton
_G.mbCancel = 3
---@type ModalResult
_G.mrNone = 0
---@type ModalResult
_G.mrOk = 1
---@type ModalResult
_G.mrCancel = 2
---@type ModalResult
_G.mrAbort = 3
---@type ModalResult
_G.mrRetry = 4
---@type ModalResult
_G.mrIgnore = 5
---@type ModalResult
_G.mrYes = 6
---@type ModalResult
_G.mrNo = 7

--# Table-based globals
_G.libmono = {
    monopipe = {
        writeByte = function(b) return true end,
        writeQword = function(q) return true end,
        readByte = function() return 0 end,
        readWord = function() return 0 end,
        readString = function(len) return "" end,
    }
}

-- Additional symbols found during scan
_G.monopipe = _G.libmono.monopipe
