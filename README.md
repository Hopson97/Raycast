# raycast

Minimal Raycaster created using SFML and C++, with some map editing capabilities.

Useful resources for learning how to do this:

https://permadi.com/1996/05/ray-casting-tutorial-table-of-contents/

https://lodev.org/cgtutor/raycasting.html

www.youtube.com/watch?v=gYRrGTC7GtA

GIF:

![Raycaster GIF](https://i.imgur.com/tQFahuc.gif)

## Building and Running

### Libraries

SFML is required.

These can be installed from your project manager. For example, on Debian/ Ubuntu:

```sh
sudo apt install libsfml-dev
```

If this is not possible (eg windows), you can install these manually from their respective websites:

https://www.sfml-dev.org/download.php

### Linux

To build, at the root of the project:

```sh
sh scripts/build.sh
```

To run, at the root of the project:

```sh
sh scripts/run.sh
```

To build and run in release mode, simply add the `release` suffix:

```sh
sh scripts/build.sh release
sh scripts/run.sh release
```

You can also create a deployable build (that can be sent) by doing:

```sh
sh scripts/deploy.sh
```

