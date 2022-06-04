# Particle_Sim

## Description

This project is for learning aspects of graphics programming and reviewing basic physics.

## Cloning

Clone the repository recursively or use `git submodule update` in the following way to include submodules
```
git clone --recurse-submodules <url>
```
or
```
git submodule update --init --recurisve
```

## Dependencies

 * [CMake](https://cmake.org/download/) (version 3.22 or higher)
 * [MinGW-W64](https://www.mingw-w64.org/) ([MSYS2](https://www.msys2.org/) has a MINGW64 terminal as well)
   * You will also need the following packages: [mingw-w64-cmake](https://packages.msys2.org/base/mingw-w64-cmake), and [mingw-w64-make](https://packages.msys2.org/base/mingw-w64-make)
 * [Ruby](https://www.ruby-lang.org/en/documentation/installation/) (for Unity scripts)

## Building

This project is expected to be built in an MINGW64 environment.  Currently building has been tested in MSYS2 MinGW x64 bash terminal.
```
./build.sh
```
