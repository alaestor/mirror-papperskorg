local fmt = require("fmt")
local alce = require("globals")

---@class printers
local printers = {}

---@class alce_warning_local
---@field name string
---@field value any

---@class alce_warning
---@field message string
---@field arguments table
---@field source string
---@field short_source string
---@field line integer
---@field function_name string?
---@field traceback string
---@field locals alce_warning_local[]?

local function stringify(...)
    local args = table.pack(...)
    local result = ""
    for i = 1, args.n do
        local value = args[i]
        local value_type = type(value)
        if value_type == 'string' or value_type == 'number' then
            result = result .. tostring(value)
        else
            result = result .. fmt.pretty({ value = value })
        end
    end
    return result
end

local function capture_warning(...)
    local info = debug.getinfo(3, "nSl") or {}
    local warning = {
        message = stringify(...),
        arguments = table.pack(...),
        source = info.source or "unknown",
        short_source = info.short_src or "unknown",
        line = info.currentline or -1,
        function_name = info.name,
        traceback = debug.traceback(nil, 3),
    }

    if alce.cfg.warning_collect_locals then
        warning.locals = {}
        local index = 1
        while true do
            local name, value = debug.getlocal(3, index)
            if name == nil then break end
            warning.locals[#warning.locals + 1] = { name = name, value = value }
            index = index + 1
        end
    end

    local history = alce.warnings.history
    history[#history + 1] = warning
    local history_size = alce.cfg.warning_history_size
    if type(history_size) ~= "number" or history_size < 1 then history_size = 5 end
    history_size = math.floor(history_size)
    while #history > history_size do table.remove(history, 1) end
    alce.warnings.last = warning

    return warning
end

---Prints a table formatted by fmt.table
---@overload fun(optional_title?: string, tbl: table)
function printers.inspect(optional_title, tbl)
    if type(optional_title) ~= 'string' then
        if tbl == nil then
            if type(optional_title) == 'table' then
                tbl = optional_title
                optional_title = nil
            else
                error('alce.inspect(): invalid arguments')
            end
        end
    end

    local table_to_print = tbl
    if optional_title then
        table_to_print = { [tostring(optional_title)] = tbl }
    end

    print('[INSPECT] ' .. fmt.table({ tbl = table_to_print }))
end

---Prints a sorted array of the table's keys
---@overload fun(optional_title?: string, tbl: table)
function printers.inspectKeys(optional_title, tbl)
    if tbl == nil then
        if type(optional_title) == 'table' then
            tbl = optional_title
            optional_title = nil
        else
            error('alce.inspectKeys(): invalid argument(s)')
        end
    end

    local keys = {}
    for key, _ in pairs(tbl) do table.insert(keys, key) end
    table.sort(keys)

    local table_to_print = keys
    if optional_title then
        table_to_print = { [tostring(optional_title)] = keys }
    end

    print('[INSPECT] ' .. fmt.table({ tbl = table_to_print }))
end

---Pretty-stringifies, concatenates, and prints arguments
---@overload fun(...: any)
function printers.prettyprint(...)
    print(stringify(...))
end

---Prints a message with source line number (when debug_print is enabled)
---@overload fun(...: any)
function printers.debug(...)
    local info = debug.getinfo(2, "Sl")
    if alce.cfg.debug_print then
        printers.prettyprint(string.format("[DEBUG] L%i: ", info.currentline), ...)
    end
end

---Prints a message with source line number (when warn_print is enabled)
---@overload fun(...: any)
function printers.warn(...)
    local warning = capture_warning(...)
    if alce.cfg.warn_print then
        print(string.format('[WARN] L%i: %s', warning.line, warning.message))
    end
end

return printers
