# table-builder

`table-builder` compiles a declarative Lua definition into a standalone script
that Cheat Engine can execute to construct its address list. The generated
script uses only CE's Lua API; it does not require ALCE, save the table, or
install a table script.

## Definition format

A definition returns one table containing a dense `records` array. Every record
has a `kind`, a string `description` (which may be blank), and an optional dense
`children` array.

- `header` creates a group header.
- `aa` requires `script` containing the complete auto-assembler source.
- `record` accepts `address`, `vtype`, `offsets`, and `dropdown`. Each offset may
  be a number or a Cheat Engine address expression string.

`record.dropdown` accepts either inline `items` text or a `linked_record`
description, plus `read_only`, `description_only`, and `display_as_item`
booleans.

The optional `properties` table uses exact CE property names. The supported
whitelist is:

- Common: `Color`, `Collapsed`, `Options`
- Value records: `ShowAsHex`, `ShowAsSigned`, `AllowIncrease`,
  `AllowDecrease`, `CustomTypeName`
- AA records: `Async`
- Type-specific: `String.Size`, `String.Unicode`, `String.Codepage`,
  `Binary.Startbit`, `Binary.Size`, `Aob.Size`

Type-specific properties require the matching `vtype`. Properties that activate
records, mutate process memory, install callbacks, control hierarchy, or expose
read-only CE state are intentionally rejected.

## Usage

```sh
table-builder definition.lua -o build-table.lua
```

Definitions may contain `__EMBED_FILE__` markers as ordinary Lua expressions.
Resolve them with the separate preprocessor before compiling:

```sh
lua-embed-file --root table-src table-src/definition.lua |
  table-builder - -o build-table.lua
```

Executing `build-table.lua` validates the complete definition, stages the new
records, and removes the previous roots only after construction succeeds. The
resulting address list can then be saved normally from Cheat Engine.
