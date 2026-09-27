local validators = require("validators")
local fnlua = require("fn")
local fn = fnlua.wrap
local Param = fnlua.Param

---Convert a byte value to its 8-bit binary string representation.
---@param byte number
---@return string
local function byte_to_bits(byte)
    local bits = {}
    for i = 7, 0, -1 do bits[#bits + 1] = (byte >> i) & 1 end
    return table.concat(bits)
end

---Formatting utilities for strings, numbers, and tables.
---@class fmt
local fmt = {}

---Convert a string to title case.
---@overload fun(str: string): string
function fmt.titleCase(str)
    return string.gsub(tostring(str), "(%a)([%w']*)", function(first, rest) return first:upper() .. rest:lower() end)
end

---Convert a number to a big-endian binary string in groups of 8.
---@overload fun(args: { value: number, useDouble?: boolean, useNativeEndian?: boolean }): string
fmt.binary = fn({
    params = {
        value = Param.required():validate(function(value) return type(value) == "number" end),
        useDouble = Param.default({ value = false }):validate(function(value) return type(value) == "boolean" end),
        useNativeEndian = Param.default({ value = false }):validate(function(value) return type(value) == "boolean" end),
    },
    body = function(args)
    local value = args.value
    local useDouble = args.useDouble
    local useNativeEndian = args.useNativeEndian
    local endian = useNativeEndian and '' or '>'
    local fmt_str = (math.type(value) == 'integer') and (endian .. 'j') or (endian .. (useDouble and 'd' or 'f'))
    local bytes = string.pack(fmt_str, value)
    local t = {}
    for i = 1, #bytes do t[i] = byte_to_bits(string.byte(bytes, i)) end
    return '0b' .. table.concat(t, ' ')
end,
})

---Lookup table mapping byte values (0–255) to two-character hex strings.
---@type table<number, string>
local BYTE_TO_HEX = {}
for i = 0, 255 do BYTE_TO_HEX[i] = string.format("%02X", i) end

---Convert a number to a hexadecimal string.
---@overload fun(args: { value: number, withPadding?: boolean, useNativeEndian?: boolean }): string
fmt.hex = fn({
    params = {
        value = Param.required():validate(function(value) return type(value) == "number" end),
        withPadding = Param.default({ value = false }):validate(function(value) return type(value) == "boolean" end),
        useNativeEndian = Param.default({ value = false }):validate(function(value) return type(value) == "boolean" end),
    },
    body = function(args)
    local value = args.value
    local withPadding = args.withPadding
    local useNativeEndian = args.useNativeEndian
    local endian = useNativeEndian and '' or '>'
    local fmt_str = (math.type(value) == 'integer') and (endian .. 'j') or (endian .. 'd')
    local bytes = string.pack(fmt_str, value)
    local hex = {}
    for i = 1, #bytes do hex[i] = BYTE_TO_HEX[string.byte(bytes, i)] end
    local result = table.concat(hex)
    if withPadding ~= true then
        result = string.gsub(tostring(result), "^0+", "")
        if result == "" then result = "0" end
    end
    return '0x' .. result
end,
})

---Format a number as a memory address string.
---@overload fun(value: number): string
function fmt.address(value)
    if not validators.isInteger(value) then return 'NaN: ' .. tostring(value) end

    local full = fmt.hex({ value = value, withPadding = true, useNativeEndian = false })
    if targetIs64Bit() then
        return full
    else
        return '0x' .. full:sub(-8)
    end
end

---Replace non-alphanumeric characters in a string with underscores.
---@overload fun(str: string): string
function fmt.sanitizeSymbolName(str)
    return string.gsub(tostring(str), '[^%a%d]', '_')
end

---Return a human-readable single-line representation of a value.
---@overload fun(args: { value: any, usePrintFullTable?: boolean }): string
fmt.pretty = fn({
    params = {
        value = Param.required(),
        usePrintFullTable = Param.default({ value = false }):validate(function(value) return type(value) == "boolean" end),
    },
    body = function(args)
    local value = args.value
    local usePrintFullTable = args.usePrintFullTable
    local t = type(value)
    if t == 'table' and usePrintFullTable then
        return fmt.table({ tbl = value })
    elseif t == 'string' then
        return '"' .. value .. '"'
    elseif t == 'number' then
        return string.format('%s  (%s: %s)', tostring(value), math.type(value), fmt.hex({ value = value }))
    elseif t == 'function' or t == 'thread' or t == 'userdata' then
        return '<' .. tostring(value) .. '>'
    else
        return tostring(value)
    end
end,
})

---Return a human-readable multi-line representation of a table.
---@overload fun(args: { tbl: table, useDepthLimit?: number, useKeysToIgnore?: table, useDontToString?: boolean, useDontSortKeys?: boolean }): string
fmt.table = fn({
    params = {
        tbl = Param.required():validate(validators.isTable),
        useDepthLimit = Param.optional(),
        useKeysToIgnore = Param.default({ value = {} }):validate(validators.isTable),
        useDontToString = Param.default({ value = false }):validate(function(value) return type(value) == "boolean" end),
        useDontSortKeys = Param.default({ value = false }):validate(function(value) return type(value) == "boolean" end),
    },
    body = function(args)
    local tbl = args.tbl
    local useDepthLimit = args.useDepthLimit
    local useKeysToIgnore = args.useKeysToIgnore
    local useDontToString = args.useDontToString
    local useDontSortKeys = args.useDontSortKeys
    local internal_depth = 0
    local internal_seen = {}
    local internal_path = "root"

    local depth = internal_depth
    local seen = internal_seen
    local path = internal_path
    local keysToIgnore = useKeysToIgnore or {}
    local dontToString = useDontToString == true

    if useDepthLimit == depth then return fmt.pretty({ value = tbl }) end
    if seen[tbl] then
        return string.format("<circular reference to %s @ %s>", seen[tbl], string.gsub(tostring(tbl), "table: ", ""))
    end
    seen[tbl] = path
    if not dontToString then
        local mt = getmetatable(tbl)
        if mt and type(mt.__tostring) == "function" then
            local ok, str = pcall(mt.__tostring, tbl)
            if ok then return str end
        end
    end
    local result = {}
    local prefix = string.rep("  ", depth)
    local keys = {}
    for k in pairs(tbl) do if not keysToIgnore[k] then table.insert(keys, k) end end
    if not useDontSortKeys then
        table.sort(keys, function(a, b)
            local ta, tb = type(a), type(b)
            if ta == tb then return a < b end
            return ta < tb
        end)
    end
    for _, k in ipairs(keys) do
        local v = tbl[k]
        local key_str = (type(k) == "string") and k or "[" .. tostring(k) .. "]"
        --? local current_path = path .. "." .. key_str
        local line_prefix = prefix .. key_str .. " = "
        if type(v) ~= "table" then
            table.insert(result, line_prefix .. fmt.pretty({ value = v }))
        else
            if seen[v] then
                table.insert(result,
                    line_prefix ..
                    string.format("<circular reference to %s @ %s>", seen[v], string.gsub(tostring(v), "table: ", "")))
            elseif next(v) == nil then
                table.insert(result, line_prefix .. "{}")
            else
                table.insert(result, line_prefix .. "{")
                table.insert(result, fmt.table({
                    tbl = v,
                    useDepthLimit = useDepthLimit,
                    useKeysToIgnore = keysToIgnore,
                    useDontToString = dontToString,
                    useDontSortKeys = useDontSortKeys,
                }))
                table.insert(result, prefix .. "}")
            end
        end
    end
    return table.concat(result, "\n")
end,
})

return fmt
