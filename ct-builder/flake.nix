{
  description = "Compile declarative Lua definitions into Cheat Engine 7.7 tables";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    fnlua.url = "git+https://git.0x04.cc/alaestor/fnlua";
    fnlua.inputs.nixpkgs.follows = "nixpkgs";
  };

  outputs = { self, nixpkgs, fnlua }:
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
      packages = forAllSystems (system:
        let
          pkgs = import nixpkgs { inherit system; };
          fnluaPackage = fnlua.packages.${system}.fnlua;
          luaPath = "${./src}/?.lua;${fnluaPackage}/?.lua;;";
          ct-builder = pkgs.writeShellApplication {
            name = "ct-builder";
            runtimeInputs = [ pkgs.lua5_3 ];
            text = ''
              export LUA_PATH=${nixpkgs.lib.escapeShellArg luaPath}
              exec lua ${./src/ct/cli_main.lua} "$@"
            '';
          };
        in
        {
          default = ct-builder;
          inherit ct-builder;
        });

      apps = forAllSystems (system: {
        default = {
          type = "app";
          program = nixpkgs.lib.getExe self.packages.${system}.default;
          meta.description = "Compile Lua definitions into Cheat Engine 7.7 tables";
        };
      });

      devShells = forAllSystems (system:
        let
          pkgs = import nixpkgs { inherit system; };
          fnluaPackage = fnlua.packages.${system}.fnlua;
          annotations = fnlua.packages.${system}.annotations;
        in
        {
          default = pkgs.mkShell {
            packages = [
              pkgs.lua5_3
              pkgs.lua-language-server
              fnluaPackage
            ];
            shellHook = ''
              export LUA_PATH="$PWD/src/?.lua;$PWD/tests/?.lua;${fnluaPackage}/?.lua;;"
              export FNLUA_ANNOTATIONS_PATH="${annotations}"
            '';
          };
        });

      checks = forAllSystems (system:
        let
          pkgs = import nixpkgs { inherit system; };
          fnluaPackage = fnlua.packages.${system}.fnlua;
          luaPath = "${./src}/?.lua;${./tests}/?.lua;${fnluaPackage}/?.lua;;";
        in
        {
          tests = pkgs.runCommand "ct-builder-tests"
            {
              nativeBuildInputs = [
                pkgs.lua5_3
                pkgs.libxml2
                pkgs.diffutils
                self.packages.${system}.ct-builder
              ];
            }
            ''
              export LUA_PATH=${nixpkgs.lib.escapeShellArg luaPath}
              cd ${./.}
              lua tests/test.lua
              bash tests/integration.sh ${nixpkgs.lib.escapeShellArg (nixpkgs.lib.getExe self.packages.${system}.ct-builder)}
              touch "$out"
            '';
        });
    };
}
