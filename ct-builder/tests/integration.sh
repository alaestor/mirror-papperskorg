#!/usr/bin/env bash
set -euo pipefail

compiler=$1
test_tmp=$(mktemp -d)
trap 'rm -rf "$test_tmp"' EXIT

"$compiler" tests/fixtures/empty.lua -o "$test_tmp/empty.ct"
xmllint --noout "$test_tmp/empty.ct"
xmllint --c14n tests/expected_empty.ct > "$test_tmp/expected-empty.c14n"
xmllint --c14n "$test_tmp/empty.ct" > "$test_tmp/actual-empty.c14n"
diff -u "$test_tmp/expected-empty.c14n" "$test_tmp/actual-empty.c14n"

"$compiler" -o "$test_tmp/core.ct" tests/fixtures/core.lua
xmllint --c14n tests/expected_core.ct > "$test_tmp/expected-core.c14n"
xmllint --c14n "$test_tmp/core.ct" > "$test_tmp/actual-core.c14n"
diff -u "$test_tmp/expected-core.c14n" "$test_tmp/actual-core.c14n"

"$compiler" --root tests/fixtures tests/fixtures/structured.lua -o "$test_tmp/structured.ct"
xmllint --noout "$test_tmp/structured.ct"
test "$(xmllint --xpath 'count(/CheatTable/CheatEntries/CheatEntry)' "$test_tmp/structured.ct")" = "2"
test "$(xmllint --xpath 'count(//CheatEntry)' "$test_tmp/structured.ct")" = "6"
test "$(xmllint --xpath 'string(//CheatEntry[ID="10"]/AssemblerScript)' "$test_tmp/structured.ct" | grep -c '\[ENABLE\]')" = "1"
test "$(xmllint --xpath 'string(/CheatTable/LuaScript)' "$test_tmp/structured.ct")" = 'print("global")'
test "$(xmllint --xpath 'string(//DropDownList)' "$test_tmp/structured.ct" | head -n 1)" = "0:zero"

"$compiler" - < tests/fixtures/empty.lua > "$test_tmp/stdin.ct"
xmllint --noout "$test_tmp/stdin.ct"

printf 'preserved\n' > "$test_tmp/existing.ct"
if "$compiler" tests/fixtures/invalid.lua -o "$test_tmp/existing.ct"; then
  echo "invalid definition unexpectedly compiled" >&2
  exit 1
fi
grep -Fx "preserved" "$test_tmp/existing.ct"

if "$compiler" tests/fixtures/core.lua -o tests/fixtures/core.lua; then
  echo "same input/output path unexpectedly accepted" >&2
  exit 1
fi

if "$compiler" --root tests/fixtures tests/fixtures/missing-source.lua -o "$test_tmp/missing.ct"; then
  echo "missing source unexpectedly compiled" >&2
  exit 1
fi
test ! -e "$test_tmp/missing.ct"

echo "Integration tests passed"
