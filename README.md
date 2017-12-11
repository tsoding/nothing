[![Build Status](https://travis-ci.org/tsoding/nothing.svg?branch=master)](https://travis-ci.org/tsoding/nothing)

# Nothing

![](https://i.imgur.com/06VwEk3.gif)

## Quick Start

```console
$ nix-shell              # Only on NixOS
$ mkdir build
$ cd build/
$ cmake ..
$ make
$ cd ..
$ ./nothing
```

## Build on Windows

You need to install [conan][] and [Visual Studio 2017][visual-studio].

### Dependencies

Current version of [SDL2/2.0.5@lasote/stable][conan-sdl2] in the conan-transit
repository is not compatible with the latest conan, so you'll need to install
it locally from the git repository:

```console
$ git clone https://github.com/lasote/conan-sdl2.git # temporary, I hope hope hope
$ cd conan-sdl2
$ conan export SDL2/2.0.5@lasote/stable
```

### Building

Execute the following commands inside of the `nothing` directory:

```console
$ conan install --build missing --install-folder build
$ cd build
$ cmake .. -G "Visual Studio 15 2017 Win64"
```

After that, build the `build/nothing.sln` file with Visual Studio.

## Support

You can support my work via

- Twitch channel: https://www.twitch.tv/subs/tsoding
- Patreon: https://www.patreon.com/tsoding

[conan]: https://www.conan.io/
[conan-sdl2]: https://bintray.com/conan/conan-transit/SDL2%3Alasote/2.0.5%3Astable
[visual-studio]: https://www.visualstudio.com/
