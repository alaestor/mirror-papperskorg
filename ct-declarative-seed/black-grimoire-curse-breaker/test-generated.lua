local generated_path, mock_path = ...

assert(generated_path, "generated table path is required")
assert(mock_path, "Cheat Engine mock path is required")

dofile(mock_path)
local roots = dofile(generated_path)

local function count(records)
    local total = 0
    for _, record in ipairs(records) do
        total = total + 1
        for index = 0, record.Count - 1 do
            total = total + count({ record.Child[index] })
        end
    end
    return total
end

assert(#roots == 2, "expected hello-world and Enable roots")
assert(count(roots) == 68, "expected 68 records")
assert(_G.alce == nil, "table construction must not load ALCE")

local hello = getAddressList():getMemoryRecordByDescription("print hello world")
assert(hello ~= nil, "hello-world record is missing")
assert(hello.Script:find("print%('Hello World'%)"), "hello-world script was not embedded")

local name = getAddressList():getMemoryRecordByDescription("Name")
assert(name ~= nil, "Name record is missing")
assert(name.OffsetText[0] == "alce.mono.T.string.index_from")
assert(name.OffsetText[1] == "mc.Player.offset.name")
assert(name.String.Size == 10)
assert(name.String.Unicode == true)

local selected_item = getAddressList():getMemoryRecordByDescription("Selected Item")
assert(selected_item ~= nil, "Selected Item record is missing")

local blank_headers = 0
for index = 0, getAddressList().Count - 1 do
    local record = getAddressList().MemoryRecord[index]
    if record.IsGroupHeader and record.Description == "" then
        blank_headers = blank_headers + 1
    end
end
assert(blank_headers == 1, "expected one blank spacer header")
