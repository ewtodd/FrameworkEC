{
  description = "ChromiumOS EC development environment";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let pkgs = nixpkgs.legacyPackages.${system};
      in {
        devShells.default = pkgs.mkShell {
          buildInputs = with pkgs; [
            # Native compiler for host utilities
            gcc
            gnumake
            binutils
            ncurses
            # Cross compiler for ARM EC
            gcc-arm-embedded

            # Libraries and tools
            libftdi1
            pkg-config
            python3
            coreutils
            findutils
            gawk
            util-linux
            git
            which
            file

          ];

          shellHook = ''
            # Clear problematic environment variables
            unset TMPDIR TMP TEMP out CC CXX

            # Set up proper directories
            export TMPDIR=/tmp
            export TMP=/tmp
            export TEMP=/tmp

            # Set up compilers explicitly
            export CROSS_COMPILE=arm-none-eabi-
            export BUILDCC=gcc  # Native GCC for host utilities
            export CC=gcc       # Native GCC as default
            export CXX=g++      # Native G++ as default

            # Library paths
            export PKG_CONFIG_PATH="${pkgs.libftdi1}/lib/pkgconfig:$PKG_CONFIG_PATH"

            # Disable problematic warnings
            export CFLAGS="-Wno-error=array-bounds -Wno-array-bounds"
            export CPPFLAGS="-Wno-error=array-bounds -Wno-array-bounds"

            echo "ChromiumOS EC development environment loaded"
            echo "Native compiler: $(which gcc)"
            echo "Cross-compiler: $(which arm-none-eabi-gcc)"
            echo "Build compiler: $BUILDCC"
          '';
        };
      });
}
