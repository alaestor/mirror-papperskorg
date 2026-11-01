{ ... }:
{
  perSystem =
    {
      pkgs,
      self',
      ...
    }:
    let
      hcodex = pkgs.writeShellApplication {
        name = "hcodex";
        runtimeInputs = [ pkgs.coreutils ];
        text = ''
          export CODEX_HOME="$HOME/.codex-headroom"
          mkdir -p "$CODEX_HOME"
          ${self'.packages.headroom}/bin/headroom wrap codex --rtk "$@"
        '';
      };
    in
    {
      apps.hcodex = {
        type = "app";
        program = "${hcodex}/bin/hcodex";
        meta.description = "Run Headroom-wrapped Codex with a persistent isolated CODEX_HOME.";
      };

      devShells.hcodex = pkgs.mkShell {
        packages = [ hcodex ];
        shellHook = ''
          export CODEX_HOME="$HOME/.codex-headroom"
          mkdir -p "$CODEX_HOME"
        '';
      };
    };
}
