---
include_toc: true
---

- Abandoned CE entirely in favor of my hax framework

# ct-builder

`ct-builder` compiles validated Lua definitions into `.ct` files that aim to be compatible with Cheat Engine 7.7

I was unable to find an explicit schema or first-party documentation regarding the file layout, so the structure was inferred exclusively from example tables serialized by CE 7.7 which have been retained here as test proofs.

## Usage

```sh
nix develop git+https://codeberg.org/alaestor/ct-builder
ct-builder table.lua -o table.ct
# Definitions can also arrive on stdin:
ct-builder - < table.lua > table.ct
```

Paths passed to `ct.file(...)` are relative to the definition file. Use `--root DIR` to override that base; stdin definitions use the current directory. Definitions execute as ordinary Lua.

```lua
local ct = require("ct")

return ct.table({
  auto_attach = "game.exe",
  uses_mono = true,
  lua_script = ct.file("table.lua"),
  entries = {
    ct.aa({
      id = 10,
      description = "Enable",
      sections = {
        header = { lua = ct.file("header.lua") },
        enable = {
          lua = ct.file("enable.lua"),
          asm = ct.file("enable.asm"),
        },
        disable = {
          lua = ct.file("disable.lua"),
          asm = ct.file("disable.asm"),
        },
      },
    }),
    ct.group({
      description = "Player",
      children = {
        ct.value({
          description = "Health",
          value_type = ct.types.float,
          address = "player",
          offsets = { "18", "30" },
        }),
      },
    }),
  },
})
```

Entry definitions must be created by `ct.group`, `ct.value`, or `ct.aa`. Constructors use [fnlua](https://codeberg.org/alaestor/fnlua) to reject misspelled fields and invalid values early. IDs are optional: explicit non-negative IDs are reserved first, and remaining entries receive deterministic depth-first IDs.

An AA entry accepts either `script` for complete source text, or `sections`; set `async = true` to emit Cheat Engine's `Async="1"` script attribute. Structured sections may contain `comment` and `header`, `enable`, and `disable` tables with separate `lua` and `asm` sources. Only populated language blocks are emitted; `[ENABLE]` and `[DISABLE]` are always present. Any text field accepts a literal string or `ct.file("path")`.

## Supported CE 7.7 features

- Table version and options, auto-attach, custom script errors, comments, and global Lua script.
- Nested headers, asynchronous Auto Assembler records, normal addresses, pointers, and all value types demonstrated by the reference tables.
- Display and persistence flags, activation values, group behavior, CE-native six-digit BGR colors, dropdowns, and demonstrated hotkey actions/sounds.
- XML defaults matching CE output by default. Set `emit_defaults = false` on `ct.table` to use experimental aggressive minimization.

`UserdefinedSymbols` is currently emitted empty. Cheat-list `CheatCodes` are unimplemented and omitted, matching a newly serialized empty CE 7.7 table. Unknown fields are intentionally rejected until a CE 7.7-produced example establishes their representation.

## Development and testing

```console
nix flake check
nix build
```

The dev shell includes Lua 5.3, `fnlua` annotations, and LuaLS. Automated tests import XML with [xmllint](https://xmllint.com/).

## AI Policy

Any and all AI usage must be disclosed in communication with this project or its authors: when you contribute a PR, issue, message,... agents must identify themselves and humans must say whether AI was used and to what extent.
