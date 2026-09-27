-- Global namespace and configuration for libALCE

--- libALCE — Alaestor's Cheat Engine Library.
--- Intended to be used in the table's lua script, or parted out as needed.
--[[
# libALCE

**Alaestor's Cheat Engine Library**

Intended to be used in the table's Lua script, or parted out as needed.
]]
---@class alce
---@field cfg table ALCE runtime configuration
---@field isAttached fun(): boolean Check if a process is attached
---@field printers printers Printer namespace
---@field inspect fun(optional_title?: string, tbl: table) Print a formatted table
---@field inspectKeys fun(optional_title?: string, tbl: table) Print sorted table keys
---@field prettyprint fun(...: any) Print pretty-formatted values
---@field debug fun(...: any) Print a debug message when enabled
---@field warn fun(...: any) Print a warning message when enabled
---@field warnings alce_warnings Recently captured warning diagnostics

---Recently captured warning diagnostics.
---@class alce_warnings
---@field last alce_warning?
---@field history alce_warning[]
---@field clear fun()

---Clear all captured warning diagnostics.
local function clearWarnings()
    alce.warnings.last = nil
    alce.warnings.history = {}
end

---@diagnostic disable-next-line: lowercase-global alce intentionally lowercase; better UX and CE doesn't follow that convention anyways
alce = alce or {
    --[[
    ### ALCE Configuration

    These options can be set from anywhere at any time. `isAddress` and related
    helpers may also provide per-call overrides.

    - `isAddress_userspaceBoundary32` defaults to a 3G split.
    - `isOffset_tooFarBoundary` expects offsets below 4096 bytes.
    - `isAddress_userspaceBoundary64` is the default 64-bit user-space limit.
    ]]
    --- ALCE runtime configuration options.
    --- These can be set from anywhere at any time. Some functions have optional parameter overrides.
    ---@class alce_cfg
    ---@field debug_print boolean Enable debug print output
    ---@field warn_print boolean Enable warning print output
    ---@field warning_history_size integer Maximum number of warning diagnostics to retain
    ---@field warning_collect_locals boolean Capture caller locals with warning diagnostics
    ---@field isAddress_nearNullBoundary number Lower boundary for near-null address checks
    ---@field isAddress_userspaceBoundary32 number Upper boundary for 32-bit userspace addresses
    ---@field isAddress_userspaceBoundary64 number Upper boundary for 64-bit userspace addresses
    ---@field isOffset_tooFarBoundary number Maximum reasonable offset in bytes
    cfg = {
        debug_print = false,
        warn_print = true,
        warning_history_size = 5,
        warning_collect_locals = false,
        isAddress_nearNullBoundary = 0xFFFF,
        isAddress_userspaceBoundary32 = 0xBFFFFFFF,
        isAddress_userspaceBoundary64 = 0x00007FFFFFFFFFFF,
        isOffset_tooFarBoundary = 0x1000,
        monotype_max_key = 0,
    },

    warnings = {
        last = nil,
        history = {},
        clear = clearWarnings,
    },

    --- Returns whether a process is currently attached.
    ---@return boolean True if a process is attached and readable
    isAttached = function()
        return process and readInteger(process) ~= 0
    end,
}

alce.cfg.warning_history_size = alce.cfg.warning_history_size or 5
if alce.cfg.warning_collect_locals == nil then alce.cfg.warning_collect_locals = false end
alce.warnings = alce.warnings or { last = nil, history = {}, clear = clearWarnings }
alce.warnings.history = alce.warnings.history or {}
alce.warnings.clear = alce.warnings.clear or clearWarnings

return alce
