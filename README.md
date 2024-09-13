## Prerequisites
- [Meson](https://mesonbuild.com/SimpleStart.html#installing-meson)
- [MinGW](https://www.mingw-w64.org/) (Windows only)

Run `setup.ps1` or `setup.sh` (depending on your system) to set up the project.
To compile and run the program:
```bash
meson compile -C build
./build/lowlevelgame  
```
## Controls
- WASD to move
- Space to fly up
- Shift to fly down
- Mouse to look around
- Mouse wheel to zoom
- ESC to unlock cursor
