{ inputs, ...} :
let

  np = inputs.unstable-nixpkgs;

  # normal host information

  system = "x86_64-linux";
  hostname = "cryptid";
  username = "user";

  # cryptid-specific

  version                   = "20260609";
  bootable                  = "mkbootable-${hostname}";
  persist-partition-label   = "PERSIST";
  persist-partition-mount   = "/persist";
  vault-name                = "vault";
  vault-fs                  = "FAT";
  vault-size                = "300K";
  vault-hasher              = "sha512";
  vault-crypto              = "AES-Twofish";
  vault-pim                 = "0"; # use defaults
  expiry-in-months          = "13"; # one month buffer for annual extension
  yubi-retries-pin          = "10"; # signing, decryption, authentication
  yubi-retries-puk          = "10"; # the unblock pin, to reset user pin retry counter
  yubi-retries-admin        = "10"; # changing pins, setting retries, etc.
  yubi-stock-user-pin       = "123456"; # from the factory
  yubi-stock-admin-pin      = "12345678"; # from the factory
  path-firsttimeflag        = "/tmp/first-time-init";
  path-expectScriptLog      = "./expectScript.log";
  path-vault-file           = "${persist-partition-mount}/${vault-name}";
  path-vault-mount          = "/run/media/${vault-name}";
  pathv-master-private      = "${path-vault-mount}/gpg.private.asc";
  pathv-master-public       = "${persist-partition-mount}/public/gpg.public.asc";
  pathv-master-revoke       = "${persist-partition-mount}/revoke/gpg_revocation_cert.rev";
  pathv-breakglass-private  = "${path-vault-mount}/id_ed25519_breakglass";
  pathv-breakglass-public   = "${persist-partition-mount}/id_ed25519_breakglass.pub";
  pathv-yubicodes           = "${path-vault-mount}/yubicodes/yubicodes.txt";
  mkdirs = [
    "${persist-partition-mount}/public"
    "${persist-partition-mount}/revoke"
  ];

  # reusable text for scripts

  docstr = ''
    *************
    ** CRYPTID **              version ${version}
    *************

    Bootable Offline NixOS for cryptographic ID management

    Helper scripts

      ? ----------------- print this menu
      list-files -------- list contents of persist and vault
      list-gpg ---------- list gpg keys
      print_keyid ------- print the first private GPG signature
      print_serial ------ print the YubiKey serial

    Lesser scripts: medium-level abstractions

      vault-init -------- initialize the persistent vault
      vault-open -------- open the persistent vault
      vault-cd ---------- change directory to vault mount
      vault-close ------- close the persistent vault
      gpg-clean --------- delete and recreate the gpg folder
      gpg-new-rootkey --- create a new root key
      gpg-new-subkeys --- create new sign/encrypt subkeys
      gpg-backup -------- export keys to vault
      gpg-restore ------- import keys from vault
      breakglass-init --- create and backup emergency key
      breakglass-test --- verify emergency key backup
      yubi-reset -------- reset a yubikey to factory defaults
      yubi-new-ssh ------ create a resident ssh key on the yubikey
      yubi-move-keys ---- move the two subkeys to the yubikey
      yubi-extend ------- extend all subkeys by ${expiry-in-months} months

    Greater scripts: high-level workflow compositions

      first-time-init --- make the vault, gpg keys, backup
      init-yubikey ------ restore backup, provision yubikey
      extend ------------ restore backup, extend subkeys, backup
      rotate ------------ NOT IMPLEMENTED ... yet
    ______________________________________________________

    Persistent is storage mounted at: ${persist-partition-mount}/
    ______________________________________________________
  '';
