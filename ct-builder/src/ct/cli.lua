local compiler = require("ct.compiler")

---@class CtCliOptions
---@field input? string
---@field output? string
---@field root? string
---@field help? boolean

---@class CtCli
local cli = {}

local usage = [[Usage: ct-builder [--root DIR] [-o OUTPUT] INPUT

Compile a Lua Cheat Engine table definition to CE 7.7 XML.
Use - for INPUT to read Lua from standard input. XML is written to
standard output unless -o/--output is provided.
]]

---@param argv string[]
---@return CtCliOptions?, string?
local function parse(argv)
  local options = {}
  local index = 1
  while index <= #argv do
    local argument = argv[index]
    if argument == "-h" or argument == "--help" then
      options.help = true
    elseif argument == "-o" or argument == "--output" then
      index = index + 1
      if argv[index] == nil then
        return nil, argument .. " requires a path"
      end
      options.output = argv[index]
    elseif argument == "--root" then
      index = index + 1
      if argv[index] == nil then
        return nil, "--root requires a directory"
      end
      options.root = argv[index]
    elseif argument:sub(1, 1) == "-" and argument ~= "-" then
      return nil, "unknown option: " .. argument
    elseif options.input ~= nil then
      return nil, "only one input is supported"
    else
      options.input = argument
    end
    index = index + 1
  end
  if not options.help and options.input == nil then
    return nil, "missing INPUT"
  end
  return options
end

---@param path string
---@return string
local function directory(path)
  local result = path:match("^(.*)/[^/]*$")
  if result == nil or result == "" then
    return "."
  end
  return result
end

---@param path string
---@return string
local function normalized_absolute(path)
  if path:sub(1, 1) ~= "/" then
    path = (os.getenv("PWD") or ".") .. "/" .. path
  end
  local parts = {}
  for part in path:gmatch("[^/]+") do
    if part == ".." then
      if #parts > 0 then
        parts[#parts] = nil
      end
    elseif part ~= "." and part ~= "" then
      parts[#parts + 1] = part
    end
  end
  return "/" .. table.concat(parts, "/")
end

---@param path string
---@return string
local function read_file(path)
  local handle, open_error = io.open(path, "rb")
  if not handle then
    error(string.format("cannot read input %q: %s", path, open_error), 0)
  end
  local content, read_error = handle:read("*a")
  handle:close()
  if not content then
    error(string.format("cannot read input %q: %s", path, read_error), 0)
  end
  return content
end

---@param path string
---@param content string
local function atomic_write(path, content)
  local temporary
  for attempt = 1, 100 do
    local candidate = string.format("%s.tmp.%d.%d", path, os.time(), attempt)
    local existing = io.open(candidate, "rb")
    if existing then
      existing:close()
    else
      temporary = candidate
      break
    end
  end
  if not temporary then
    error("cannot allocate a temporary output beside " .. path, 0)
  end

  local handle, open_error = io.open(temporary, "wb")
  if not handle then
    error(string.format("cannot open temporary output %q: %s", temporary, open_error), 0)
  end
  local ok, write_error = handle:write(content)
  local close_ok, close_error = handle:close()
  if not ok or not close_ok then
    os.remove(temporary)
    error(string.format("cannot write output %q: %s", path, write_error or close_error), 0)
  end
  local renamed, rename_error = os.rename(temporary, path)
  if not renamed then
    os.remove(temporary)
    error(string.format("cannot replace output %q: %s", path, rename_error), 0)
  end
end

---@param source_text string
---@param chunk_name string
---@return unknown
local function evaluate(source_text, chunk_name)
  local chunk, load_error = load(source_text, chunk_name, "t", _G)
  if not chunk then
    error(load_error, 0)
  end
  local ok, result = pcall(chunk)
  if not ok then
    error(result, 0)
  end
  return result
end

---@param argv string[]
---@return integer
function cli.run(argv)
  local options, parse_error = parse(argv)
  if not options then
    io.stderr:write("ct-builder: " .. parse_error .. "\n\n" .. usage)
    return 2
  end
  if options.help then
    io.stdout:write(usage)
    return 0
  end
  if options.output and options.input ~= "-"
    and normalized_absolute(options.output) == normalized_absolute(options.input)
  then
    io.stderr:write("ct-builder: input and output must be different files\n")
    return 1
  end

  local ok, result = pcall(function()
    local source_text
    local chunk_name
    if options.input == "-" then
      source_text = io.read("*a")
      chunk_name = "=stdin"
    else
      source_text = read_file(options.input)
      chunk_name = "@" .. options.input
    end
    local definition = evaluate(source_text, chunk_name)
    local base_dir = options.root or (options.input == "-" and "." or directory(options.input))
    return compiler.compile(definition, { base_dir = base_dir })
  end)
  if not ok then
    io.stderr:write("ct-builder: " .. tostring(result) .. "\n")
    return 1
  end

  if options.output then
    local write_ok, write_error = pcall(atomic_write, options.output, result)
    if not write_ok then
      io.stderr:write("ct-builder: " .. tostring(write_error) .. "\n")
      return 1
    end
  else
    io.stdout:write(result)
  end
  return 0
end

return cli
