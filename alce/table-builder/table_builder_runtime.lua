local table_builder = {}

local PROPERTY_RULES = {
    Color = { type = "number" },
    ShowAsHex = { type = "boolean", kinds = { record = true } },
    ShowAsSigned = { type = "boolean", kinds = { record = true } },
    AllowIncrease = { type = "boolean", kinds = { record = true } },
    AllowDecrease = { type = "boolean", kinds = { record = true } },
    Collapsed = { type = "boolean" },
    Options = { type = "string" },
    Async = { type = "boolean", kinds = { aa = true } },
    CustomTypeName = { type = "string", kinds = { record = true }, vartype = "vtCustom" },
    ["String.Size"] = { type = "number", kinds = { record = true }, vartype = "vtString" },
    ["String.Unicode"] = { type = "boolean", kinds = { record = true }, vartype = "vtString" },
    ["String.Codepage"] = { type = "boolean", kinds = { record = true }, vartype = "vtString" },
    ["Binary.Startbit"] = { type = "number", kinds = { record = true }, vartype = "vtBinary" },
    ["Binary.Size"] = { type = "number", kinds = { record = true }, vartype = "vtBinary" },
    ["Aob.Size"] = { type = "number", kinds = { record = true }, vartype = "vtByteArray" },
}

local PROPERTY_ORDER = {
    "Color",
    "ShowAsHex",
    "ShowAsSigned",
    "AllowIncrease",
    "AllowDecrease",
    "Collapsed",
    "Options",
    "Async",
    "CustomTypeName",
    "String.Size",
    "String.Unicode",
    "String.Codepage",
    "Binary.Startbit",
    "Binary.Size",
    "Aob.Size",
}

local COMMON_FIELDS = {
    kind = true,
    description = true,
    children = true,
    properties = true,
}

local KIND_FIELDS = {
    header = {},
    aa = { script = true },
    record = {
        address = true,
        vtype = true,
        offsets = true,
        dropdown = true,
    },
}

local DROPDOWN_FIELDS = {
    items = true,
    linked_record = true,
    read_only = true,
    description_only = true,
    display_as_item = true,
}

local function fail(path, message)
    error(path .. ": " .. message, 0)
end

local function is_nonblank(value)
    return type(value) == "string" and value:match("%S") ~= nil
end

local function validate_array(value, path)
    if type(value) ~= "table" then
        fail(path, "must be an array")
    end
    local count, maximum = 0, 0
    for key in pairs(value) do
        if type(key) ~= "number" or key % 1 ~= 0 or key < 1 then
            fail(path, "must be a dense array")
        end
        count = count + 1
        maximum = math.max(maximum, key)
    end
    if count ~= maximum then
        fail(path, "must be a dense array")
    end
    return count
end

local function validate_properties(node, path)
    local properties = node.properties
    if properties == nil then
        return
    end
    if type(properties) ~= "table" then
        fail(path .. ".properties", "must be a table")
    end
    for name, value in pairs(properties) do
        local property_path = path .. ".properties[" .. string.format("%q", tostring(name)) .. "]"
        if type(name) ~= "string" or PROPERTY_RULES[name] == nil then
            fail(property_path, "is not a supported writable property")
        end
        local rule = PROPERTY_RULES[name]
        if type(value) ~= rule.type then
            fail(property_path, "must be " .. rule.type)
        end
        if rule.kinds and not rule.kinds[node.kind] then
            fail(property_path, "is not valid for " .. node.kind .. " records")
        end
        if rule.vartype then
            local expected = rawget(_G, rule.vartype)
            if expected == nil or node.vtype ~= expected then
                fail(property_path, "requires vtype = " .. rule.vartype)
            end
        end
    end
end

local function validate_dropdown(dropdown, path)
    if type(dropdown) ~= "table" then
        fail(path, "must be a table")
    end
    for key in pairs(dropdown) do
        if not DROPDOWN_FIELDS[key] then
            fail(path .. "." .. tostring(key), "is not a supported dropdown field")
        end
    end
    if dropdown.items ~= nil and type(dropdown.items) ~= "string" then
        fail(path .. ".items", "must be a string")
    end
    if dropdown.linked_record ~= nil and not is_nonblank(dropdown.linked_record) then
        fail(path .. ".linked_record", "must be a nonblank string")
    end
    if dropdown.items ~= nil and dropdown.linked_record ~= nil then
        fail(path, "items and linked_record are mutually exclusive")
    end
    for _, name in ipairs({ "read_only", "description_only", "display_as_item" }) do
        if dropdown[name] ~= nil and type(dropdown[name]) ~= "boolean" then
            fail(path .. "." .. name, "must be boolean")
        end
    end
end

