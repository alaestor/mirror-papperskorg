{ ... }:
{
  perSystem =
    { pkgs, ... }:
    let
      pname = "luapack";
      version = "0.1.1";
      src = pkgs.fetchgit {
        url = "https://git.0x04.cc/alaestor/fork-luapack";
        rev = "v${version}";
        hash = "sha256-i3McLeW66JpLrQbSLgHo2Xt68G0U32vb9uTsHyXnaO0=";
      };
    in
    {
      packages.luapack = pkgs.rustPlatform.buildRustPackage {
        inherit pname version src;
        cargoLock.lockFile = "${src}/Cargo.lock";
        doCheck = false;

        meta = with pkgs.lib; {
          description = "A basic rust application for efficiently bundling Lua scripts into monolithic releases.";
          homepage = "https://git.0x04.cc/alaestor/fork-luapack";
          license = licenses.mpl20;
          mainProgram = "luapack";
          platforms = platforms.unix;
        };
      };
    };
}
