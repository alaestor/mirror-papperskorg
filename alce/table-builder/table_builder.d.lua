---@meta

---@alias TableBuilderRecordKind "header"|"aa"|"record"

---@class TableBuilderDropdown
---@field items string?
---@field linked_record string?
---@field read_only boolean?
---@field description_only boolean?
---@field display_as_item boolean?

---@class TableBuilderRecord
---@field kind TableBuilderRecordKind
---@field description string
---@field children TableBuilderRecord[]?
---@field properties table<string, boolean|string|number>?
---@field script string?
---@field address string|number?
---@field vtype VirtualType?
---@field offsets (number|string)[]?
---@field dropdown TableBuilderDropdown?

---@class TableBuilderDefinition
---@field records TableBuilderRecord[]
