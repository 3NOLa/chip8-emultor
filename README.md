# CHIP-8 Emulator

A simple and efficient CHIP-8 emulator written in C, using SDL for graphics, input, and timing. This project faithfully emulates the CHIP-8 virtual machine, allowing you to run classic CHIP-8 programs and games.

## Features
- Full implementation of the CHIP-8 instruction set
- Graphical rendering using SDL
- Keyboard input handling
- Timers and sound support
- ROM loading capability

## Prerequisites
Make sure you have the following installed:
- GCC or Clang (or another C compiler)
- SDL3 (Simple DirectMedia Layer)
- Make (optional, for easy building)

## Building the Emulator
Clone the repository and compile the project:
```sh
git clone <repo-url>
cd chip8-emulator
make  # If using Makefile
# OR manually compile:
gcc -o chip8 main.c chip8.c -lSDL3
```

## Running a ROM
To run a CHIP-8 game or program:
```sh
./chip8 path/to/rom.ch8
```

## Controls
Most CHIP-8 programs use a 4x4 keypad mapped as follows:
```
1 2 3 C      ->     1 2 3 4
4 5 6 D      ->     Q W E R
7 8 9 E      ->     A S D F
A 0 B F      ->     Z X C V
```
You can use these keys to interact with CHIP-8 programs.

## Roadmap / Future Improvements
- Super CHIP-8 support
- Customizable key bindings
- Debugging tools (step execution, memory viewer)
- Performance optimizations

## References
- [CHIP-8 Instruction Set]([https://en.wikipedia.org/wiki/CHIP-8](http://devernay.free.fr/hacks/chip8/C8TECH10.HTM)
- [Multigesture.net CHIP-8 Guide](http://www.multigesture.net/articles/how-to-write-an-emulator-chip-8-interpreter/)

## License
This project is licensed under the MIT License. Feel free to modify and distribute it.

---

Feel free to contribute by submitting issues or pull requests!

