{ inputs, ... }:

{
  nucleus.inputs.serena = {
    url = "github:oraios/serena/v1.6.1";
    inputs.nixpkgs.follows = "nixpkgs";
  };

  perSystem =
    { pkgs, system, ... }:
    let
      upstreamPackages = inputs.serena.packages.${system};
    in
    {
      packages.serena = pkgs.symlinkJoin {
        name = "serena-1.6.1";
        paths = [ upstreamPackages.serena ];
        nativeBuildInputs = [ pkgs.makeWrapper ];
        # Serena supports per-language `ls_path` configuration.
        #
        # Downstream consumers can implement these overrides themselves.
        # Serena already discovers `nixd` from `PATH`, so this package
        # only adds `nixd`to the executable wrapper.
        #
        # note: serena currently overrides `SHELLCHECK_PATH`, so
        #   bash-lsp should be wrapped to correct it, as demonstrated in
        #   downstream alaestor/flake.git's `cxs` codex launch script
        #
        # validation steps:
        #   1. Build `.#serena` to ensure the upstream uv2nix compiles.
        #   2. Verify `nixd` remains discoverable from `PATH`.
        #   3. Run `nix flake check`.
        postBuild = ''
          wrapProgram $out/bin/serena \
            --prefix PATH : ${pkgs.lib.makeBinPath [ pkgs.nixd ]}
        '';

        meta = upstreamPackages.serena.meta // {
          mainProgram = "serena";
        };
      };
    };
}
