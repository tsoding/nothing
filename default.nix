with import <nixpkgs> {}; {
    nothingEnv = stdenv.mkDerivation {
        name = "nothing-env";
        buildInputs = [ stdenv
                        gcc
                        SDL2
                        mesa
                        pkgconfig
                        cmake
                        xqilla
                        valgrind
                      ];
        LD_LIBRARY_PATH="${mesa}/lib";
    };
}
