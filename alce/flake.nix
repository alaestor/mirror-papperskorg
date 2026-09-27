{
  description = "Alaestor's Cheat Engine Library";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-parts = {
      url = "github:hercules-ci/flake-parts";
      inputs.nixpkgs-lib.follows = "nixpkgs";
    };
    alpkgs = {
      url = "git+https://git.0x04.cc/alaestor/pkgs";
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
  };

  outputs =
    inputs@{
      flake-parts,
      nixpkgs,
      alpkgs,
      cea,
      fnlua,
      ...
    }:
    flake-parts.lib.mkFlake { inherit inputs; } {

      systems = [
        "x86_64-linux"
        "aarch64-linux"
        "aarch64-darwin"
      ];

      perSystem =
        {
          pkgs,
          self',
          system,
          lib,
          ...
        }:
        let
          pkgs_with_overlays = import inputs.nixpkgs {
            inherit system;
            overlays = [ inputs.alpkgs.overlays.default ];
          };

          ap = alpkgs.packages.${system};
          fnluaPkgs = fnlua.packages.${system};
          lua = pkgs.lua5_3;

          luaSearchPath = "src/?.lua;tools/?.lua;tests/?.lua;table-builder/?.lua;lua-embed-file/?.lua;${fnluaPkgs.fnlua}/?.lua;;";

          runUnitTests = ''
            export LUA_PATH="${luaSearchPath}"

            find ./tests/ -name '*.test.lua' -exec sh -c 'lua tests/test_runner.lua "$1"' _ {} \;
          '';

          runPackageTests = ''
            export LUA_PATH="tools/?.lua;tests/?.lua;;"

            echo "=== Testing alce-full ==="
            lua tests/test-package.lua "${self'.packages.alce-full}/alce.lua"

            echo
            echo "=== Testing alce ==="
            lua tests/test-package.lua "${self'.packages.alce}/alce.lua"
          '';

          lua-embed-file-command = pkgs.writeShellApplication {
            name = "lua-embed-file";
            runtimeInputs = [ lua ];
            text = ''
              export LUA_PATH="${./lua-embed-file}/?.lua;;"
              exec lua ${./lua-embed-file/lua_embed_file_cli.lua} "$@"
            '';
          };

          lua-embed-file = pkgs.symlinkJoin {
            name = "lua-embed-file";
            paths = [ lua-embed-file-command ];
            postBuild = ''
              mkdir -p $out/share/lua-embed-file
              cp ${./lua-embed-file/lua_embed_file.d.lua} $out/share/lua-embed-file/
            '';
            meta.mainProgram = "lua-embed-file";
          };

          table-builder-command = pkgs.writeShellApplication {
            name = "table-builder";
            runtimeInputs = [ lua ];
            text = ''
              export LUA_PATH="${./table-builder}/?.lua;;"
              export TABLE_BUILDER_RUNTIME="${./table-builder/table_builder_runtime.lua}"
              exec lua ${./table-builder/table_builder_cli.lua} "$@"
            '';
          };

          table-builder = pkgs.symlinkJoin {
            name = "table-builder";
            paths = [ table-builder-command ];
            postBuild = ''
              mkdir -p $out/share/table-builder
              cp ${./table-builder/table_builder.d.lua} $out/share/table-builder/
              cp ${./table-builder/README.md} $out/share/table-builder/
            '';
            meta.mainProgram = "table-builder";
          };

          runLuaEmbedFileTests = ''
            embed_test_tmp=$(mktemp -d)
            trap 'rm -rf "$embed_test_tmp"' EXIT

            lua-embed-file \
              --root tests/fixtures/lua_embed_file \
              -o "$embed_test_tmp/generated.lua" \
              tests/fixtures/lua_embed_file/template.lua
            lua tests/test-lua-embed-file.lua "$embed_test_tmp/generated.lua"

            lua-embed-file \
              --root tests/fixtures/lua_embed_file \
              tests/fixtures/lua_embed_file/template.lua \
              > "$embed_test_tmp/stdout.lua"
            cmp "$embed_test_tmp/generated.lua" "$embed_test_tmp/stdout.lua"

            printf 'preserved\n' > "$embed_test_tmp/existing.lua"
            if lua-embed-file \
              --root tests/fixtures/lua_embed_file \
              -o "$embed_test_tmp/existing.lua" \
              tests/fixtures/lua_embed_file/missing.lua; then
              echo "lua-embed-file unexpectedly accepted a missing embedded file" >&2
              exit 1
            fi
            grep -Fx 'preserved' "$embed_test_tmp/existing.lua"

            if lua-embed-file \
              --root tests/fixtures/lua_embed_file \
              -o tests/fixtures/lua_embed_file/./template.lua \
              tests/fixtures/lua_embed_file/template.lua; then
              echo "lua-embed-file unexpectedly accepted input as output" >&2
              exit 1
            fi
          '';

          runTableBuilderTests = ''
            export LUA_PATH="tools/?.lua;tests/?.lua;;"

            table_builder_tmp=$(mktemp -d)
            trap 'rm -rf "$table_builder_tmp"' EXIT

            lua-embed-file \
              --root tests/fixtures/table_builder \
              tests/fixtures/table_builder/definition.lua \
              | table-builder -o "$table_builder_tmp/generated.lua"
            lua tests/test-table-builder.lua "$table_builder_tmp/generated.lua"

            table-builder \
              tests/fixtures/table_builder/inline.lua \
              > "$table_builder_tmp/stdout.lua"
            table-builder \
              -o "$table_builder_tmp/file.lua" \
              tests/fixtures/table_builder/inline.lua
            cmp "$table_builder_tmp/stdout.lua" "$table_builder_tmp/file.lua"

            printf 'preserved\n' > "$table_builder_tmp/existing.lua"
            if printf 'return {' \
              | table-builder -o "$table_builder_tmp/existing.lua"; then
              echo "table-builder unexpectedly accepted invalid Lua" >&2
              exit 1
            fi
            grep -Fx 'preserved' "$table_builder_tmp/existing.lua"

            if table-builder \
              -o tests/fixtures/table_builder/./inline.lua \
              tests/fixtures/table_builder/inline.lua; then
              echo "table-builder unexpectedly accepted input as output" >&2
              exit 1
            fi
          '';

          alce-bundler = pkgs.writeShellApplication {
            name = "alce-bundle";
            runtimeInputs = [
              ap.lunar-bundler
              ap.minilua
            ];
            text = ''
              set -euo pipefail

              minify=false
              if [[ "''${1:-}" == "--minify" ]]; then
                minify=true
                shift
              fi
              if (( $# != 0 )); then
                echo "Usage: alce-bundle [--minify]" >&2
                exit 2
              fi
              if [[ ! -f src/main.lua ]]; then
                echo "alce-bundle must be run from the ALCE repository root" >&2
                exit 1
              fi

              bundle_tmp=$(mktemp -d)
              trap 'rm -rf "$bundle_tmp"' EXIT

              {
                printf 'package.preload["fn"] = function()\n'
                cat ${fnluaPkgs.fnlua-full}/fn.lua
                printf '\nend\n'
              } > "$bundle_tmp/fnlua_preload.lua"
              printf '[resolve]\nexternals = ["fn"]\n' > "$bundle_tmp/lunar_bundler.toml"

              lunar-bundler \
                --lua-version 53 \
                --mode=production \
                --config "$bundle_tmp/lunar_bundler.toml" \
                --inject-top "$bundle_tmp/fnlua_preload.lua" \
                -p src src/main.lua -o alce.lua

              if [[ "$minify" == true ]]; then
                minilua --quiet --no-banner --in-place alce.lua
              fi
            '';
          };

          copy-alce = pkgs.writeShellApplication {
            name = "copy-alce";
            runtimeInputs = [
              alce-bundler
              pkgs.coreutils
              pkgs.wl-clipboard
            ];
            text = ''
              set -euo pipefail

              if [[ ! -f src/main.lua ]]; then
                echo "copy-alce must be run from the ALCE repository root" >&2
                exit 1
              fi

              copy_tmp=$(mktemp -d)
              trap 'rm -rf "$copy_tmp"' EXIT
              cp -R src "$copy_tmp/src"
              (
                cd "$copy_tmp"
                alce-bundle
              )
              wl-copy < "$copy_tmp/alce.lua"
              echo "Copied the non-minified ALCE bundle to the clipboard"
            '';
          };
        in
        {
          _module.args.pkgs = pkgs_with_overlays;

          devShells.default = pkgs.mkShell rec {
            packages =
              [
                lua
                alce-bundler
                lua-embed-file
                table-builder
                pkgs.lua-language-server
                fnluaPkgs.fnlua # redundant, for printout
                fnluaPkgs.annotations # redundant, for printout
              ]
              ++ lib.optionals pkgs.stdenv.isLinux [ copy-alce ];

            shellHook = ''
              export CE_ANNOTATIONS_PATH="${cea.packages.${system}.cheat-engine-api}"
              export LUA_PATH="${luaSearchPath}"
              export FNLUA_ANNOTATIONS_PATH="${fnluaPkgs.annotations}"
              echo "Development environment activated with:"
              printf \\n\\t${lib.concatStringsSep "\\\\n\\\\t" (lib.map (p: p.out.name) packages)}\\n
            '';
          };

          apps =
            {
              test = {
                type = "app";
                meta.description = "unit tests: runs all `*.test.lua` files via `test_runner.lua`";
                program = pkgs.lib.getExe (
                  pkgs.writeShellApplication {
                    name = "test";
                    runtimeInputs = [ lua ];

                    text = ''
                      set -euo pipefail

                      ${runUnitTests}
                    '';
                  }
                );
              };

              test-package = {
                type = "app";
                meta.description = "integration tests: validates the alce-full and alce package artifacts";
                program = pkgs.lib.getExe (
                  pkgs.writeShellApplication {
                    name = "test-package";
                    runtimeInputs = [ lua ];

                    text = ''
                      set -euo pipefail

                      ${runPackageTests}

                      echo
                      echo "All package tests passed"
                    '';
                  }
                );
              };

              lua-embed-file = {
                type = "app";
                meta.description = "embeds external text files into a Lua source template";
                program = pkgs.lib.getExe lua-embed-file;
              };

              table-builder = {
                type = "app";
                meta.description = "compiles declarative Cheat Engine table definitions";
                program = pkgs.lib.getExe table-builder;
              };
            }
            // lib.optionalAttrs pkgs.stdenv.isLinux {
              copy-alce = {
                type = "app";
                meta.description = "copies a fresh non-minified ALCE bundle to the Wayland clipboard";
                program = pkgs.lib.getExe copy-alce;
              };
            };

          checks = {
            test = pkgs.stdenvNoCC.mkDerivation {
              name = "test";
              src = ./.;
              buildInputs = [ lua ];
              buildPhase = runUnitTests;
              installPhase = ''
                mkdir -p $out
                touch $out/success
              '';
            };

            test-package = pkgs.stdenvNoCC.mkDerivation {
              name = "test-package";
              src = ./.;
              buildInputs = [ lua ];
              buildPhase = runPackageTests;
              installPhase = ''
                mkdir -p $out
                touch $out/success
              '';
            };

            lua-embed-file = pkgs.stdenvNoCC.mkDerivation {
              name = "lua-embed-file-test";
              src = ./.;
              nativeBuildInputs = [
                lua
                lua-embed-file
              ];
              buildPhase = runLuaEmbedFileTests;
              installPhase = ''
                mkdir -p $out
                touch $out/success
              '';
            };

            table-builder = pkgs.stdenvNoCC.mkDerivation {
              name = "table-builder-test";
              src = ./.;
              nativeBuildInputs = [
                lua
                lua-embed-file
                table-builder
              ];
              buildPhase = runTableBuilderTests;
              installPhase = ''
                mkdir -p $out
                touch $out/success
              '';
            };
          };

          packages.default = self'.packages.alce;

          packages.alce-bundler = alce-bundler;
          packages.lua-embed-file = lua-embed-file;
          packages.table-builder = table-builder;

          packages.alce-full = pkgs.stdenvNoCC.mkDerivation {
            pname = "alce-full";
            version = "0.1.0";
            src = ./.;
            nativeBuildInputs = [ alce-bundler ];
            buildPhase = ''
              alce-bundle
            '';
            installPhase = ''
              mkdir -p $out
              mv alce.lua $out/
            '';
          };

          packages.alce = pkgs.stdenvNoCC.mkDerivation {
            pname = "alce";
            version = "0.1.0";
            src = ./.;
            nativeBuildInputs = [ alce-bundler ];
            buildPhase = ''
              alce-bundle --minify
            '';
            installPhase = ''
              mkdir -p $out
              mv alce.lua $out/
            '';
          };
        };
    };
}
