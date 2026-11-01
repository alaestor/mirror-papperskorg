{ inputs, ... }:
{
  nucleus.inputs = {
    pyproject-nix = {
      url = "github:pyproject-nix/pyproject.nix";
      inputs.nixpkgs.follows = "nixpkgs";
    };
    uv2nix = {
      url = "github:pyproject-nix/uv2nix";
      inputs.pyproject-nix.follows = "pyproject-nix";
      inputs.nixpkgs.follows = "nixpkgs";
    };
    pyproject-build-systems = {
      url = "github:pyproject-nix/build-system-pkgs";
      inputs.pyproject-nix.follows = "pyproject-nix";
      inputs.uv2nix.follows = "uv2nix";
      inputs.nixpkgs.follows = "nixpkgs";
    };
  };

  perSystem =
    {
      pkgs,
      lib,
      self',
      ...
    }:
    let
      version = "1.6.6";
      src = pkgs.fetchFromGitHub {
        owner = "topoteretes";
        repo = "cognee";
        tag = "v${version}";
        hash = "sha256-qc7zIeyK4Eel/54KmwD3YI9FDGR778DuZYzKTz7WxZ8=";
      };
      workspace = inputs.uv2nix.lib.workspace.loadWorkspace { workspaceRoot = src; };
      # Git does not contain the JSON extension. Release archives bundle it.
      release = pkgs.fetchPypi {
        pname = "cognee";
        inherit version;
        hash = "sha256-jQp0n3y7hApnjKOVhwqfpabyVEoTVXWdeAQtgnYOUOc=";
      };
      extensionPlatform = if pkgs.stdenv.hostPlatform.isAarch64 then "linux_arm64" else "linux_amd64";
      pythonSet =
        (pkgs.callPackage inputs.pyproject-nix.build.packages {
          python = pkgs.python313;
        }).overrideScope
          (
            lib.composeManyExtensions [
              inputs.pyproject-build-systems.overlays.default
              (workspace.mkPyprojectOverlay { sourcePreference = "wheel"; })
              (final: prev: {
                cognee = prev.cognee.overrideAttrs (old: {
                  nativeBuildInputs = (old.nativeBuildInputs or [ ]) ++ [ pkgs.autoPatchelfHook ];
                  buildInputs = (old.buildInputs or [ ]) ++ [ pkgs.stdenv.cc.cc.lib ];
                  postPatch = (old.postPatch or "") + ''
                    tar xf ${release} --wildcards --strip-components=1 \
                      'cognee-${version}/cognee_db_workers/ladybug_extensions/*/${extensionPlatform}/libjson.lbug_extension'
                    # The upstream defaults live beside the module, inside the read-only store.
                    substituteInPlace cognee/base_config.py \
                      --replace-fail 'get_absolute_path(".data_storage")' 'str(Path.home() / ".cognee" / "data")' \
                      --replace-fail 'get_absolute_path(".cognee_system")' 'str(Path.home() / ".cognee" / "system")' \
                      --replace-fail 'get_absolute_path(".cognee_cache")' 'str(Path.home() / ".cognee" / "cache")'
                  '';
                });
                ladybug = prev.ladybug.overrideAttrs (old: {
                  buildInputs = (old.buildInputs or [ ]) ++ [ pkgs.openssl ];
                });
                langdetect = prev.langdetect.overrideAttrs (old: {
                  nativeBuildInputs =
                    (old.nativeBuildInputs or [ ]) ++ final.resolveBuildSystem { setuptools = [ ]; };
                });
              })
            ]
          );
    in
    {
      packages.cognee = (pythonSet.mkVirtualEnv "cognee-${version}" { cognee = [ ]; }).overrideAttrs {
        inherit src version;
        passthru = {
          inherit release;
          updateCustomDeps = [ "release" ];
        };
        meta = {
          description = "Build knowledge graphs and semantic memory for AI applications";
          homepage = "https://github.com/topoteretes/cognee";
          license = lib.licenses.asl20;
          mainProgram = "cognee-cli";
          platforms = [
            "x86_64-linux"
            "aarch64-linux"
          ];
        };
      };

      checks.cognee =
        pkgs.runCommand "cognee-smoke"
          {
            nativeBuildInputs = [ self'.packages.cognee ];
          }
          ''
            export COGNEE_LOGS_DIR="$TMPDIR/logs"
            export DATA_ROOT_DIRECTORY="$TMPDIR/data"
            export SYSTEM_ROOT_DIRECTORY="$TMPDIR/system"
            export CACHE_ROOT_DIRECTORY="$TMPDIR/cache"
            export TELEMETRY_DISABLED=1
            export COGNEE_TRACING_ENABLED=false
            cognee-cli --help > help.txt
            grep -q 'cognify' help.txt
            python ${./_smoke.py} ${version}
            touch "$out"
          '';
    };
}
