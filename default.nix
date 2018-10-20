with import <nixpkgs> {}; {
    nothingEnv = stdenv.mkDerivation {
        name = "nothing-env";
        buildInputs = [ stdenv
                        gcc
                        SDL2
                        SDL2_mixer
                        mesa
                        pkgconfig
                        cmake
                        valgrind
                        racket
                        inotifyTools
                        python3
                      ];
        LD_LIBRARY_PATH="${mesa}/lib";
    };
}
