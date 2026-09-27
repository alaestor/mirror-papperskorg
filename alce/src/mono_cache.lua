local mono_cache = {
    generation = 1,
    processID = nil,
    classesByID = {},
    classesByImage = {},
}

local classTables = setmetatable({}, { __mode = "k" })
local hookInstalled = false

---Invalidates every Mono handle and representation owned by ALCE.
---@param processID number?
function mono_cache.reset(processID)
    mono_cache.generation = mono_cache.generation + 1
    mono_cache.processID = processID
    mono_cache.classesByID = {}
    mono_cache.classesByImage = {}

    for classTable in pairs(classTables) do
        classTable:_invalidateCache()
    end
end

---@param generation integer
---@param operation string
function mono_cache.assertCurrent(generation, operation)
    assert(generation == mono_cache.generation,
        string.format("%s: stale Mono representation; the process changed. Reload the class table", operation))
end

---@param classTable ClassTable
function mono_cache.registerClassTable(classTable)
    classTables[classTable] = true
end

---Installs ALCE's process-open invalidation hook while preserving an existing hook.
function mono_cache.installOpenProcessHook()
    if hookInstalled then return end
    hookInstalled = true

    local previous = MainForm.OnProcessOpened
    ---@diagnostic disable-next-line duplicate-doc-field MainForm.OnProcessOpened
    MainForm.OnProcessOpened = function(processID, processHandle, caption)
        mono_cache.reset(processID)
        if previous then return previous(processID, processHandle, caption) end
    end
end

return mono_cache
