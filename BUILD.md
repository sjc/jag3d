# Building the Project

By default, `make` will build a `.cof` file. This type of file is also known as `.abs`. These can be used on real hardware with a Game Drive or Skunkboard, as well as with emulators. Everything is located in RAM.

## Building a .j64 ROM File

Build a ROM image with `make j64`. This places the code and data sections in the ROM address space.

Building the ROM image requires files from 42Bastian's [new_bjl repo](https://github.com/42Bastian/new_bjl). Download or clone the repo and then adjust the `BJL_BIN` variable in the Makefile to point to it.

If you experience issues with the `.j64` build which weren't present in the `.cof`, check that all your variables are in the bss section, and that you aren't trying to update constants from the data section.

## Building for More Performance

For best performance, the 68k game logic can be copied into DRAM and run from there. This involves building the game code (everything from main.c onwards) to run from RAM (so, the `.cof` path), and then having a small amount of 'boot' code which runs from ROM, sets up the hardware, and then copies the game code (possibly decompressing it) into DRAM. An example of this can be implemented if there's enough demand.
