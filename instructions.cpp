#include "instructions.hpp"
#include "globals.h"
#include <SDL3/SDL_main.h>

#define OPCODE(i)  ((i & 0xF000) >> 12)
#define X(i)       ((i & 0x0F00) >> 8)
#define Y(i)       ((i & 0x00F0) >> 4)
#define N(i)       ((i & 0x000F))
#define NN(i)      ((i & 0x00FF))
#define NNN(i)     ((i & 0x0FFF))

#define SHIFT_IMPLEMENTATION "old"

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
                case(0x0EE): // 00EE: return from subrutine
                    m.PC = m.pila.top();
                    m.pila.pop();
                    break;
            }
            break;
        
        case(0x1): // 1NNN: jump to NNN
            m.PC = NNN(instruction);
            break;
        
        case(0x2): // 2NNN: Call a subroutine
            m.pila.push(m.PC);
            m.PC = NNN(instruction);

        case(0x3): // 3XNN: Skip one instruction if VX == NN
            if(m.V[X(instruction)] == NN(instruction)) {
                m.PC += 0x2;
            }
            break;
        
        case(0x4): // 4XNN: Skip one instruction if VX != NN
            if(m.V[X(instruction)] != NN(instruction)) {
                m.PC += 0x2;
            }
            break;
        
        case(0x5): // 5XY0: Skip one instruction if VX == VY
            if(m.V[X(instruction)] == m.V[Y(instruction)]) {
                m.PC += 0x2;
            }
            break;

        case(0x6): // 6XNN: Set VX to NN
            m.V[X(instruction)] = NN(instruction);
            break;

        case(0x7): // 7XNN: Add NN to VX
            m.V[X(instruction)] += NN(instruction);
            break;

        // Logical and arithmetic instructions
        case(0x8): 
            switch(N(instruction)){
                case(0x0): { // 8XY0: Set VX to VY
                    m.V[X(instruction)] = m.V[Y(instruction)];
                    break;
                }
                case(0x1): { // 8XY1: VX = VX or VY
                    m.V[X(instruction)] |= m.V[Y(instruction)];
                    break;
                }
                case(0x2): { // 8XY2: VX = VX and VY
                    m.V[X(instruction)] &= m.V[Y(instruction)];
                    break;
                }
                case(0x3): {// 8XY3: VX = VX xor VY
                    m.V[X(instruction)] ^= m.V[Y(instruction)];
                    break;
                }
                case(0x4): {// 8XY4: VX = VX + VY
                    uint16_t sum = m.V[X(instruction)] + m.V[Y(instruction)];
                    m.V[X(instruction)] = sum & 0xFF;
                    m.V[0xF] = (sum > 0xFF) ? 1 : 0;
                    break;
                }
                case(0x5): {// 8XY5: VX = VX - VY 
                    uint8_t vx = m.V[X(instruction)];
                    uint8_t vy = m.V[Y(instruction)];
                    m.V[X(instruction)] = vx - vy;
                    m.V[0xF] = (vx >= vy) ? 1 : 0;
                    break;
                }
                case(0x6): {// 8XY6: VX >>= VY
                    if(SHIFT_IMPLEMENTATION == "old") {
                        m.V[X(instruction)] = m.V[Y(instruction)];
                    }
                    m.V[0xF] = m.V[X(instruction)] & 1; // save LSB before shifting
                    m.V[X(instruction)] >>= 1;
                    break;
                }
                case(0x7): {// 8XY7: VX = VY - VX
                    uint8_t vx = m.V[X(instruction)];
                    uint8_t vy = m.V[Y(instruction)];
                    m.V[X(instruction)] = vy - vx;
                    m.V[0xF] = (vy >= vx) ? 1 : 0;
                    break;
                }
                case(0xE): {// 8XYE" VX <<= VY
                    if(SHIFT_IMPLEMENTATION == "old") {
                        m.V[X(instruction)] = m.V[Y(instruction)];
                    }
                    m.V[0xF] = m.V[X(instruction)] & 0x80; // save LSB before shifting
                    m.V[X(instruction)] <<= 1;
                    break;
                } 
                }
            break;

        case(0x9): // 9XY0: Skip one instruction if VX != VY
            if(m.V[X(instruction)] != m.V[Y(instruction)]) {
                m.PC += 0x2;
            }
            break;

        case(0xA): // ANNN: Set I register to NNN
            m.I = NNN(instruction);
            break;
        
        case(0xD): // DXYN: Draw sprite on VX and VY of length N and set VF = 1 on collision
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