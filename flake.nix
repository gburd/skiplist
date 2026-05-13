{
  description = "Header-only C splay-list with lock-free concurrency.";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    let
      # Supported systems.  Linux + Darwin cover developer machines and
      # CI; FreeBSD is included because the implementation is portable
      # C11 with no Linux-specific syscalls.
      systems = [
        "x86_64-linux"
        "i686-linux"
        "aarch64-linux"
        "riscv64-linux"
        "x86_64-darwin"
        "aarch64-darwin"
      ];
    in
    flake-utils.lib.eachSystem systems (system:
      let
        pkgs = import nixpkgs { inherit system; };
        inherit (pkgs) lib stdenv;

        isLinux = stdenv.hostPlatform.isLinux;

        # Tools needed on every supported platform.
        commonTools = with pkgs; [
          # Build systems
          gnumake
          meson
          ninja
          autoconf
          automake
          libtool
          pkg-config

          # Compilers (stdenv already provides one; expose the other so
          # both gcc and clang are available for matrix-style local
          # testing).
          gcc
          clang

          # Documentation / formatting
          clang-tools
          graphviz-nox

          # Coverage
          gcovr
          lcov

          # Misc
          ed
          gettext
          m4
          perl
          python3
          ripgrep
        ];

        # Tools that don't reliably build on non-Linux platforms.
        linuxTools = with pkgs; lib.optionals isLinux [
          gdb
          valgrind
        ];

        # Static glibc is only meaningful on Linux.
        linuxBuildInputs = with pkgs; lib.optionals isLinux [
          libbacktrace
          glibc.out
          glibc.static
        ];
      in
      {
        devShells.default = pkgs.mkShell {
          name = "skiplist";
          packages = commonTools ++ linuxTools;
          buildInputs = linuxBuildInputs;
          shellHook = ''
            echo "skiplist development shell (${system})"
            echo "  cc:    $(${stdenv.cc}/bin/cc --version | head -1)"
            echo "  meson: $(meson --version)"
            echo "  ninja: $(ninja --version 2>/dev/null || echo missing)"
          '';
        };

        # Header-only library: 'nix build' installs the public header
        # plus the pkg-config file via meson.
        packages.default = stdenv.mkDerivation {
          pname = "skiplist";
          version = "1.0.0";
          src = ./.;
          nativeBuildInputs = with pkgs; [ meson ninja pkg-config ];
          meta = with lib; {
            description = "Header-only C splay-list with lock-free concurrency";
            homepage = "https://codeberg.org/gregburd/skiplist";
            license = [ licenses.isc licenses.mit ];
            platforms = systems;
            maintainers = [ ];
          };
        };
      });
}
