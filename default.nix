with import <nixpkgs> {}; {
    nothingEnv = gcc8Stdenv.mkDerivation {
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
