{ ... }:
{
  perSystem =
    { pkgs, self', ... }:
    let
      pname = "token-viewer";
      version = "0.0.7-unstable-2026-07-09";
      src = pkgs.fetchFromGitHub {
        owner = "headroomlabs-ai";
        repo = "tokview";
        rev = "e28578d046e15db7492ab6387564248fde4524d8";
        hash = "sha256-wZpOVqd0RQb1IPSgcAWvK1yD1UKEsvXE6nwZcP6RXow=";
      };
      npmDepsHash = "sha256-PwWM6746P4xOlqfEqtZT7oFsYFaUv9Ryzj9pCXNFG+g=";
      # LiteLLM's proxy imports get_flat_dependant, removed in FastAPI 0.141.
      # Keep one consistent Python package set on the last compatible release.
      python = pkgs.python313.override {
        packageOverrides = final: prev: {
          fastapi = prev.fastapi.overridePythonAttrs (old: {
            version = "0.140.6";
            src = pkgs.fetchFromGitHub {
              owner = "fastapi";
              repo = "fastapi";
              tag = "0.140.6";
              hash = "sha256-ZWLXNHmM2uGE5y7hLYW4zNSeCGo9d9X2KNT70aXK6W0=";
            };
            # Nixpkgs' check inputs target 0.141; Tokview exercises this version.
            doCheck = false;
          });
        };
      };
      pythonPackages = python.pkgs;
    in
    {
      packages.tokview = pythonPackages.buildPythonPackage {
        inherit
          pname
          version
          src
          npmDepsHash
          ;
        pyproject = true;
        build-system = [ pythonPackages.hatchling ];
        nativeBuildInputs = [
          pkgs.nodejs
          pkgs.npmHooks.npmConfigHook
        ];
        npmRoot = "web";
        npmDeps = pkgs.fetchNpmDeps {
          inherit src;
          sourceRoot = "source/web";
          hash = npmDepsHash;
        };
        postConfigure = ''
          npm --prefix web run build
          test -f web/build/index.html
        '';
        dependencies =
          (with pythonPackages; [
            aiosqlite
            click
            fastapi
            httpx
            litellm
            pydantic
            pydantic-settings
            pyyaml
            rich
            structlog
            textual
            uvicorn
          ])
          ++ pythonPackages.litellm.optional-dependencies.proxy;
        # Upstream does not use pydantic-settings, and its structlog API works
        # with Nixpkgs' newer release. Keep the upstream tests as the check.
        pythonRelaxDeps = [
          "pydantic-settings"
          "structlog"
        ];
        nativeCheckInputs = with pythonPackages; [
          pytestCheckHook
          pytest-asyncio
          pytest-httpx
        ];
        pythonImportsCheck = [
          "tokview.cli"
          "tokview.server"
          "tokview.tui"
          "litellm.proxy.proxy_server"
        ];
        env.LITELLM_LOCAL_MODEL_COST_MAP = "True";
        passthru.updateScript = pkgs.nix-update-script {
          extraArgs = [
            "--flake"
            "--version=branch"
          ];
        };
        passthru.tests.proxy =
          pkgs.runCommand "tokview-proxy-smoke-test"
            {
              nativeBuildInputs = [ (python.withPackages (_: [ self'.packages.tokview ])) ];
              SSL_CERT_FILE = "${pkgs.cacert}/etc/ssl/certs/ca-bundle.crt";
            }
            ''
              python - <<'PY'
              import asyncio
              import http.client
              import json
              import multiprocessing
              import socket
              import tempfile
              import time
              from pathlib import Path
              from tokview.config import TokviewConfig
              import tokview.server as server

              root = Path(tempfile.mkdtemp())
              server.DEFAULT_DIR = root
              config = TokviewConfig.model_validate({
                  'proxy': {'port': 48020},
                  'dashboard': {'port': 48021},
                  'storage': {'path': str(root / 'db.sqlite')},
              })
              child = multiprocessing.Process(target=lambda: asyncio.run(server.serve(config)))
              child.start()
              try:
                  ready = False
                  for _ in range(200):
                      assert child.is_alive(), 'proxy exited before becoming ready'
                      try:
                          connection = http.client.HTTPConnection('127.0.0.1', 48021, timeout=1)
                          connection.request('GET', '/api/health')
                          response = connection.getresponse()
                          assert response.status == 200
                          assert json.loads(response.read())['status'] == 'ok'
                          with socket.create_connection(('127.0.0.1', 48020), timeout=1):
                              pass
                          ready = True
                          break
                      except (OSError, http.client.HTTPException):
                          time.sleep(0.1)
                  assert ready, 'proxy and dashboard failed to become ready'
                  assert (root / 'db.sqlite').is_file()
                  print('PASS: proxy, dashboard, and SQLite start without provider credentials')
              finally:
                  child.terminate()
                  child.join(timeout=10)
                  if child.is_alive():
                      child.kill()
                      child.join()
              PY
              touch "$out"
            '';
        meta = with pkgs.lib; {
          description = "Local token-usage observability proxy and dashboard for coding agents";
          homepage = "https://github.com/headroomlabs-ai/tokview";
          license = licenses.mit;
          mainProgram = "tokview";
          platforms = platforms.unix;
        };
      };
    };
}
