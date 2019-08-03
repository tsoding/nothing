[![Tsoding](https://img.shields.io/badge/twitch.tv-tsoding-purple?logo=twitch&style=for-the-badge)](https://www.twitch.tv/tsoding)
[![Build Status](https://travis-ci.org/tsoding/nothing.svg?branch=master)](https://travis-ci.org/tsoding/nothing)
[![Build status](https://ci.appveyor.com/api/projects/status/gxfgojq4ko98e0g0/branch/master?svg=true)](https://ci.appveyor.com/project/rexim/nothing/branch/master)

# Nothing

![](https://i.imgur.com/7mECYKU.gif)
![](https://i.imgur.com/ABcJqB5.gif)

## Dependencies

- [gcc]
- [cmake]
- [libsdl2-dev]
- [inotify-tools]

### Ubuntu

```console
$ sudo apt-get install gcc cmake libsdl2-dev inotify-tools libxml2-dev
```

### MacOS

```console
$ brew install gcc cmake sdl2
```

### NixOS

For [NixOS] we have a development environment defined in [default.nix]
with all of the required dependencies. You can enter the environment
with `nix-shell` command:

```console
$ nix-shell
```

### Arch Linux

```console
$ sudo pacman -S gcc cmake sdl2 inotify-tools libxml2
```

### Windows

- [Visual Studio 2015+](https://visualstudio.microsoft.com/)
- [SDL2 VC Development Libraries](https://www.libsdl.org/release/SDL2-devel-2.0.9-VC.zip)

## Quick Start

### Linux

```console
$ mkdir build
$ cd build/
$ cmake ..
$ make
$ cd ../data/
$ ../build/nothing
```

### Windows

- Enter the Visual Studio Command Line Development Environment https://docs.microsoft.com/en-us/cpp/build/building-on-the-command-line
  - Basically just find `vcvarsall.bat` and run `vcvarsall.bat x64` inside of cmd
- Download [SDL2 VC Development Libraries](https://www.libsdl.org/release/SDL2-devel-2.0.9-VC.zip) and copy it to `path\to\nothing`

```console
> cd path\to\nothing
> 7z x SDL2-devel-2.0.9-VC.zip
> move SDL2-2.0.9 SDL2
> mkdir build
> cd build
> cmake ..
> cmake --build .
> cd ../data/
> ..\build\nothing
```

## Controls

### Game

#### Keyboard

| Key     | Action                                                      |
|---------|-------------------------------------------------------------|
| `d`     | Move to the right                                           |
| `a`     | Move to the left                                            |
| `SPACE` | Jump                                                        |
| `c`     | Open debug console                                          |
| `r`     | Reload the current level including the Player's position    |
| `q`     | Reload the current level preserving the Player's position   |
| `p`     | Toggle game pause                                           |
| `l`     | Toggle transparency on objects. Useful for debugging levels |
| `TAB`   | Switch to Level Editor                                      |

#### Gamepad

| Button       | Action                 |
|--------------|------------------------|
| `Left Stick` | Movement of the Player |
| `1`          | Jump                   |

### Consolé

| Key       | Action                   |
|-----------|--------------------------|
| `ESC`     | Exit console             |
| `Enter`   | Evaluate the expression  |
| `Up/Down` | Traverse console history |

<!-- TODO(#914): Level Editor is not documented -->

## Support

You can support my work via

- Twitch channel: https://www.twitch.tv/subs/tsoding
- Patreon: https://www.patreon.com/tsoding

[visual-studio]: https://www.visualstudio.com/
[svg2rects.py]: ./devtools/svg2rects.py
[./levels/]: ./levels/
[./levels/Makefile]: ./levels/Makefile
[gcc]: https://gcc.gnu.org/
[cmake]: https://cmake.org/
[libsdl2-dev]: https://www.libsdl.org/
[NixOS]: https://nixos.org/
[default.nix]: ./default.nix
[inotify-tools]: https://github.com/rvoicilas/inotify-tools
