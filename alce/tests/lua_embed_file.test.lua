local test = require("test_utils")
local lua_embed_file = require("lua_embed_file")

local function transform(source, files)
    local reads = {}
    local generated = lua_embed_file.transform({
        source = source,
        source_name = "template.lua",
        root = "/project",
        read_file = function(path)
            reads[path] = (reads[path] or 0) + 1
            local content = files[path]
            if content == nil then
                return nil, "not found"
            end
            return content
        end,
    })
    return generated, reads
end

local function evaluate(source)
    local chunk, load_error = load(source, "generated", "t", {})
    if chunk == nil then
        error(load_error)
    end
    return chunk()
end

local function expect_error_contains(fn, expected)
    local ok, message = pcall(fn)
    test.expect(ok).to_be_false()
    test.expect(tostring(message):find(expected, 1, true) ~= nil).to_be_true()
end

test.run_and_report(function()
    test.describe("lua file embedding", function()
        test.it("should replace a marker with the referenced file contents", function()
            local generated = transform(
                'return __EMBED_FILE__("scripts/example.cea")',
                { ["/project/scripts/example.cea"] = "define(value, 42)\n[ENABLE]\nvalue:\n" }
            )

            test.expect(evaluate(generated)).to_eq("define(value, 42)\n[ENABLE]\nvalue:\n")
        end)

        test.it("should preserve leading newlines and normalize line endings", function()
            local generated = transform(
                'return __EMBED_FILE__("example.txt")',
                { ["/project/example.txt"] = "\r\nfirst\rsecond\n\rthird" }
            )

            test.expect(evaluate(generated)).to_eq("\nfirst\nsecond\nthird")
        end)

        test.it("should choose a delimiter that cannot close inside the content", function()
            local content = "plain ]] and nested ]=] and deeper ]==]"
            local generated = transform(
                'return __EMBED_FILE__("brackets.txt")',
                { ["/project/brackets.txt"] = content }
            )

            test.expect(evaluate(generated)).to_eq(content)
        end)

        test.it("should support multiple markers and cache repeated files", function()
            local generated, reads = transform(
                'return __EMBED_FILE__("a.txt"), __EMBED_FILE__("a.txt"), __EMBED_FILE__("b.txt")',
                {
                    ["/project/a.txt"] = "alpha",
                    ["/project/b.txt"] = "beta",
                }
            )
            local first, second, third = evaluate(generated)

            test.expect(first).to_eq("alpha")
            test.expect(second).to_eq("alpha")
            test.expect(third).to_eq("beta")
            test.expect(reads["/project/a.txt"]).to_eq(1)
            test.expect(reads["/project/b.txt"]).to_eq(1)
        end)

        test.it("should ignore marker text in strings and comments", function()
            local source = [==[
local short = '__EMBED_FILE__("short.txt")'
local long = [=[__EMBED_FILE__("long.txt")]=]
-- __EMBED_FILE__("line.txt")
--[[
__EMBED_FILE__("block.txt")
]]
return short, long
]==]
            local generated, reads = transform(source, {})
            local short, long = evaluate(generated)

            test.expect(short).to_eq('__EMBED_FILE__("short.txt")')
            test.expect(long).to_eq('__EMBED_FILE__("long.txt")')
            test.expect(next(reads)).to_eq(nil)
        end)

        test.it("should normalize paths that remain inside the root", function()
            local generated = transform(
                'return __EMBED_FILE__("./scripts/old/../example.txt")',
                { ["/project/scripts/example.txt"] = "content" }
            )

            test.expect(evaluate(generated)).to_eq("content")
        end)

        test.it("should report source locations for unreadable files", function()
            expect_error_contains(function()
                transform('\nreturn __EMBED_FILE__("missing.txt")', {})
            end, "template.lua:2:8:")
        end)

        test.it("should reject paths outside the configured root", function()
            expect_error_contains(function()
                transform('return __EMBED_FILE__("../outside.txt")', {})
            end, "embedded path escapes --root")

            expect_error_contains(function()
                transform('return __EMBED_FILE__("/absolute.txt")', {})
            end, "embedded path must be relative")
        end)

        test.it("should reject dynamic marker arguments and NUL bytes", function()
            expect_error_contains(function()
                transform("return __EMBED_FILE__(path)", {})
            end, "path must be a string literal")

            expect_error_contains(function()
                transform(
                    'return __EMBED_FILE__("binary.dat")',
                    { ["/project/binary.dat"] = "before\0after" }
                )
            end, "must not contain NUL bytes")
        end)

        test.it("should reject executable uses of the reserved identifier", function()
            expect_error_contains(function()
                transform("return __EMBED_FILE__", {})
            end, "must be called with one string literal")
        end)
    end)
end)