local function validate_node(node, path, seen)
    if type(node) ~= "table" then
        fail(path, "must be a table")
    end
    if seen[node] then
        fail(path, "must not reuse or cycle a record definition")
    end
    seen[node] = true

    if KIND_FIELDS[node.kind] == nil then
        fail(path .. ".kind", "must be \"header\", \"aa\", or \"record\"")
    end
    if type(node.description) ~= "string" then
        fail(path .. ".description", "must be a string")
    end
    for key in pairs(node) do
        if not COMMON_FIELDS[key] and not KIND_FIELDS[node.kind][key] then
            fail(path .. "." .. tostring(key), "is not supported for " .. node.kind .. " records")
        end
    end

    if node.kind == "aa" and not is_nonblank(node.script) then
        fail(path .. ".script", "must be a nonblank string")
    elseif node.kind == "record" then
        if node.address ~= nil and type(node.address) ~= "string" and type(node.address) ~= "number" then
            fail(path .. ".address", "must be a string or number")
        end
        if node.vtype ~= nil and type(node.vtype) ~= "number" then
            fail(path .. ".vtype", "must be a CE vartype number")
        end
        if node.offsets ~= nil then
            local count = validate_array(node.offsets, path .. ".offsets")
            for index = 1, count do
                local offset_type = type(node.offsets[index])
                if offset_type ~= "number" and offset_type ~= "string" then
                    fail(path .. ".offsets[" .. index .. "]", "must be a number or string")
                end
            end
        end
        if node.dropdown ~= nil then
            validate_dropdown(node.dropdown, path .. ".dropdown")
        end
    end

    validate_properties(node, path)
    if node.children ~= nil then
        local count = validate_array(node.children, path .. ".children")
        for index = 1, count do
            validate_node(node.children[index], path .. ".children[" .. index .. "]", seen)
        end
    end
end

local function validate_definition(definition)
    if type(definition) ~= "table" then
        fail("definition", "must be a table")
    end
    for key in pairs(definition) do
        if key ~= "records" then
            fail("definition." .. tostring(key), "is not a supported field")
        end
    end
    local count = validate_array(definition.records, "records")
    local seen = {}
    for index = 1, count do
        validate_node(definition.records[index], "records[" .. index .. "]", seen)
    end
end

local function set_nested_property(record, name, value)
    local owner_name, field_name = name:match("^([^.]+)%.([^.]+)$")
    if owner_name then
        record[owner_name][field_name] = value
    else
        record[name] = value
    end
end

local function apply_properties(record, properties)
    if properties == nil then
        return
    end
    for _, name in ipairs(PROPERTY_ORDER) do
        if properties[name] ~= nil then
            set_nested_property(record, name, properties[name])
        end
    end
end

local function configure_dropdown(record, dropdown)
    if dropdown == nil then
        return
    end
    if dropdown.items ~= nil then
        record.DropDownList.Text = dropdown.items
    elseif dropdown.linked_record ~= nil then
        record.DropDownLinkedMemrec = dropdown.linked_record
    end
    record.DropDownReadOnly = dropdown.read_only == true
    record.DropDownDescriptionOnly = dropdown.description_only == true
    record.DisplayAsDropDownListItem = dropdown.display_as_item ~= false
end

local function create_node(address_list, node, parent, new_roots)
    local record = address_list:createMemoryRecord()
    if parent then
        record:appendToEntry(parent)
    else
        new_roots[#new_roots + 1] = record
    end

    record.Description = node.description
    record.DontSave = false
    if node.kind == "header" then
        record.IsGroupHeader = true
        record.Options = "[moHideChildren,moAllowManualCollapseAndExpand]"
    elseif node.kind == "aa" then
        record.Type = vtAutoAssembler
        record.Script = node.script
    else
        record.Type = node.vtype or vtDword
        if type(node.address) == "number" then
            record.Address = string.format("%X", node.address)
        elseif node.address ~= nil then
            record.Address = node.address
        end
        if node.offsets then
            record.OffsetCount = #node.offsets
            for index, offset in ipairs(node.offsets) do
                if type(offset) == "number" then
                    record.Offset[index - 1] = offset
                else
                    record.OffsetText[index - 1] = offset
                end
            end
        end
        configure_dropdown(record, node.dropdown)
    end
    apply_properties(record, node.properties)

    for _, child in ipairs(node.children or {}) do
        create_node(address_list, child, record, new_roots)
    end
    return record
end

local function snapshot_roots(address_list)
    local roots = {}
    for index = 0, address_list:getCount() - 1 do
        local record = address_list:getMemoryRecord(index)
        if record and record.Parent == nil then
            roots[#roots + 1] = record
        end
    end
    return roots
end

local function destroy_roots(roots)
    for index = #roots, 1, -1 do
        roots[index]:destroy()
    end
end

---Build a declarative memory-record hierarchy in the current CE address list.
---@param definition TableBuilderDefinition
---@return MemoryRecord[]
function table_builder.build(definition)
    validate_definition(definition)
    local address_list = getAddressList()
    local old_roots = snapshot_roots(address_list)
    local new_roots = {}
    local ok, result = pcall(function()
        for _, node in ipairs(definition.records) do
            create_node(address_list, node, nil, new_roots)
        end
        return new_roots
    end)
    if not ok then
        destroy_roots(new_roots)
        error(result, 0)
    end
    destroy_roots(old_roots)
    address_list:rebuildDescriptionCache()
    return result
end

return table_builder
