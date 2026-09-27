{ pkgs
, alce
, ct-builder
, name ? builtins.baseNameOf (toString ./. )
}:
let
  system = pkgs.stdenv.hostPlatform.system;
in
pkgs.runCommand "${name}.ct" {
  nativeBuildInputs = [ ct-builder.packages.${system}.ct-builder ];
} ''
  source_root="$TMPDIR/${name}"
  cp -R ${./.} "$source_root"
  chmod -R u+w "$source_root"

  global_script="$source_root/scripts-legacy/global.lua"
  grep -Fxq -- "--lib alce placeholder" "$global_script"
  cp "$global_script" "$global_script.template"
  {
    cat ${alce.packages.${system}.alce}/alce.lua
    tail -n +2 "$global_script.template"
  } > "$global_script"

  ct-builder \
    --root "$source_root" \
    "$source_root/table.lua" \
    -o "$out"
''
