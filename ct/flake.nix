{
  description = "Alaestor Weissman's Cheat Engine Tables";

  # Note: each subdirectory is the source for a different cheat table, packaged
  # by the folder's default.nix. As such, this flake has no default package.

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-parts = {
      url = "github:hercules-ci/flake-parts";
      inputs.nixpkgs-lib.follows = "nixpkgs";
    };
    alpkgs = {
      url = "git+https://git.0x04.cc/alaestor/pkgs.git";
      inputs = {
        nixpkgs.follows = "nixpkgs";
        flake-parts.follows = "flake-parts";
      };
    };
    fnlua = {
      url = "git+https://git.0x04.cc/alaestor/fnlua.git";
      inputs = {
        flake-parts.follows = "flake-parts";
        nixpkgs.follows = "nixpkgs";
        alpkgs.follows = "alpkgs";
      };
    };
    cea = {
      url = "git+https://git.0x04.cc/alaestor/zed-cea.git";
      inputs.nixpkgs.follows = "nixpkgs";
    };
    alce = {
      url = "git+https://git.0x04.cc/alaestor/alce.git";
      inputs = {
        nixpkgs.follows = "nixpkgs";
        fnlua.follows = "fnlua";
        alpkgs.follows = "alpkgs";
        flake-parts.follows = "flake-parts";
        cea.follows = "cea";
      };
    };
    ct-builder = {
      url = "git+https://git.0x04.cc/alaestor/ct-builder.git";
      inputs = {
        nixpkgs.follows = "nixpkgs";
        fnlua.follows = "fnlua";
      };
    };
  };

  outputs = inputs@{ flake-parts, nixpkgs, ct-builder, alce, cea, fnlua, ... }:
    let
      systems = [ "x86_64-linux" "aarch64-linux" "aarch64-darwin" ];
      packageDirectories = builtins.filter
        (name: builtins.pathExists (./. + "/${name}/default.nix"))
        (builtins.attrNames (nixpkgs.lib.filterAttrs (_: type: type == "directory") (builtins.readDir ./.)));
    in flake-parts.lib.mkFlake { inherit inputs; } {
      inherit systems;

      perSystem = { pkgs, system, ... }: {
        packages = builtins.listToAttrs (map (name: {
          inherit name;
          value = import (./. + "/${name}") {
            inherit pkgs ct-builder alce name;
          };
        }) packageDirectories);

        devShells = {
          default = pkgs.mkShell {
            packages = [
              ct-builder.packages.${system}.ct-builder
              fnlua.packages.${system}.fnlua
              pkgs.lua5_3
              pkgs.lua-language-server
              pkgs.libxml2
              pkgs.wl-clipboard
            ];
            shellHook = ''
              export ALCE_SOURCE_PATH="${alce}/src"
              export CE_ANNOTATIONS_PATH="${cea.packages.${system}.cheat-engine-api}"
              export FNLUA_ANNOTATIONS_PATH="${fnlua.packages.${system}.annotations}"
              export LUA_PATH="$ALCE_SOURCE_PATH/?.lua;$CE_ANNOTATIONS_PATH/?.lua;$FNLUA_ANNOTATIONS_PATH/?.lua;;"

              # Build a package from this flake and copy its resulting XML to the Wayland clipboard.
              # Usage: ct-copy <package>
              # The last-built package name is cached so a bare `ct-copy` repeats it.
              ct-copy() {
                local pkg="''${1:-$CT_LAST_PKG}"
                if [[ -z "$pkg" ]]; then
                  echo "ct-copy: no package specified and none cached" >&2
                  echo "usage: ct-copy <package>" >&2
                  return 1
                fi
                local out
                out="$(nix build ".#$pkg" --no-link --print-out-paths)" || return 1
                CT_LAST_PKG="$pkg"
                wl-copy < "$out"
                echo "Copied $pkg output to clipboard"
              }
            '';
          };
        };
      };
    };
}
