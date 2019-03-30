with import <nixpkgs> {}; {
    nothingEnv = stdenv.mkDerivation {
        name = "nothing-env";
        buildInputs = [ stdenv
                        gcc
                        SDL2
                        SDL2_mixer
                        pkgconfig
                        cmake
                        valgrind
                        racket
                        inotifyTools
                        libxml2
                      ];
    };
}
