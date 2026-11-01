{ ... }:
{
  perSystem =
    {
      pkgs,
      self',
      ...
    }:
    let
      hcodex-temp = pkgs.writeShellApplication {
        name = "hcodex-temp";
        runtimeInputs = [ pkgs.coreutils ];
        text = ''
          source_codex_home="''${CODEX_HOME:-$HOME/.codex}"
          session_parent="''${XDG_CACHE_HOME:-$HOME/.cache}/hcodex-temp"
          mkdir -p "$session_parent"
          session_root="$(mktemp -d --tmpdir="$session_parent" session.XXXXXXXXXX)"
          session_codex_home="$session_root/.codex"

          cleanup() {
            rm -rf -- "$session_root"
          }
          trap cleanup EXIT

          mkdir -p "$session_codex_home"
          if [[ -d "$source_codex_home" ]]; then
            cp -a --reflink=auto "$source_codex_home/." "$session_codex_home/"
          fi

          export CODEX_HOME="$session_codex_home"
          ${self'.packages.headroom}/bin/headroom wrap codex "$@"
        '';
      };
    in
    {
      apps.hcodex-temp = {
        type = "app";
        program = "${hcodex-temp}/bin/hcodex-temp";
        meta.description = "Run Headroom-wrapped Codex with an ephemeral copy of CODEX_HOME.";
      };
    };
}