in
{
  /**
    Bootable installer ISO (~1.5GB) intended for offline management of cryptographic identities; not intended for system installation. Provides various commandline tools for working with pgp, yubikeys, and veracrypt containers.
  */
  flake.nixosConfigurations = inputs.self.lib.mkNixos system hostname np;
  flake.modules.nixos.${hostname} =  {config, pkgs, lib, ...} :
  let

    mkScript = name: content: pkgs.writeShellScriptBin name ''
      printf "\n— ${name} —\n"
      set -eo pipefail
      # workaround: wrapper to allow `return` because `exit` is problematic when used with `source`
      _impl() {
      ${content}
      }
      _impl "$@"
    '';

    bash-ask-pass = var: str: ''
      if [[ -z "''$${var}" ]]; then
        while true; do
          read -rsp "Enter ${str} password: " pass1
          echo
          read -rsp "Confirm ${str} password: " pass2
          echo
          if [[ "$pass1" == "$pass2" ]]; then
            export ${var}="$pass1"
            break
          else
            echo "Passwords do not match. Please try again, or Ctrl+C to exit."
          fi
        done
      fi
    '';

    bash-make-gnupg-home = ''
      #https://github.com/Mic92/dotfiles/blob/ed0ac1af816a7ebb7c5d4f040b77fa88e3ec1c79/nixos/images/yubikey-image.nix
      export GNUPGHOME=/run/user/$(id -u)/gnupghome
      if [ ! -d "$GNUPGHOME" ]; then
        mkdir "$GNUPGHOME"
        chmod 700 "$GNUPGHOME"
      fi
      # workaround: ssh-keygen uses ccid killing device requiring a replug
      sudo echo "disable-ccid" > "$GNUPGHOME/scdaemon.conf"
    '';

    bash-get-keyid = ''
      if [[ -z "$KEYID" ]]; then
        export KEYID=$(gpg --list-secret-keys --keyid-format long --with-colons | awk -F: '/^fpr/{print $10; exit})
        [[ -z "$KEYID" ]] && { echo "ERROR: failed to find KEYID"; return 1; }
      fi
    '';

    bash-get-yubiserial = ''
      mapfile -t serials < <( ykman list --serials | sed '/^[[:space:]]*$/d' )
      if [[ ''${#serials[@]} -eq 0 ]]; then
          echo "Error: No YubiKeys detected." >&2
          return 1
      fi
      if [[ ''${#serials[@]} -ne 1 ]]; then
          echo "Error: Expected exactly one YubiKey to be inserted." >&2
          printf 'Detected serials:\n%s\n' "''${serials[@]}" >&2
          return 1
      fi
      YUBISERIAL=''${serials[0]}
      if [[ ! $YUBISERIAL =~ ^[0-9]+$ ]]; then
          echo "Error: Invalid serial number: $YUBISERIAL" >&2
          return 1
      fi
    '';

    # are these overkill? maybe... but decreases verbosity and makes the scripts more reliable.
    expect-error-cases-backslash = ''
      eof { puts "Error: unexpected EOF"; exit 3} \
      timeout { puts "Error: timeout"; exit 2 }
    '';
    expect-error-cases = builtins.replaceStrings ["\\"] [""] expect-error-cases-backslash;
    expect-default-spawn = ''if {\$spawn eq ""} { set spawn \$::spawn_id }'';
    expect-header = ''
      set timeout 5
      proc safe_expect { pattern {spawn ""} } {
        ${expect-default-spawn}
        # need to avoid {} to allow pattern string interp
        expect -i \$spawn \
          \$pattern {} \
          ${expect-error-cases-backslash}
      }

      proc safe_send { text {spawn ""} } {
        ${expect-default-spawn}
        send -i \$spawn -- "\$text\r"
      }

      proc safe_expect_end {{spawn ""}} {
        ${expect-default-spawn}
        expect -i \$spawn {
          eof {}
          timeout {
            puts "Error: timeout while expecting EOF."
            exit 2
          }
        }
      }

      proc oi { pattern text {spawn ""} } {
        ${expect-default-spawn}
        safe_expect \$pattern \$spawn
        safe_send \$text \$spawn
      }
    '';
    expect-gpgeditkey = ''

      proc revoke_key { keyindex {spawn ""} } {
        ${expect-default-spawn}
        oi "gpg>" "key \$keyindex" \$spawn
        oi "gpg>" "revoke" \$spawn
        oi "gpg>" "key \$keyindex" \$spawn
      }

      # move key number index to the yubikey (assumes no subkey passphrase; only yubi admin pin)
      proc move_key_to_card { keyindex yubiadminpin {spawn ""} } {
        ${expect-default-spawn}
        set selection \$keyindex
        oi "gpg>" "key \$keyindex" \$spawn
        oi "gpg>" "keytocard" \$spawn
        oi "Your selection?" "\$selection" \$spawn
        while {1} {
          expect -i \$spawn {
            # sometimes it wants the pass twice (on the first run)
            "Enter passphrase: " { safe_send \$yubiadminpin \$spawn }
            "gpg>" {
              # deselect
              safe_send "key \$keyindex" \&spawn
              break
            }
            ${expect-error-cases}
          }
        }
        puts "Moved key \$keyindex selection \$selection to smartcard."
      }



      # returns the number of `ssb` subkeys listed by gpg
      proc count_subkeys {{spawn""}} {
        ${expect-default-spawn}
        oi "gpg>" "list" \$spawn
        set subkey_count 0
        expect -i \$spawn {
            -re "^ssb" {
                incr subkey_count
                exp_continue
            }
            -notransfer "gpg>" {}
            ${expect-error-cases}
        }
        if {$subkey_count == 0} {
            puts "Error: No subkeys found"
            exit 1
        }
        if {$subkey_count % 2 != 0} {
            puts "Error: Number of subkeys must be divisible by 2"
            exit 1
        }
        return $subkey_count
      }
    '';

  in {

    # platform
    imports = with inputs.self.modules.nixos; [
      isolive
      airgap
      # not using crypto components from flake.
      # better to micro-manage; purpose-build.
    ];
    networking.hostName = hostname;
    image.fileName = lib.mkForce "${hostname}-${pkgs.stdenv.hostPlatform.system}";
    boot = {
      tmp.cleanOnBoot = true;
      supportedFilesystems = { btrfs = true; };
      kernel.sysctl = {"kernel.unprivileged_bpf_disabled" = 1;};
      loader.grub = {
        enable                   = true;
        device                   = "nodev";
        efiSupport               = true;
        efiInstallAsRemovable    = true;
      };
    };
    swapDevices = [];
    fileSystems = lib.mkOverride 59 (
      config.lib.isoFileSystems // {
      "${persist-partition-mount}" = {
        device = "/dev/disk/by-label/${persist-partition-label}";
        fsType = "btrfs";
        neededForBoot = true;
        options = [ "defaults" "compress=zstd" "noatime" ];
      };}
    );

    # user
    services.getty.autologinUser = lib.mkForce username;
    users.users."${username}" = {
      isNormalUser      = true;
      description       = "admin";
      extraGroups       = [ "wheel" "systemd-journal" ];
      hashedPassword    = "$y$j9T$P3GLie4lneNg.IMiCEfVU/$B.gUTUGT8pHCKFwtpel.XMKnwgLUU.NTFz700AbM0wB";
    };

    # services
    services.pcscd.enable = true;
    programs.gnupg.agent.enable = true;
    programs.gnupg.agent.pinentryPackage = pkgs.pinentry-tty;
    hardware.gpgSmartcards.enable = true;

    environment.etc."issue".text = docstr;
    environment.interactiveShellInit = bash-make-gnupg-home;

    environment.systemPackages = with pkgs; [
      expect
      pwgen-secure
      paperkey
      qrencode
      kpcli
      zbar
      veracrypt
      gnupg
      pcsc-tools
      yubikey-manager
    ]++[
      /**
        Implements various helper scripts. Refer to `modules/hosts/cryptid.nix` and `docs/masterkey-protocol.md` for details. These scripts help automate creation and management of a single cryptographic identity with sensible parameters and disaster response pathways. The scripts won't be useful to you if your needs are more elaborate.
      */

      # utility scripts

      (pkgs.writeShellScriptBin "_debug" ''
        # source to declare a bunch of variables to reduce interactions
        export CRYPTID_DEBUG=1
        export NAME="MrDebug"
        export EMAIL="d@d.com"
        export SSHLABEL="d@sshlabel"
        pass="password123"
        export VAULTPASS=$pass
        export YUBIPINADMIN=$pass
        export YUBIPINUSER=$pass
        export YUBIPINRESET=$pass
        export YUBIFIDOPIN=$pass
        echo "Debug vars set"
      '')

      (pkgs.writeShellScriptBin "print_keyid" ''
        set -eo pipefail
        ${bash-get-keyid}
        echo $KEYID
      '')

      (pkgs.writeShellScriptBin "print_yubiserial" ''
          set -eo pipefail
          ${bash-get-yubiserial}
          echo $YUBISERIAL
      '')

      # ensures various /persist/ folders exist in preparation for writing
      (pkgs.writeShellScriptBin "mkdirs" ''
        set -euo pipefail
        dirs=( ${builtins.concatStringsSep " " mkdirs} )
        for dir in "''${dirs[@]}"; do
          sudo mkdir -p "$dir"
        done
      '')

      # returns the filepath with the filename prefixed with a timestamp
      (pkgs.writeShellScriptBin "timestamped_now" ''
        set -euo pipefail
        path="$1"
        dir=$(dirname "$path")
        base=$(basename "$path")
        ts=$(date +%Y%m%dT%H%M%S)
        echo "''${dir}/''${ts}-''${base}"
      '')

      # returns the filepath with most recent prefixed filename
      (pkgs.writeShellScriptBin "timestamped_last" ''
        set -euo pipefail
        path="$1"
        dir=$(dirname "$path")
        base=$(basename "$path")
        # lexicographically sort time-prefixed files and pick the last one
        last=$(find "$dir" -maxdepth 1 -name "*-''${base}" | sort | tail -n 1)
        if [ -z "$last" ]; then
            # Fallback: if no timestamped versions exist, complain but return the original path
            echo "Error: didn't find timestamp-version files; did the user run scripts out-of-order? Just returning '$1'" >&2
            echo "$path"
        else
            echo "$last"
        fi
      '')



      # Lesser scripts: medium-level abstractions

      (pkgs.writeShellScriptBin "?" ''
        cat << EOF
        ${docstr}
        EOF
      '')

      (pkgs.writeShellScriptBin "list-files" ''
        set -eo pipefail
        printf "\n${persist-partition-mount}/"
        find "${persist-partition-mount}/." -print | sed -e 's;[^/]*/;|____;g;s;____|; |;g'
        if veracrypt --text --list | grep -q "${path-vault-mount}"; then
          printf "\n${path-vault-mount}/\n"
          find "${path-vault-mount}/." -print | sed -e 's;[^/]*/;|____;g;s;____|; |;g'
        else
          echo "Vault not open."
        fi
      '')

      (pkgs.writeShellScriptBin "list-creds" ''
        set -eo pipefail
        YUBI_FILE=$(timestamped_last "${pathv-yubicodes}")
        printf "\nYUBI: passcodes in $YUBI_FILE \n"
        cat $YUBI_FILE
        YUBIFIDOPIN=$(sudo grep "^FIDO2 PIN: " "$YUBI_FILE" | tail -n 1 | awk -F': ' '{print $2}')
        printf "\nYUBI-OpenPGP:\n"
        ykman openpgp info
        printf "\nYUBI-FIDO:\n"
        ykman fido credentials list --pin "$YUBIFIDOPIN"
        printf "\nGPG-card:\n"
        gpg --card-status
        printf "\nGPG-system:\n"
        gpg -K --with-keygrip
      '')

      (mkScript "vault-init" ''
        if [ -f "${path-vault-file}" ]; then
          echo "Error: vault already exists." >&2
          return 1;
        fi
        ${bash-ask-pass "VAULTPASS" "new vault"}
        echo Creating vault ...
        creation_args=(
          --text
          --create "${path-vault-file}"
          --volume-type "normal"
          --filesystem "${vault-fs}"
          --hash "${vault-hasher}"
          --encryption "${vault-crypto}"
          --size "${vault-size}"
          --password "$VAULTPASS"
          --pim "${vault-pim}"
          --keyfiles ""
          --random-source=/dev/urandom
        )
        sudo veracrypt "''${creation_args[@]}"
      '')

      (mkScript "vault-open" ''
        if veracrypt --text --list | grep -q "${path-vault-mount}"; then
          echo "Vault is open."
          return 0
        fi
        [[ -z "$VAULTPASS" ]] && read -rsp "Enter existing vault password: " VAULTPASS
        echo Opening vault ...
        sudo mkdir -p ${path-vault-mount}
        mounting_args=(
          --text
          --mount "${path-vault-file}"
          --password "$VAULTPASS"
          --pim "${vault-pim}"
          --keyfiles ""
          --protect-hidden no
          "${path-vault-mount}"
        )
        sudo veracrypt "''${mounting_args[@]}"
      '')

      (pkgs.writeShellScriptBin "vault-cd" ''cd "${path-vault-mount}"'')

      (mkScript "vault-close" ''
        veracrypt --text --unmount "${path-vault-mount}"
        sync
      '')

      (mkScript "gpg-clean" ''
          gpgconf --kill gpg-agent 2>/dev/null || true
          if [ -d "$GNUPGHOME" ]; then
            rm -rf "$GNUPGHOME"
          fi
          ${bash-make-gnupg-home}
          echo "Refreshed $GNUPGHOME"
      '')

      (mkScript "gpg-new-rootkey" ''
        # Prompt for user details
        [[ -z "$NAME" ]] && read -p "Enter your full name: " NAME
        [[ -z "$EMAIL" ]] && read -p "Enter your email address: " EMAIL
        echo Generating master key ...
        cat <<EOF | gpg -q --batch --pinentry-mode=loopback --passphrase "" --generate-key
        %no-protection
        Key-Type: eddsa
        Key-Curve: ed25519
        Key-Usage: cert
        Passphrase: ""
        Name-Real: "$NAME"
        Name-Email: "$EMAIL"
        Expire-Date: 0
        Preferences: SHA512 AES256 ZLIB BZIP2 ZIP Uncompressed
        %commit
        %echo Master key created with email: $EMAIL
        EOF
      '')

      (mkScript "gpg-new-subkeys" ''
        ${bash-get-keyid}
        echo "Creating sign subkey ..."
        gpg -q --quick-add-key --pinentry-mode=loopback --passphrase "" "$KEYID" ed25519 sign "${expiry-in-months}m"
        echo "Creating encrypt subkey ..."
        gpg -q --quick-add-key --pinentry-mode=loopback --passphrase "" "$KEYID" cv25519 encrypt "${expiry-in-months}m"
      '')

      (mkScript "gpg-backup" ''
        ${bash-get-keyid}
        echo Creating GPG backup ...
        mkdirs
        OUT_PRI=$(timestamped_now "${pathv-master-private}")
        OUT_REV=$(timestamped_now "${pathv-master-revoke}")
        OUT_PUB=$(timestamped_now "${pathv-master-public}")

        TMP_PRI=$(mktemp /tmp/gpg-secret.XXXXXX)
        gpg --yes --batch --export-secret-keys --armor $KEYID > "$TMP_PRI"
        if [[ ! -s "$TMP_PRI" ]]; then
          echo "Error: Secret keys export failed (file is empty or missing)" >&2
          exit 1
        fi
        sudo mv "$TMP_PRI" "$OUT_PRI"

        echo "Saved secret keys to: '$OUT_PRI'"
        TMP_REV=$(mktemp /tmp/gpg-revoke.XXXXXX)
        rm -rf "$TMP_REV"
        expect -d << EXPECT_SCRIPT &> ${path-expectScriptLog}
        ${expect-header}
        spawn gpg --output "$TMP_REV" --gen-revoke "$KEYID"
        oi "Create a revocation certificate" "y"
        safe_expect "0 = No reason specified"
        oi "Your decision?" "0"
        oi "optional description" ""
        oi "Is this okay" "y"
        safe_expect_end
        EXPECT_SCRIPT
        if [[ ! -s "$TMP_REV" ]]; then
          echo "Error: Revocation certificate creation failed (file is empty or missing)" >&2
          exit 1
        fi
        sudo mv "$TMP_REV" "$OUT_REV"
        echo "Saved revoke certificate to: '$OUT_REV'"

        TMP_PUB=$(mktemp /tmp/gpg-pub.XXXXXX)
        gpg --export --armor $KEYID > "$TMP_PUB"
        if [[ ! -s "$TMP_PUB" ]]; then
          echo "Error: public key export failed (file is empty or missing)" >&2
          exit 1
        fi
        sudo mv "$TMP_PUB" "$OUT_PUB"
        echo "Saved public keys to: $OUT_PUB"
      '')

      (mkScript "gpg-restore" ''
        TARGET=$(timestamped_last "${pathv-master-private}")
        echo "Importing most recent key: $TARGET"
        # ensure the agent is running
        gpgconf --launch gpg-agent
        sleep 1
        gpg --batch --import "$TARGET"
        ${bash-get-keyid}
        echo "Imported/Found key: $KEYID"
        echo "$KEYID:6:" | gpg --import-ownertrust
        gpg --list-secret-keys
        gpg --card-status
      '')

      (mkScript "breakglass-init" ''
        echo Generating emergency "break glass" SSH key ...
        mkdirs
        TMP_DIR=$(mktemp -d)
        TMP_FILE="$TMP_DIR/key"
        TMP_PRI="$TMP_FILE"
        TMP_PUB="''${TMP_FILE}.pub"
        OUT_PRI=$(timestamped_now "${pathv-breakglass-private}")
        OUT_PUB=$(timestamped_now "${pathv-breakglass-public}")

        ssh-keygen -t ed25519 -C "breakglass@cryptid-vault" -f "$TMP_FILE" -N ""

        if [[ ! -s "$TMP_PRI" ]]; then
          echo "Error: private key creation failed (file is empty or missing)" >&2
          exit 1
        fi
        if [[ ! -s "$TMP_PUB" ]]; then
          echo "Error: public key creation failed (file is empty or missing)" >&2
          exit 1
        fi
        sudo mv "$TMP_PRI" "$OUT_PRI"
        echo "Saved private key to: $OUT_PRI"
        sudo mv "$TMP_PUB" "$OUT_PUB"
        echo "Saved public key to: $OUT_PUB"
      '')

      (mkScript "breakglass-test" ''
        PRIV_PATH=$(timestamped_last "${pathv-breakglass-private}")
        PUB_PATH=$(timestamped_last "${pathv-breakglass-public}")
        printf "Verifying emergency SSH keys...\nPrivate: $PRIV_PATH\nPublic:  $PUB_PATH\n"
        GEN_PUB=$(ssh-keygen -y -f "$PRIV_PATH")
        REAL_PUB=$(cat "$PUB_PATH")
        if [ "$GEN_PUB" == "$REAL_PUB" ]; then echo "TEST PASSED"; else echo "FAIL: SSH CORRUPT OR INVALID" >&2; fi
      '')

      (mkScript "yubi-reset" ''
        ${bash-get-yubiserial}
        printf "Preparing to factory-reset YubiKey serial $YUBISERIAL ...\n\n!!!\n!!!   WARNING: ALL OPENPGP AND FIDO APPLETS WILL BE RESET ONCE YOU BEGIN!\n!!!\n\n"

        echo "Resetting FIDO ..."
        ykman --device "$YUBISERIAL" fido reset
        echo "Resetting OpenPGP ..."
        ykman --device "$YUBISERIAL" openpgp reset -f
        printf "SKIPPING: OAUTH, PIV, HSMAUTH, OTP  (note: factory OTP is irreplacable)\nCompleted applet reset of: FIDO, OpenPGP.\n"

        printf "\nInitializing Yubikey OpenPGP Access ...\n"
        ykman openpgp access set-retries ${yubi-retries-pin} ${yubi-retries-admin} ${yubi-retries-puk} --admin-pin ${yubi-stock-admin-pin} --force
        YUBI_FILE=$(timestamped_now "${pathv-yubicodes}")

        ${bash-ask-pass "YUBIPINADMIN" "new OpenPGP admin"}
        ykman openpgp access change-admin-pin --admin-pin ${yubi-stock-admin-pin} --new-admin-pin $YUBIPINADMIN
        sudo echo "OpenPGP PIN admin: $YUBIPINADMIN" >> "$YUBI_FILE"

        ${bash-ask-pass "YUBIPINUSER" "new OpenPGP user"}
        ykman openpgp access change-pin --pin ${yubi-stock-user-pin} --new-pin "$YUBIPINUSER"
        sudo echo "OpenPGP PIN user: $YUBIPINUSER" >> "$YUBI_FILE"

        ${bash-ask-pass "YUBIPINRESET" "new OpenPGP reset"}
        ykman openpgp access change-reset-code --admin-pin "$YUBIPINADMIN" --reset-code "$YUBIPINRESET"
        sudo echo "OpenPGP PUK (reset code): $YUBIPINRESET" >> "$YUBI_FILE"

        printf "\nInitializing Yubikey FIDO2 Access ...\n"
        ${bash-ask-pass "YUBIFIDOPIN" "new FIDO"}
        ykman fido access change-pin --new-pin "$YUBIFIDOPIN"
        sudo echo "FIDO2 PIN: $YUBIFIDOPIN" >> "$YUBI_FILE"

        printf "\nDone. All pins have been saved in '$YUBI_FILE'\nIt's recommended that you backup the FIDO2 pin and OpenPGP user / reset pins to a password manager for ease of recovery.\nYou can further customize the card (holder name, language, login data, keyserver,...) via gpg --card-edit\n"
      '')

      (mkScript "yubi-new-ssh" ''
        [[ -z "$EMAIL" ]] && read -p "Enter your email address: " EMAIL
        [[ -z "$SSHLABEL" ]] && read -p "Enter a label (e.g. 'user@yubikey-primary'): " $SSHLABEL
        printf "\nGenerating YubiKey FIDO2 SSH key ...\n\nYou will be prompted for your FIDO pin: $YUBIFIDOPIN\n"
        ssh-keygen -t ed25519-sk -O resident -O application=ssh:$SSHLABEL -C "$EMAIL" -f "/tmp/id_ed25519_sk" -N ""
      '')

      (mkScript "yubi-move-keys" ''
        ${bash-get-keyid}
        [[ -z "$YUBIPINADMIN" ]] && read -p "Enter your YubiKey Admin Password: " YUBIPINADMIN
        echo "Moving the newest two GPG subkeys to YubiKey ..."

        expect -d << EXPECT_SCRIPT &> ${path-expectScriptLog}

        ${expect-header}
        ${expect-gpgeditkey}

        spawn gpg --expert --pinentry-mode loopback --edit-key $KEYID

        set index2 [count_subkeys]
        set index1 [expr {\$index - 1}]
        move_key_to_card \$index1 "$YUBIPINADMIN"
        move_key_to_card \$index2 "$YUBIPINADMIN"

        oi "gpg>" "save"
        safe_expect_end
        EXPECT_SCRIPT
        gpg --card-status
        echo "Done."
      '')

      (mkScript "yubi-move-keys" ''
        ${bash-get-keyid}
        [[ -z "$YUBIPINADMIN" ]] && read -p "Enter your YubiKey Admin Password: " YUBIPINADMIN
        echo "Moving the newest two GPG subkeys to YubiKey ..."

        expect -d << EXPECT_SCRIPT &> ${path-expectScriptLog}

        ${expect-header}
        ${expect-gpgeditkey}

        spawn gpg --expert --pinentry-mode loopback --edit-key $KEYID

        set index2 [count_subkeys]
        set index1 [expr {\$index - 1}]
        move_key_to_card \$index1 "$YUBIPINADMIN"
        move_key_to_card \$index2 "$YUBIPINADMIN"

        oi "gpg>" "save"
        safe_expect_end
        EXPECT_SCRIPT
        gpg --card-status
        echo "Done."
      '')

      (mkScript "yubi-extend" ''
        gpg --card-status
        gpg -K
        ${bash-get-keyid}
        gpg --quick-set-expire $KEYID '${expiry-in-months}m' '*'
        echo "Current subkeys:"
        gpg --list-secret-keys --keyid-format LONG "$KEYID"
      '')

      # Greater scripts: high-level workflow compositions

      (mkScript "first-time-init" ''
        # make vault
        source vault-init
        source vault-open
        # make root key
        source gpg-new-rootkey
        source gpg-new-subkeys
        source gpg-backup
        # make "breakglass" emergency key
        source breakglass-init
        source breakglass-test
        # lockup and prep for shutdown
        source vault-close
        touch ${path-firsttimeflag}
        printf "\n\n---\nDone. Please reboot and run 'init-yubikey' to complete initialization.\n"
      '')

      (mkScript "init-yubikey" ''
        if [ -f ${path-firsttimeflag} ]; then
            printf "Error: '${path-firsttimeflag}' exists; dirty environment.\nIt's strongly recommended that you reboot to validate the recovery paths.\nAlternatively, you can run 'init-yubikey-dirty'.\nAborting.\n" >&2
            return 1
        fi
        # validate recovery paths
        source vault-open
        source gpg-restore
        source breakglass-test
        # provision yubikey and create subkeys
        source yubi-reset
        source yubi-new-ssh
        source yubi-move-keys
        source vault-close
      '')

      (mkScript "init-yubikey-dirty" ''
        echo "Deleting GPG state ..."
        source gpg-clean
        rm -rf "${path-firsttimeflag}"
        source init-yubikey
        touch "${path-firsttimeflag}"
      '')

      (mkScript "extend" ''
        source vault-open
        source gpg-restore
        source yubi-extend
        source vault-close
      '')

      (mkScript "rotate" ''
        echo TODO: FUNCTIONALITY NOT IMPLEMENTED
      '')
    ];
  };

  /**
  Comes with a helper script to destructively provision a bootable USB flashdrive formatted with an additional btrfs partition for manually managed persistance. The rationale for this is a backup strategy which bundles data with the tools needed to use it. Encrypted containers can be made on the persistant partition and the entire drive can be cloned for redundancy.

  > [!CAUTION]
  > ```
  > nix run .#mkbootable-cryptid -- /dev/sdX
  > ```

  Tip: use can use the following command to quickly enumerate removable blockdevices alongside their sizes.
  ```
  nix run nixpkgs#nushell -- -c "lsblk --json | from json | get blockdevices | where rm == true | select name size"
  ```
  */
  flake.apps."${system}"."${bootable}".program =
  let
    iso = inputs.self.nixosConfigurations.${hostname}.config.system.build.isoImage;
    pkgs = np.legacyPackages."${system}";
  in
    toString (pkgs.writeShellScript bootable ''
      set -euo pipefail
      echo "Starting @ $(date +%Y%m%dT%H%M%S)"

      if [ -z "$1" ]; then
        echo "Usage: nix run .#${bootable} -- /dev/sdX"
        echo "WARNING: This is a destructive operation."
        exit 1
      fi

      DEV="$1"

      if [ ! -b "$DEV" ]; then
        echo "Error: $DEV is not a block device"
        exit 1
      fi

      echo "WARNING: This will ERASE ALL DATA on $DEV"
      read -p "Press Y to continue, any other key to abort: " -n 1 -r
      echo
      if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        echo "Aborted."
        exit 1
      fi

      printf "\nWiping $DEV ...\n"
      sudo wipefs -a "$DEV"
      echo ",," | sudo sfdisk --quiet "$DEV"
      sleep 1
      udevadm settle
      sleep 2

      printf "\nWriting ISO to $DEV ...\n"
      cat "${iso}/iso"/*.iso | sudo dd of="$DEV" bs=4M conv=fsync && sync
      sleep 1
      udevadm settle
      sleep 2

      printf "\nAppending partition to $DEV ...\n"
      echo ", ,L" | sudo sfdisk --append --quiet "$DEV"
      sleep 1
      udevadm settle
      sleep 2

      echo ""
      LAST_PART=$(sudo partx -rgo NR "$DEV" | tail -1)
      echo "Formatting ''${DEV}''${LAST_PART} as btrfs..."
      sudo ${pkgs.btrfs-progs}/bin/mkfs.btrfs -L "${persist-partition-label}" -f -m dup -d dup -M -q "''${DEV}''${LAST_PART}"

      echo "Done @ $(date +%Y%m%dT%H%M%S)"
  '');
}
