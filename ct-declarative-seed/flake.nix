{
  description = "Alaestor's Cheat Engine tables";

  inputs = {
    alce.url = "git+https://codeberg.org/alaestor/alce.git";
    nixpkgs.follows = "alce/nixpkgs";
  };

  outputs =
    {
      self,
      alce,
      nixpkgs,
    }:
    let
      systems = [
        "x86_64-linux"
        "aarch64-linux"
        "x86_64-darwin"
        "aarch64-darwin"
      ];
      forAllSystems = nixpkgs.lib.genAttrs systems;
    in
    {
      packages = forAllSystems (
        system:
        let
          pkgs = import nixpkgs { inherit system; };
          table = pkgs.stdenvNoCC.mkDerivation {
            pname = "black-grimoire-curse-breaker-table";
            version = "0.1.0";
            src = ./black-grimoire-curse-breaker;

            nativeBuildInputs = [
              pkgs.lua5_3
              alce.packages.${system}.lua-embed-file
              alce.packages.${system}.table-builder
            ];

            dontConfigure = true;

            buildPhase = ''
              runHook preBuild
              ls
              lua-embed-file --root src table.lua | table-builder - -o table.lua
              runHook postBuild
            '';

            doCheck = true;
            checkPhase = ''
              runHook preCheck
              lua test-generated.lua table.lua ${alce}/tools/env_mock.lua
              runHook postCheck
            '';

            installPhase = ''
              runHook preInstall
              mkdir -p "$out"
              install -m 0644 table.lua "$out/table.lua"
              runHook postInstall
            '';
          };
        in
        {
          default = table;
          black-grimoire-curse-breaker = table;
        }
      );

      checks = forAllSystems (system: {
        black-grimoire-curse-breaker =
          self.packages.${system}.black-grimoire-curse-breaker;
      });
    };
}
