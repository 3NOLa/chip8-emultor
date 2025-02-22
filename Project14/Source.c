#define _CRT_SECURE_NO_WARNINGS
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <SDL3/SDL.h>
#include "chip.h"

#define WINDOW_WIDTH 640  // 64 pixels * 10 scale
#define WINDOW_HEIGHT 320 // 32 pixels * 10 scale
#define PIXEL_SCALE 10  

#define CHIP8_CPU_HZ 500  // CPU speed - 500Hz is a good starting point
#define TIMER_HZ 60       // Timer speed - should be 60Hz

SDL_Window* window;
SDL_Renderer* renderer;


int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    // argv[1] contains the file path or filename
    char* filename = argv[1];
    printf("File to open: %s\n", filename);

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_Log("SDL could not initialize! SDL_Error: %s", SDL_GetError());
        return 1;
    }

    window = SDL_CreateWindow("chip 8 emulator", WINDOW_WIDTH, WINDOW_HEIGHT,0);
    if (!window) {
        SDL_Log("Window could not be created! SDL_Error: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    renderer = SDL_CreateRenderer(window, NULL,0);
    if (!renderer) {
        SDL_Log("Renderer could not be created! SDL_Error: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    chip cpu;
    chip_init(&cpu);
    load_program(&cpu, filename);
    srand(time(NULL));

    Uint64 previous_cpu_time = SDL_GetTicks();
    Uint64 previous_timer_time = SDL_GetTicks();

    const double CPU_INTERVAL = 1000.0 / CHIP8_CPU_HZ;    // Time between CPU cycles
    const double TIMER_INTERVAL = 1000.0 / TIMER_HZ;      // Time between timer updates


    int running = 1;
    SDL_Event event;
    while (running) {
        Uint64 current_time = SDL_GetTicks();

        // Handle input events
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = 0;
            }
            else if (event.type == SDL_EVENT_KEY_DOWN || event.type == SDL_EVENT_KEY_UP) {
                handle_input(&cpu, &event);
            }
        }

        // Update CPU at correct speed
        if (current_time - previous_cpu_time >= CPU_INTERVAL) {
            uint16_t opcode = fetch(&cpu);
            decode(&cpu, opcode);
            previous_cpu_time = current_time;
        }

        // Update timers at 60Hz
        if (current_time - previous_timer_time >= TIMER_INTERVAL) {
            if (cpu.delay > 0) cpu.delay--;
            if (cpu.sound > 0)
            {   
                printf("%c", 7);
                cpu.sound--;
            }
            previous_timer_time = current_time;
        }

        render_display(&cpu, renderer);
        SDL_Delay(1); // Small delay to prevent excessive CPU usage
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

void render_display(cptr cpu, SDL_Renderer* renderer) {
    // Set background to black and clear the screen
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // Set color for pixels (White)
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            // Correct indexing for CHIP-8 display (64x32)
            if (cpu->display[y * 64 + x]) {
                SDL_FRect rect = {
                    x * PIXEL_SCALE,  // Scale x-coordinate
                    y * PIXEL_SCALE,  // Scale y-coordinate
                    (float)PIXEL_SCALE, // Width
                    (float)PIXEL_SCALE  // Height
                };
                SDL_RenderFillRect(renderer, &rect);
            }
        }
    }

    SDL_RenderPresent(renderer);
} 

void handle_input(cptr cpu, SDL_Event* event) {
    int key_state = (event->type == SDL_EVENT_KEY_DOWN) ? 1 : 0;  // 1 for key press, 0 for release
    printf("Key event detected: %s\n", key_state ? "Pressed" : "Released"); // Debug output

    switch (event->key.key) {
    case SDLK_1: cpu->keyboard[0x1] = key_state; break;
    case SDLK_2: cpu->keyboard[0x2] = key_state; break;
    case SDLK_3: cpu->keyboard[0x3] = key_state; break;
    case SDLK_4: cpu->keyboard[0xC] = key_state; break;
    case SDLK_Q: cpu->keyboard[0x4] = key_state; break;
    case SDLK_W: cpu->keyboard[0x5] = key_state; break;
    case SDLK_E: cpu->keyboard[0x6] = key_state; break;
    case SDLK_R: cpu->keyboard[0xD] = key_state; break;
    case SDLK_A: cpu->keyboard[0x7] = key_state; break;
    case SDLK_S: cpu->keyboard[0x8] = key_state; break;
    case SDLK_D: cpu->keyboard[0x9] = key_state; break;
    case SDLK_F: cpu->keyboard[0xE] = key_state; break;
    case SDLK_Z: cpu->keyboard[0xA] = key_state; break;
    case SDLK_X: cpu->keyboard[0x0] = key_state; break;
    case SDLK_C: cpu->keyboard[0xB] = key_state; break;
    case SDLK_V: cpu->keyboard[0xF] = key_state; break;
    default:
        printf("Unknown key pressed: %d\n", event->key.key);
        break;  
    }
}

void handle_0_instructions(cptr cpu, uint16_t opcode);
void handle_8_instructions(cptr cpu, uint8_t X, uint8_t Y, uint16_t opcode);
void handle_E_instructions(cptr cpu, uint8_t X, uint16_t opcode);
void handle_D_instructions(cptr cpu, uint8_t X, uint8_t Y, uint16_t opcode);
void handle_F_instructions(cptr cpu, uint8_t X, uint16_t opcode);

void decode(cptr cpu, uint16_t opcode) {
    uint8_t instruction = (opcode & 0xF000) >> 12;
    uint8_t X = (opcode & 0x0F00) >> 8;
    uint8_t Y = (opcode & 0x00F0) >> 4;
    uint16_t adr = opcode & 0x0FFF;

    switch (instruction) {
    case 0x0: handle_0_instructions(cpu, opcode); break;
    case 0x1: cpu->pc = opcode & 0x0FFF; break;
    case 0x2:
        cpu->stack[cpu->sp++] = cpu->pc;
        cpu->pc = adr; // This was missing!
        break;
    case 0x3: if (cpu->registers[X] == (opcode & 0x00FF)) cpu->pc += 2; break;
    case 0x4: if (cpu->registers[X] != (opcode & 0x00FF)) cpu->pc += 2; break;
    case 0x5: if (cpu->registers[X] == cpu->registers[Y]) cpu->pc += 2; break;
    case 0x6: cpu->registers[X] = opcode & 0x00FF; break;
    case 0x7: cpu->registers[X] += opcode & 0x00FF; break;
    case 0x8: handle_8_instructions(cpu, X, Y, opcode); break;
    case 0x9: if (cpu->registers[X] != cpu->registers[Y]) cpu->pc += 2; break;
    case 0xA: cpu->I = adr; break;
    case 0xB: cpu->pc = adr + cpu->registers[0]; break;
    case 0xC: {
        uint8_t random = (uint8_t)(rand() % 250);
        cpu->registers[X] = random & (opcode & 0x00FF);
        break;
    }
    case 0xD: handle_D_instructions(cpu, X, Y, opcode); break; // Original empty case preserved
    case 0xE: handle_E_instructions(cpu, X, opcode); break;
    case 0xF: handle_F_instructions(cpu, X, opcode); break;
    default: printf("Unknown opcode: 0x%X\n", opcode); break;
    }
}

void handle_0_instructions(cptr cpu, uint16_t opcode) {
    if (((opcode & 0x0F00) >> 8) == 0) {
        switch (opcode & 0x000F) {
        case 0x0:
            if ((opcode & 0x00FF) == 0xE0) {
                // Clear screen in CPU
                for (int i = 0; i < (SCREEN_WIDTH * SCREEN_HEIGHT); i++)
                    cpu->display[i] = 0;

                //clear display
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                SDL_RenderClear(renderer);
                SDL_RenderPresent(renderer);
            }
            break;
        case 0xE:
            cpu->pc = cpu->stack[--cpu->sp];
            break;
        }
    }
    else {
        cpu->pc = opcode & 0x0FFF;
    }
}

void handle_8_instructions(cptr cpu, uint8_t X, uint8_t Y, uint16_t opcode) {
    uint16_t result, diff;

    switch (opcode & 0x000F) {
    case 0x0:
        cpu->registers[X] = cpu->registers[Y];
        break;
    case 0x1:
        cpu->registers[X] = cpu->registers[X] | cpu->registers[Y];
        break;
    case 0x2:
        cpu->registers[X] = cpu->registers[X] & cpu->registers[Y];
        break;
    case 0x3:
        cpu->registers[X] = cpu->registers[X] ^ cpu->registers[Y];
        break;
    case 0x4:
        result = cpu->registers[X] + cpu->registers[Y];
        cpu->registers[0xF] = (result > 0x00FF) ? 1 : 0;
        cpu->registers[X] = result & 0x00FF;
        break;
    case 0x5:
        cpu->registers[0xF] = (cpu->registers[X] > cpu->registers[Y]) ? 1 : 0;
        cpu->registers[X] -= cpu->registers[Y];
        break;
    case 0x6:
        cpu->registers[0xF] = cpu->registers[X] & 1;
        cpu->registers[X] >>= 1;
        break;
    case 0x7:
        diff = cpu->registers[Y] - cpu->registers[X];
        cpu->registers[0xF] = (cpu->registers[Y] > cpu->registers[X]) ? 1 : 0;
        cpu->registers[X] = diff;
        break;
    case 0xE:
        cpu->registers[0xF] = (cpu->registers[X] & 0x80) >> 7;
        cpu->registers[X] <<= 1;
        break;
    default:
        printf("Unknown inside 0x8 opcode: 0x%X\n", opcode);
        break;
    }
}

void handle_D_instructions(cptr cpu, uint8_t X, uint8_t Y, uint16_t opcode)
{
    uint8_t height = opcode & 0x000F;

    // Reset collision flag
    cpu->registers[0xF] = 0;

    // Get starting coordinates from registers VX and VY
    uint8_t xCrd = cpu->registers[X] % SCREEN_WIDTH;
    uint8_t yCrd = cpu->registers[Y] % SCREEN_HEIGHT;

    // Loop through each row of the sprite
    for (int row = 0; row < height; row++) {
        // Get the sprite data for this row from memory at I
        uint8_t spriteByte = cpu->memory[cpu->I + row];

        // If we're trying to draw past the bottom of the screen, stop
        if (yCrd + row >= SCREEN_HEIGHT) break;

        // Loop through each bit in the sprite byte
        for (int col = 0; col < 8; col++) {
            // If we're trying to draw past the right of the screen, stop this row
            if (xCrd + col >= SCREEN_WIDTH) break;

            // Get the bit from the sprite byte, starting from the leftmost bit
            uint8_t spritePixel = (spriteByte & (0x80 >> col));

            // Calculate the index in the display buffer
            uint32_t displayIndex = ((yCrd + row) * SCREEN_WIDTH) + (xCrd + col);

            // If the sprite pixel is set
            if (spritePixel) {
                // If the display pixel is already set, record the collision
                if (cpu->display[displayIndex]) {
                    cpu->registers[0xF] = 1;
                }
                // XOR the pixel
                cpu->display[displayIndex] ^= 1;
            }
        }
    }
}

void handle_E_instructions(cptr cpu, uint8_t X, uint16_t opcode) {
    if ((opcode & 0x00FF) == 0x9E) {
        if (cpu->keyboard[cpu->registers[X]])
            cpu->pc += 2;
    }
    else {
        if (!cpu->keyboard[cpu->registers[X]])
            cpu->pc += 2;
    }
}

void handle_F_instructions(cptr cpu, uint8_t X, uint16_t opcode) {
    uint8_t value;

    switch (opcode & 0x00FF) {
    case 0x07:
        cpu->registers[X] = cpu->delay;
        break;
    case 0x0A: {
        bool keyPressed = false;
        for (int i = 0; i < NUM_KEYS; i++) {
            if (cpu->keyboard[i]) {
                cpu->registers[X] = i;
                keyPressed = true;
                break;
            }
        }
        if (!keyPressed) {
            cpu->pc -= 2;  // Return to this instruction until a key is pressed
        }
        break;
    }
    case 0x15:
        cpu->delay = cpu->registers[X];
        break;
    case 0x18:
        cpu->sound = cpu->registers[X];
        break;
    case 0x29:
        // Fx29 - Set I to the location of the sprite for digit Vx (each font sprite is 5 bytes long)
        cpu->I = (cpu->registers[X] * 5);
        break;
    case 0x1E:
        cpu->registers[0xF] = (cpu->I + cpu->registers[X] > 0xFFF) ? 1 : 0;
        cpu->I += cpu->registers[X];
        break;
    case 0x33:
        value = cpu->registers[X];
        cpu->memory[cpu->I] = value / 100;
        cpu->memory[cpu->I + 1] = (value / 10) % 10;
        cpu->memory[cpu->I + 2] = value % 10;
        break;
    case 0x55:
        for (int i = 0; i <= X; i++)  // Note: should be <= X, not < 16
            cpu->memory[cpu->I + i] = cpu->registers[i];
        cpu->I += X + 1;
        break;
    case 0x65:
        for (int i = 0; i < X; i++)
            cpu->registers[i] = cpu->memory[cpu->I + i];
        break;
    default:
        printf("Unknown opcode in 0xF opcode: 0x%X\n", opcode);
        break;
    }
}

uint16_t fetch(cptr cpu)
{
    uint16_t in1 = cpu->memory[cpu->pc];
    uint16_t in2 = cpu->memory[cpu->pc + 1];
    cpu->pc += 2;

    uint16_t opcode = (uint16_t)((in1 << 8) | in2);
    //printf("Fetched opcode: 0x%04X at PC: 0x%03X\n", opcode, cpu->pc - 2);

    return opcode;
}

void load_program(cptr cpu, char* filename)
{
    FILE* f = fopen(filename, "rb");
    if (!f) {
        printf("Failed to open ROM file!\n");
        return;
    }

    size_t bytesRead = fread(&cpu->memory[PROGRAM_START_ADDR], 1, TOTAL_RAM - PROGRAM_START_ADDR, f);
    fclose(f);

    if (bytesRead > 0) {
        printf("ROM loaded successfully, %zu bytes read.\n", bytesRead);
        // Print first few bytes of the ROM
        for (int i = 0; i < 10; i++) {
            printf("Byte %d: 0x%02X\n", i, cpu->memory[PROGRAM_START_ADDR + i]);
        }
    }
    else {
        printf("Error: ROM is empty or failed to load!\n");
    }
}

void debug_print_display(cptr cpu) {
    printf("Display buffer contents:\n");
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            printf("%c", cpu->display[y * SCREEN_WIDTH + x] ? '#' : '.');
        }
        printf("\n");
    }
}