with import <nixpkgs> {}; {
    nothingEnv = stdenv.mkDerivation {
        name = "nothing-env";
        buildInputs = [ stdenv
                        gcc
                        gdb
                        SDL2
                        pkgconfig
                        cmake
                        valgrind
                      ];
    };
}
