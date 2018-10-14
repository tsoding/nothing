[![Build Status](https://travis-ci.org/tsoding/nothing.svg?branch=master)](https://travis-ci.org/tsoding/nothing)

# Nothing

![](https://i.imgur.com/7mECYKU.gif)
![](https://i.imgur.com/ABcJqB5.gif)

## Quick Start

```console
$ nix-shell              # Only on NixOS
$ mkdir build
$ cd build/
$ cmake ..
$ make
$ ./nothing <level-file>
$ ./nothing_test
```

## Editing Levels

Generally creating a level looks like:

```
SVG File -> Custom Level File -> Game
```

To convert SVG to the level file install [xqilla] and run
[svg2rects.xqe] script:

```console
$ xqilla ./devtools/svg2rects.xqe -i <svg-file> -o <level-file>
```

All of the levels reside in the [./levels/] folder. Use
[./levels/Makefile] to automatically rebuild all levels.

### Level Editing Workflow

1. `$ inkscape ./levels/level.svg &`
2. `$ ./build/nothing ./levels/level.txt &`
3. `$ make watch`
4. Edit Level in Inkscape and Save
5. Switch to the Game and reload level by pressing Q
6. Go to 1

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
[xqilla]: http://xqilla.sourceforge.net/HomePage
[svg2rects.xqe]: ./devtools/svg2rects.xqe
[./levels/]: ./levels/
[./levels/Makefile]: ./levels/Makefile
