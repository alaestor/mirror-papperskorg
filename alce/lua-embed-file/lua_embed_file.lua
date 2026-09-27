local lua_embed_file = {}

local MARKER = "__EMBED_FILE__"

local function location(source, position)
    local prefix = source:sub(1, position - 1)
    local _, newlines = prefix:gsub("\n", "")
    local line_start = prefix:match(".*\n()") or 1
    return newlines + 1, position - line_start + 1
end

local function fail(source, source_name, position, message)
    local line, column = location(source, position)
    error(string.format("%s:%d:%d: %s", source_name, line, column, message), 0)
end

local function long_bracket_at(source, position)
    local equals = source:sub(position):match("^%[(=*)%[")
    if equals == nil then
        return nil
    end
    return equals, #equals + 2
end

local function scan_short_string(source, position, source_name)
    local quote = source:sub(position, position)
    local cursor = position + 1
    while cursor <= #source do
        local char = source:sub(cursor, cursor)
        if char == "\\" then
            cursor = cursor + 2
        elseif char == quote then
            return cursor + 1
        elseif char == "\n" or char == "\r" then
            fail(source, source_name, position, "unterminated string literal")
        else
            cursor = cursor + 1
        end
    end
    fail(source, source_name, position, "unterminated string literal")
end

local function scan_long_bracket(source, position, source_name)
    local equals, opener_length = long_bracket_at(source, position)
    if equals == nil then
        return nil
    end
    local closing = "]" .. equals .. "]"
    local closing_start = source:find(closing, position + opener_length, true)
    if closing_start == nil then
        fail(source, source_name, position, "unterminated long bracket")
    end
    return closing_start + #closing
end

