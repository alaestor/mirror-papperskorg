---@class CtIdAllocation
---@field entries table<CtEntry, integer>
---@field hotkeys table<CtHotkey, integer>

---@class CtIds
local ids = {}

---@param used table<integer, boolean>
---@param id integer
---@param label string
local function reserve(used, id, label)
  if used[id] then
    error(string.format("duplicate %s ID: %d", label, id), 0)
  end
  used[id] = true
end

---@param entries CtEntry[]
---@param callback fun(entry: CtEntry)
local function visit(entries, callback)
  for _, entry in ipairs(entries) do
    callback(entry)
    visit(entry.children, callback)
  end
end

---@param used table<integer, boolean>
---@param cursor integer
---@return integer, integer
local function next_id(used, cursor)
  while used[cursor] do
    cursor = cursor + 1
  end
  used[cursor] = true
  return cursor, cursor + 1
end

---@param entries CtEntry[]
---@return CtIdAllocation
function ids.allocate(entries)
  local allocation = { entries = {}, hotkeys = {} }
  local used_entries = {}

  visit(entries, function(entry)
    if entry.id ~= nil then
      reserve(used_entries, entry.id, "entry")
      allocation.entries[entry] = entry.id
    end
  end)

  local entry_cursor = 0
  visit(entries, function(entry)
    if allocation.entries[entry] == nil then
      local allocated
      allocated, entry_cursor = next_id(used_entries, entry_cursor)
      allocation.entries[entry] = allocated
    end

    local used_hotkeys = {}
    for _, hotkey in ipairs(entry.hotkeys) do
      if hotkey.id ~= nil then
        reserve(used_hotkeys, hotkey.id, "hotkey")
        allocation.hotkeys[hotkey] = hotkey.id
      end
    end
    local hotkey_cursor = 0
    for _, hotkey in ipairs(entry.hotkeys) do
      if allocation.hotkeys[hotkey] == nil then
        local allocated
        allocated, hotkey_cursor = next_id(used_hotkeys, hotkey_cursor)
        allocation.hotkeys[hotkey] = allocated
      end
    end
  end)

  return allocation
end

return ids
