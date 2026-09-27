---
include_toc: true
---

- Abandoned CE in favor of my hax framework

# Alaestor's Cheat Engine Library

ALCE is a Lua 5.3 library for building ergonomic and resilient Mono-focused
Cheat Engine tables. It wraps common memory, Mono, type-conversion, formatting, 
and cheat-table workflows behind a modular source tree and ships as a single
`alce.lua` file for use inside Cheat Engine.

The public API is documented alongside the source with LuaLS annotations which
greatly improves the in-editor dev UX when writing scripts that use ALCE. This 
README describes the project, build interface, and module boundaries.

## Requirements

### Consuming releases (for table authors)
- Cheat Engine 7.7 or newer with its Lua 5.3 runtime and Mono APIs for actual
  table use.

`alce.lua` can be added to CE's autorun folder, or its contents can be used as the table script for easier redistribution (recommended: vendoring also prevents versioning issues as ALCE is not yet, and may never be, stable)

### Development and building (for contributors, and me when I forget)
- Nix with flakes enabled for development, building, and testing.
- Optional: A Wayland session and `wl-clipboard` for the `copy-alce` command.

ALCE depends on [fnlua](https://codeberg.org/alaestor/fnlua). Release bundles
preload fnlua, so Cheat Engine consumers only need the generated `alce.lua`.

You can build directly from this repository by using the modern nix command:

```sh
nix build git+https://codeberg.org/alaestor/alce.git#alce
```

## Flake interface

### Building

Build the minified release bundle:

```sh
nix build .#alce
```

Build the readable, non-minified bundle:

```sh
nix build .#alce-full
```

Both packages contain a single `alce.lua`. The default package is the minified
`alce` bundle. The 'full' variant is most useful for correlating line-numbers
when debugging inside of CE.

### Testing

Run the source and packaged test suites:

```sh
nix run .#test
nix run .#test-package
```

`nix flake check` runs both suites as flake checks. During development, use
`path:.` when new files have not yet been added to Git and therefore are not
visible to the normal Git-backed flake source:

```sh
nix run path:.#test-package
```

### Development

Enter the development shell from the repository root:

```sh
nix develop
```

The shell provides Lua, fnlua annotations, `alce-bundle` build script, and the Wayland `copy-alce` QoL dev build script.

#### Manually running the build script

Create `alce.lua` directly in the repository root:

```sh
nix run .#alce-bundler
nix run .#alce-bundler -- --minify
```

#### Build to clipboard

On Linux/Wayland, you can build a fresh non-minified bundle directly to your clipboard for rapid prototyping by using the `copy-alce` command provided by in the development shell. Alternatively,

```sh
nix run .#copy-alce
```

### Embedding files in Lua templates

The `lua-embed-file` package replaces calls such as
`__EMBED_FILE__("scripts/example.cea")` with collision-safe multiline Lua
strings. Embedded paths are relative to an explicit root and cannot escape it.

Run it from the development shell, or directly through the flake:

```sh
lua-embed-file --root table-src -o generated/table.lua table-src/table.lua
nix run .#lua-embed-file -- --root table-src table-src/table.lua
```

Without `-o`, generated Lua is written to stdout. Input templates remain valid
Lua source; `lua-embed-file/lua_embed_file.d.lua` provides the LuaLS declaration for
the marker in this repository. The standalone package installs the same
declaration under `share/lua-embed-file/`.

### Building Cheat Engine tables

The standalone `table-builder` package compiles a Lua file that returns a
declarative memory-record hierarchy into a self-contained script for Cheat
Engine. Running the generated script replaces the current address list; save the
table from Cheat Engine afterward to let CE produce the `.CT` XML.

```lua
return {
    records = {
        {
            kind = "header",
            description = "Player",
            children = {
                {
                    kind = "aa",
                    description = "Enable",
                    script = __EMBED_FILE__("scripts/enable.cea"),
                },
                {
                    kind = "record",
                    description = "Health",
                    address = "player+10",
                    vtype = vtDword,
                },
            },
        },
    },
}
```

Compile inline-only definitions directly, or preprocess file markers first:

```sh
table-builder table-src/table.lua -o generated/build-table.lua

lua-embed-file --root table-src table-src/table.lua |
  table-builder - -o generated/build-table.lua
```

The generated script depends only on Cheat Engine's Lua API. It does not load
ALCE, save the table, or manipulate CE's table-script editor. Builder definitions
support `header`, `aa`, and `record` nodes, ordered `children`, pointer offsets,
dropdowns, and a documented whitelist of additional writable CE properties.
`table-builder/table_builder.d.lua` provides LuaLS types for definitions.

## Internals

### Library structure

`src/main.lua` assembles the public `alce` namespace. Internal modules use bare
requires such as `require("globals")`; the bundler resolves them from `src/` and
emits one self-contained file.

| Module | Responsibility |
| --- | --- |
| `globals` | Shared `alce` namespace, configuration, attachment state, and warning history. |
| `memory` | Allocation contexts and registered-symbol lifecycle management. |
| `cheat_table` | Cheat Engine memory-record creation and table-script helpers. |
| `mono` | High-level Mono classes, methods, object aliases, and class tables. |
| `mono_plumbing` | Low-level Mono discovery and field/method processing. |
| `mono_cache` | Process-generation cache for Mono class handles, ancestry, and image indexes. |
| `mono_t` | Representations of common managed container layouts. |
| `vt` / `t` | Cheat Engine vartype operations and the public `alce.T` lookup table. |
| `monoscript` | Indexed views of Mono constants supplied by Cheat Engine. |
| `fmt` / `printers` | Formatting, inspection, debug output, and warning diagnostics. |
| `utils` / `validators` | General pointer, table, validation, and transformation helpers. |

Most consumers should work through `alce.mono`, `alce.memory`, `alce.cheat_table`,
and `alce.T`. `mono_plumbing` is exposed for low-level work but is intentionally
less stable.

### Mono class model

Mono class representations are cached for the currently opened process. Each
class stores only its declared fields and methods, links to a cached `parent`, and
performs inherited keyed lookup child-first. Shared ancestors are fetched and
processed once instead of being flattened into every requested child class.

Method lookup supports exact signatures and bare names. A bare name resolves only
when the effective inherited overload set is unambiguous. Iteration over member
maps does not currently promise a flattened inherited view.

ALCE invalidates Mono caches on every Cheat Engine `onOpenProcess` event and
preserves any previously installed callback. Class tables retain their configured
targets but unload cached representations. Classes, methods, and object aliases
from an earlier process generation reject operations until the consumer reloads
them.

### Testing

Tests use the lightweight BDD helpers in `tests/test_utils.lua` and follow a
Given–When–Then structure. Every `*.test.lua` file is discovered by the `test`
flake app. `test-package` separately loads both generated bundles to catch
bundling and dependency-resolution failures.

Cheat Engine globals are simulated by `tools/env_mock.lua`. Standalone Lua scripts
that load ALCE source or a packaged bundle must install this mock before requiring
ALCE. The mock is for tests and development only; it is not part of the runtime
library.

### Bundling notes

The release build uses `lunar-bundler` in Lua 5.3 production mode. fnlua is
injected as `package.preload["fn"]`; other runtime facilities are expected from
the Cheat Engine host unless explicitly bundled by ALCE.

Generated `alce.lua` files are build artifacts and should be regenerated from the
modular sources rather than edited directly.
