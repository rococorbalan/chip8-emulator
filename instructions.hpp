#pragma once
#include "globals.h"
#include <SDL3/SDL_main.h>
#include <stack>
using namespace std;

extern uint8_t font[];
const int FONT_SIZE = 80;

struct Machine {
    uint8_t* memory = new uint8_t[0x1000]{}; // 4KB initialized to 0

    stack<uint16_t> pila; // stack

    uint8_t sound_timer = 0;
    uint8_t delay_timer = 0;

    uint64_t last_tick = 0; // Used for timers
    uint64_t last_cpu_tick = 0; // Used for fetching instructions

    // True if a key is pressed down
    bool keypad[16] = {};

    uint16_t PC = 0x200; // Program counter
    uint16_t I; // Index (points to memory)

    uint8_t V[16] = {};

    bool display[64 * 32] = {};
    bool display_dirty = false;

    Machine() {
        // Set font in memory
        memcpy(memory + 0x50, font, FONT_SIZE);
    }

    // Fetch instruction and increment PC
    uint16_t fetch() {
        uint16_t instruction = (memory[PC] << 8) | memory[PC + 1];
        PC += 2;
        return instruction;
    }
};

void decode(uint16_t instruction, Machine &m);
void handle_input(SDL_Event *event, Machine &m);
