local alce = require("globals")

---@class MonoscriptGroup Describes a group of mono constants (monotype, fieldAttribute, methodAttribute)
---@field prefix string The string prefix of the global constants in this group
---@field prefixLen number Length of `prefix`
---@field names string[] Alphabetically sorted array of global constant names
---@field nameLookup table<number, string> : Lookup table mapping constant values to their names

-- Collection related to global constants defined in CE's `monoscript.lua`.
-- Groups: `monotype`, `fieldAttribute`, and `methodAttribute`.
-- Each group has a prefix, names, and a value-to-name lookup table.
alce.monoscript = {}

-- declare groups
for _, v in pairs({
    { name = 'monotype',        prefix = 'MONO_TYPE_' },
    { name = 'fieldAttribute',  prefix = 'FIELD_ATTRIBUTE_' },
    { name = 'methodAttribute', prefix = 'METHOD_ATTRIBUTE_' }
}) do
    alce.monoscript[v.name] = {}
    ---@type MonoscriptGroup
    local t = alce.monoscript[v.name]
    t.prefix = v.prefix
    t.prefixLen = string.len(t.prefix)
    t.names = {}
    t.nameLookup = {}
end

-- map globals to groups
for k, v in pairs(_G) do
    if type(k) == 'string' then
        ---@type MonoscriptGroup?
        for _, group in pairs(alce.monoscript) do
            if k:sub(1, group.prefixLen) == group.prefix then
                table.insert(group.names, k)
                group.nameLookup[v] = k
            end
        end
    end
end

-- sort name arrays alphabetically
---@type MonoscriptGroup?
for _, group in pairs(alce.monoscript) do
    if type(group) == 'table' and group.names then
        table.sort(group.names)
    end
end

return alce.monoscript
