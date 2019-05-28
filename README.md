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
$ ./nothing ../levels/
$ ./nothing_test
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
> cmake ..
> cmake --build .
> nothing ..\levels
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

## Editing Levels

Generally creating a level looks like:

```
SVG File -> Custom Level File -> Game
```

To convert SVG to the level file and run svg2level program:

```console
$ ./build/svg2level compile <svg-file> <level-file>
```

All of the levels reside in the [./levels/] folder. Use
[./levels/Makefile] to automatically rebuild all levels:

```console
$ cd levels/     # you must be inside of the `levels/` folder
$ make
```

### Level Editing Workflow

1. `$ inkscape ./levels/level.svg &`
2. `$ ./build/nothing ./levels/level.txt &`
3. `$ cd ./levels/`
4. `$ make watch`
5. Edit Level in Inkscape and Save
6. Switch to the Game and reload level by pressing Q
7. Go to 5

### Objects Reference

#### SVG rect node

| Regex of id  | Description                                                                                                       |
|--------------|-------------------------------------------------------------------------------------------------------------------|
| `player`     | Defines the **position** of the Player. **Size is ignored**.                                                      |
| `rect.*`     | Defines the **size** and **position** of an impenetrable platform block                                           |
| `box.*`      | Defines the **size** and **position** of a rigid box that obeys the physics of the game                           |
| `region(.*)` | Defines the **size** and **position** of a region that hides the Goals. `\1` defines the id of the Goal to hide.  |
| `goal(.*)`   | Defines the **position** of the goal. **Size is ignored**. `\1` defines the id of the region that hides the goal. |
| `lava.*`     | Defines the **position** and **size** of a lava block.                                                            |
| `background` | Defines the **color** of the background. **Position and size are ignored**.                                       |
| `backrect.*` | Defines the **size** and **position** of a solid block in the background.                                         |

#### SVG text node

| Regex of id | Description                                                                |
|-------------|----------------------------------------------------------------------------|
| `label.*`   | Defines **position** and **text** of a in-game label. **Size is ignored**. |

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
