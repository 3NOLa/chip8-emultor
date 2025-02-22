#include <stdint.h>
#include "SDL3/SDL.h"
#include "SDL3/SDL_main.h"

#define NUM_KEYS 16
#define TOTAL_RAM 4096
#define STACK_SIZE 16
#define FONTSET_SIZE 80
#define NUM_REGISTERS 16
#define TIMER_MAX 255

#define CHIP8_RAM_START_ADDR 0x000
#define CHIP8_RAM_END_ADDR 0x1FF
#define PROGRAM_START_ADDR 0x200
#define PROGRAM_END_ADDR 0xFFF

#define SCREEN_WIDTH 64
#define SCREEN_HEIGHT 32

typedef struct chip
{
	//special registers uint8 ==  unsigned int 8 bit
	uint16_t pc; //store the currently executing address (program counter)
	uint16_t I; //stores adress so only the 12 first bits are being used
	uint8_t sp; //stack pointer
	uint8_t delay;
	uint8_t sound;

	//registers
	uint8_t registers[NUM_REGISTERS];

	uint16_t stack[STACK_SIZE]; // a stack that store the address that the interpreter shoud return to

	uint8_t keyboard[NUM_KEYS]; // the keyboard of the Chip from 1-F as HEX

	uint8_t memory[TOTAL_RAM]; // the CHIP 8 have 4096 BYTES of memory

	uint8_t  display[SCREEN_WIDTH * SCREEN_HEIGHT]; // the frame that the user see have 64 * 32 pixels with on and off
}chip, * cptr;

void chip_init(cptr cpu);
uint16_t fetch(cptr cpu);
void decode(cptr cpu, uint16_t opcode);
void load_program(cptr cpu, char* filename);
void handle_input(cptr cpu, SDL_Event* event);
void render_display(cptr cpu, SDL_Renderer* renderer);
void debug_print_display(cptr cpu);

static const uint8_t chip8_fontset[] = { 0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
	 0x20, 0x60, 0x20, 0x20, 0x70, // 1
	 0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
	 0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
	 0x90, 0x90, 0xF0, 0x10, 0x10, // 4
	 0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
	 0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
	 0xF0, 0x10, 0x20, 0x40, 0x40, // 7
	 0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
	 0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
	 0xF0, 0x90, 0xF0, 0x90, 0x90, // A
	 0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
	 0xF0, 0x80, 0x80, 0x80, 0xF0, // C
	 0xE0, 0x90, 0x90, 0x90, 0xE0, // D
	 0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
	 0xF0, 0x80, 0xF0, 0x80, 0x80 };  // F

void chip_init(cptr cpu)
{
	cpu->pc = 0x200;  // Program counter starts at 0x200
	cpu->I = 0;      // Reset index register
	cpu->sp = 0;      // Reset stack pointer

	for (int i = 0; i < (SCREEN_WIDTH * SCREEN_HEIGHT); i++)// Clear display
		cpu->display[i] = 0;

	for (int i = 0; i < STACK_SIZE; i++)// Clear stack
		cpu->stack[i] = 0;

	for (int i = 0; i < NUM_REGISTERS; i++)// Clear registers V0-VF
		cpu->registers[i] = 0;

	for (int i = 0; i < TOTAL_RAM; i++)// Clear memory
		cpu->memory[i] = 0;

	// Start keyboard state as all unpressed
	for (auto i = 0; i < NUM_KEYS; i++)
		cpu->keyboard[i] = 0;

	// Load fontset
	for (int i = 0; i < FONTSET_SIZE; ++i)
		cpu->memory[i] = chip8_fontset[i];

	// Reset timers	
	cpu->delay = 0;
	cpu->sound = 0;
}

