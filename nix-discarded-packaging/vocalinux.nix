{ ... }:
let
  version = "0.17.0";
in
{
  perSystem = { lib, pkgs, self', ... }:
    let
      mkVocalinux =
        { withWhisperCpp ? true }:
        pkgs.python3Packages.buildPythonApplication {
      pname = "vocalinux";
      inherit version;
      pyproject = true;

      src = pkgs.fetchFromGitHub {
        owner = "jatinkrmalik";
        repo = "vocalinux";
        rev = "v${version}";
        hash = "sha256-5jTfzPw8eogiMiOv/E5A1QJzspdkPU0eWhTlklTSA5g=";
      };

      build-system = with pkgs.python3Packages; [
        setuptools
        wheel
      ];

      nativeBuildInputs = [
        pkgs.wrapGAppsHook3
        pkgs.gobject-introspection
      ];

      buildInputs = [
        pkgs.gtk3
        pkgs.libayatana-appindicator
        pkgs.ibus
        pkgs.portaudio
      ];

          propagatedBuildInputs =
            (with pkgs.python3Packages; [
              evdev
              lxml
              numpy
              pyaudio
              pydub
              pygobject3
              pynput
              psutil
              python-xlib
              requests
              tqdm
            ])
            ++ lib.optional withWhisperCpp self'.packages.pywhispercpp;

      # Vosk is not packaged by nixpkgs.  The default whisper.cpp engine is
      # provided by this flake's pywhispercpp package above.
          pythonRemoveDeps = [ "vosk" ] ++ lib.optional (!withWhisperCpp) "pywhispercpp";

          doCheck = false;

          postInstall = ''
        install -Dm644 vocalinux.desktop "$out/share/applications/vocalinux.desktop"
        install -Dm644 resources/icons/scalable/vocalinux.svg \
          "$out/share/icons/hicolor/scalable/apps/vocalinux.svg"
      '';

          makeWrapperArgs = [
        "--prefix"
        "PATH"
        ":"
        (lib.makeBinPath [
          pkgs.ibus
          pkgs.wl-clipboard
          pkgs.wtype
          pkgs.xclip
          pkgs.xdotool
          pkgs.ydotool
        ])
      ];

          meta = {
        description = "Voice dictation system for Linux";
        homepage = "https://github.com/jatinkrmalik/vocalinux";
        license = lib.licenses.gpl3Only;
        mainProgram = "vocalinux";
        platforms = lib.platforms.linux;
          };
        };
    in
    {
      # Upstream's default engine is whisper.cpp.  Vocanix can opt out when
      # its local remote-API service is the only intended recognition backend.
      packages.vocalinux = lib.makeOverridable mkVocalinux { };
    };
}
