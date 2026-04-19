#include "instructions.hpp"
#include "globals.h"
#include <SDL3/SDL_main.h>

#define OPCODE(i)  ((i & 0xF000) >> 12)
#define X(i)       ((i & 0x0F00) >> 8)
#define Y(i)       ((i & 0x00F0) >> 4)
#define N(i)       ((i & 0x000F))
#define NN(i)      ((i & 0x00FF))
#define NNN(i)     ((i & 0x0FFF))

uint8_t font[] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
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
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

void decode(uint16_t instruction, Machine &m) {
    switch(OPCODE(instruction)){
        case(0x0):
            switch(NNN(instruction)){
                case(0x0E0): // 00E0: clear screen
                   memset(m.display, 0, sizeof(m.display));
                    break;
            }
            break;
        
        case(0x1): // 1NNN: jump to NNN
            m.PC = NNN(instruction);
            break;

        case(0x6): // 6XNN: Set VX to NN
            m.V[X(instruction)] = NN(instruction);
            break;

        case(0x7): // 7XNN: Add NN to VX
            m.V[X(instruction)] += NN(instruction);
            break;

        case(0xA): // ANNN: Set I register to NNN
            m.I = NNN(instruction);
            break;
        
        case(0xD):
            m.V[0xF] = 0;
            uint8_t y_coord = m.V[Y(instruction)] & 31;
            for (uint8_t i = 0; i < N(instruction); i++) {
                uint8_t x_coord = m.V[X(instruction)] & 63;
                uint8_t sprite_byte = m.memory[m.I + i];
                for (int bit = 7; bit >= 0; bit--) { // loop through the sprite_byte from most to least significant bit
                    if (x_coord >= 64) break;
                    bool pixel = (sprite_byte >> bit) & 1; 
                    if (pixel && m.display[y_coord * 64 + x_coord]) m.V[0xF] = 1;
                    m.display[y_coord * 64 + x_coord] ^= pixel;
                    x_coord++;
                }
                y_coord++;
                if (y_coord >= 32) break;
            }
        
    }
}