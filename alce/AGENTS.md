# ALCE Agent Guidelines

## Authoritative Cheat Engine reference

Current Cheat Engine development is closed-source, and its public repository is
outdated. The CE 7.7 API has been documented in `tools/` annotations and mocking.

## Source and bundling conventions

- Internal modules are resolved from `src/`; use bare names such as
  `require("globals")`.
- Release bundles preload fnlua. Other runtime dependencies must be bundled
  explicitly or supplied by Cheat Engine.
- Do not edit generated `alce.lua` bundles; change the modular sources instead.
- Standalone Lua runs must load `tools/env_mock.lua` before ALCE source, for
  example `require("tools.env_mock")` before `require("main")`.

## Public API conventions

- Import fnlua with `local fnlua = require("fn")` and alias
  `local fn = fnlua.wrap`.
- Use `fn({ params = { ... }, body = function(args) ... end })` for public
  boundaries that need named inputs, defaults, validation, or transformation.
- Use `fn.method({ ... })` when the receiver is part of the public boundary.
- Build validation and normalization with immutable `Param` pipelines. Keep
  local helpers and simple predicates as ordinary Lua functions.
- Document wrapped calls with LuaLS annotations such as
  `---@overload fun(args: { address: number }): any`.

## Testing

Tests use the lightweight BDD helpers in `tests/test_utils.lua`. Every
`*.test.lua` file must wrap its suites in `test.run_and_report(function() ... end)`.

- Group related behavior with `test.describe`.
- Use `test.it` for successful behavior and `test.it_throws` when failure is part
  of the interface contract.
- Use `test.expect` assertions and follow a clear Given-When-Then flow.
- Test public guarantees rather than implementation details.
- Keep unit tests narrow; use integration tests for complete workflows.
- Include meaningful boundary and failure cases without duplicating coverage.

Import test and source modules by their bare names:

```lua
local test = require("test_utils")
require("tools.env_mock")
local module = require("module")
```

## Verification

Run both suites before completing a change:

```sh
nix run path:.#test
nix run path:.#test-package
```

Using `path:.` ensures newly created, untracked files are included.
