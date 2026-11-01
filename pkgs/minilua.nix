{ ... }:
{
  perSystem =
    { pkgs, ... }:
    let
      pname = "minilua";
      version = "1.1.0";
    in
    {
      packages.minilua = pkgs.python3.pkgs.buildPythonPackage {
        inherit pname version;

        src = pkgs.fetchgit {
          url = "https://git.0x04.cc/alaestor/fork-MiniLua";
          rev = "v${version}";
          hash = "sha256-vfE2mCe4QTdwowIAoCV6e9u5Xzj1EJEfemllIq6D1Xs=";
        };

        pyproject = true;

        build-system = with pkgs.python3.pkgs; [
          setuptools
          wheel
        ];

        dependencies = with pkgs.python3.pkgs; [
          click
          colorama
        ];

        doInstallCheck = true;

        installCheckPhase = ''
          $out/bin/minilua --version
        '';

        meta = with pkgs.lib; {
          description = "MiniLua";
          #homepage = "https://github.com/aradooo/MiniLua"; -- vanished?
          homepage = "https://git.0x04.cc/alaestor/fork-MiniLua";
          license = licenses.mit;
        };
      };
    };
}