local function scan_comment(source, position, source_name)
    local long_start = position + 2
    if long_bracket_at(source, long_start) ~= nil then
        return scan_long_bracket(source, long_start, source_name)
    end
    local newline = source:find("[\r\n]", long_start)
    return newline or (#source + 1)
end

local function skip_whitespace(source, position)
    local cursor = position
    while source:sub(cursor, cursor):match("%s") do
        cursor = cursor + 1
    end
    return cursor
end

local function decode_path_literal(source, start_position, end_position, source_name)
    local literal = source:sub(start_position, end_position - 1)
    local chunk, load_error = load("return " .. literal, "embed path", "t", {})
    if chunk == nil then
        fail(source, source_name, start_position, "invalid embedded path: " .. load_error)
    end
    local executable = assert(chunk)
    local ok, value = pcall(executable)
    if not ok or type(value) ~= "string" then
        fail(source, source_name, start_position, "embedded path must be a string literal")
    end
    return value
end

local function normalize_embedded_path(path)
    if path == "" then
        return nil, "embedded path must not be empty"
    end
    if path:match("^/") or path:match("^%a:[/\\]") then
        return nil, "embedded path must be relative to --root"
    end

    local parts = {}
    for part in (path .. "/"):gmatch("(.-)/") do
        if part == ".." then
            if #parts == 0 then
                return nil, "embedded path escapes --root"
            end
            table.remove(parts)
        elseif part ~= "" and part ~= "." then
            parts[#parts + 1] = part
        end
    end
    if #parts == 0 then
        return nil, "embedded path must name a file"
    end
    return table.concat(parts, "/")
end

local function join_path(root, path)
    if root:sub(-1) == "/" then
        return root .. path
    end
    return root .. "/" .. path
end

local function read_file(path)
    local file, open_error = io.open(path, "rb")
    if file == nil then
        return nil, open_error
    end
    local content = file:read("*a")
    local ok, close_error = file:close()
    if not ok then
        return nil, close_error
    end
    return content
end

local function lua_long_string(content)
    content = content:gsub("\r\n", "\n"):gsub("\n\r", "\n"):gsub("\r", "\n")
    if content:find("\0", 1, true) then
        return nil, "embedded files must not contain NUL bytes"
    end

    local equals_count = 0
    while content:find("]" .. string.rep("=", equals_count) .. "]", 1, true) do
        equals_count = equals_count + 1
    end
    local equals = string.rep("=", equals_count)
    local opener = "[" .. equals .. "["
    local closer = "]" .. equals .. "]"
    if content:sub(1, 1) == "\n" then
        return string.format("(%sx%s):sub(2)", opener, content .. closer)
    end
    return opener .. content .. closer
end

local function parse_marker(source, position, identifier_end, source_name)
    local cursor = skip_whitespace(source, identifier_end)
    if source:sub(cursor, cursor) ~= "(" then
        fail(source, source_name, position, MARKER .. " must be called with one string literal")
    end
    cursor = skip_whitespace(source, cursor + 1)
    local quote = source:sub(cursor, cursor)
    if quote ~= "'" and quote ~= '"' then
        fail(source, source_name, cursor, MARKER .. " path must be a string literal")
    end
    local literal_end = scan_short_string(source, cursor, source_name)
    local path = decode_path_literal(source, cursor, literal_end, source_name)
    local closing = skip_whitespace(source, literal_end)
    if source:sub(closing, closing) ~= ")" then
        fail(source, source_name, closing, MARKER .. " accepts exactly one argument")
    end
    return path, closing + 1
end

---@param args { source: string, root: string, source_name?: string, read_file?: fun(path: string): string?, string? }
---@return string
function lua_embed_file.transform(args)
    assert(type(args) == "table", "transform expects an argument table")
    assert(type(args.source) == "string", "source must be a string")
    assert(type(args.root) == "string" and args.root ~= "", "root must be a non-empty string")

    local source = args.source
    local source_name = args.source_name or "<input>"
    local file_reader = args.read_file or read_file
    local cache = {}
    local output = {}
    local cursor = 1

    while cursor <= #source do
        local char = source:sub(cursor, cursor)
        local token_end
        if char == "'" or char == '"' then
            token_end = scan_short_string(source, cursor, source_name)
        elseif char == "[" then
            token_end = scan_long_bracket(source, cursor, source_name)
        elseif char == "-" and source:sub(cursor, cursor + 1) == "--" then
            token_end = scan_comment(source, cursor, source_name)
        end

        if token_end ~= nil then
            output[#output + 1] = source:sub(cursor, token_end - 1)
            cursor = token_end
        elseif char:match("[%a_]") then
            local identifier_end = cursor + 1
            while source:sub(identifier_end, identifier_end):match("[%w_]") do
                identifier_end = identifier_end + 1
            end
            local identifier = source:sub(cursor, identifier_end - 1)
            if identifier ~= MARKER then
                output[#output + 1] = identifier
                cursor = identifier_end
            else
                local embedded_path, marker_end = parse_marker(source, cursor, identifier_end, source_name)
                local normalized_path, path_error = normalize_embedded_path(embedded_path)
                if normalized_path == nil then
                    fail(source, source_name, cursor, path_error)
                end
                local resolved_path = join_path(args.root, normalized_path)
                local content = cache[resolved_path]
                if content == nil then
                    local read_error
                    content, read_error = file_reader(resolved_path)
                    if content == nil then
                        fail(source, source_name, cursor,
                            string.format("cannot embed %q (%s): %s", embedded_path, resolved_path, read_error or "read failed"))
                    end
                    cache[resolved_path] = content
                end
                local literal, literal_error = lua_long_string(content)
                if literal == nil then
                    fail(source, source_name, cursor,
                        string.format("cannot embed %q: %s", embedded_path, literal_error))
                end
                output[#output + 1] = literal
                cursor = marker_end
            end
        else
            output[#output + 1] = char
            cursor = cursor + 1
        end
    end

    return table.concat(output)
end

local function usage()
    return [[Usage: lua-embed-file --root DIR [-o OUTPUT] INPUT

Replace __EMBED_FILE__("relative/path") calls in INPUT with Lua long strings.
Generated Lua is written to stdout unless -o or --output is supplied.]]
end

local function parse_args(argv)
    local options = {}
    local positional = {}
    local cursor = 1
    while cursor <= #argv do
        local value = argv[cursor]
        if value == "-h" or value == "--help" then
            options.help = true
        elseif value == "--root" then
            cursor = cursor + 1
            options.root = argv[cursor]
            if options.root == nil then
                return nil, "--root requires a directory"
            end
        elseif value:match("^%-%-root=") then
            options.root = value:sub(8)
        elseif value == "-o" or value == "--output" then
            cursor = cursor + 1
            options.output = argv[cursor]
            if options.output == nil then
                return nil, value .. " requires a file"
            end
        elseif value == "--" then
            for index = cursor + 1, #argv do
                positional[#positional + 1] = argv[index]
            end
            break
        elseif value:sub(1, 1) == "-" then
            return nil, "unknown option: " .. value
        else
            positional[#positional + 1] = value
        end
        cursor = cursor + 1
    end

    if options.help then
        return options
    end
    if options.root == nil or options.root == "" then
        return nil, "--root is required"
    end
    if #positional ~= 1 then
        return nil, "exactly one input file is required"
    end
    options.input = positional[1]
    local function normalized_cli_path(path)
        local absolute = path:sub(1, 1) == "/"
        local parts = {}
        for part in (path .. "/"):gmatch("(.-)/") do
            if part == ".." and #parts > 0 and parts[#parts] ~= ".." then
                table.remove(parts)
            elseif part ~= "" and part ~= "." then
                parts[#parts + 1] = part
            end
        end
        return (absolute and "/" or "") .. table.concat(parts, "/")
    end
    if options.output ~= nil and normalized_cli_path(options.output) == normalized_cli_path(options.input) then
        return nil, "output must not overwrite the input template"
    end
    return options
end

local function atomic_write(path, content)
    local temporary
    for suffix = 1, 1000 do
        local candidate = path .. ".tmp." .. suffix
        local existing = io.open(candidate, "rb")
        if existing == nil then
            temporary = candidate
            break
        end
        existing:close()
    end
    if temporary == nil then
        return nil, "could not allocate a temporary output file"
    end

    local file, open_error = io.open(temporary, "wb")
    if file == nil then
        return nil, open_error
    end
    local written, write_error = file:write(content)
    if not written then
        file:close()
        os.remove(temporary)
        return nil, write_error
    end
    local closed, close_error = file:close()
    if not closed then
        os.remove(temporary)
        return nil, close_error
    end
    local renamed, rename_error = os.rename(temporary, path)
    if not renamed then
        os.remove(temporary)
        return nil, rename_error
    end
    return true
end

---@param argv string[]
---@param streams? { stdout?: file*, stderr?: file* }
---@return boolean
function lua_embed_file.run(argv, streams)
    streams = streams or {}
    local stdout = streams.stdout or io.stdout
    local stderr = streams.stderr or io.stderr
    local options, argument_error = parse_args(argv)
    if options == nil then
        stderr:write("lua-embed-file: " .. argument_error .. "\n\n" .. usage() .. "\n")
        return false
    end
    if options.help then
        stdout:write(usage() .. "\n")
        return true
    end

    local source, source_error = read_file(options.input)
    if source == nil then
        stderr:write(string.format("lua-embed-file: cannot read %s: %s\n", options.input, source_error))
        return false
    end
    local ok, generated = pcall(lua_embed_file.transform, {
        source = source,
        source_name = options.input,
        root = options.root,
    })
    if not ok then
        stderr:write("lua-embed-file: " .. generated .. "\n")
        return false
    end

    if options.output == nil then
        stdout:write(generated)
        return true
    end
    local written, write_error = atomic_write(options.output, generated)
    if not written then
        stderr:write(string.format("lua-embed-file: cannot write %s: %s\n", options.output, write_error))
        return false
    end
    return true
end

return lua_embed_file
