#include "instructions.hpp"
#include "globals.h"
#include <SDL3/SDL_main.h>
#include <bits/stdc++.h>
#include <unordered_map>

#define OPCODE(i)  ((i & 0xF000) >> 12)
#define X(i)       ((i & 0x0F00) >> 8)
#define Y(i)       ((i & 0x00F0) >> 4)
#define N(i)       ((i & 0x000F))
#define NN(i)      ((i & 0x00FF))
#define NNN(i)     ((i & 0x0FFF))

#define SHIFT_IMPLEMENTATION "old"
#define JUMP_IMPLEMENTATION "old"
#define STORE_LOAD_IMPLEMENTATION "new"

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

unordered_map<SDL_Scancode, uint8_t> keymap = {
    {SDL_SCANCODE_X, 0x0},
    {SDL_SCANCODE_1, 0x1},
    {SDL_SCANCODE_2, 0x2},
    {SDL_SCANCODE_3, 0x3},
    {SDL_SCANCODE_Q, 0x4},
    {SDL_SCANCODE_W, 0x5},
    {SDL_SCANCODE_E, 0x6},
    {SDL_SCANCODE_A, 0x7},
    {SDL_SCANCODE_S, 0x8},
    {SDL_SCANCODE_D, 0x9},
    {SDL_SCANCODE_Z, 0xA},
    {SDL_SCANCODE_C, 0xB},
    {SDL_SCANCODE_4, 0xC},
    {SDL_SCANCODE_R, 0xD},
    {SDL_SCANCODE_F, 0xE},
    {SDL_SCANCODE_V, 0xF}
};

void handle_input(SDL_Event *event, Machine &m) {
    auto it = keymap.find(event->key.scancode);
    if (it != keymap.end()) {
        m.keypad[it->second] = (event->type == SDL_EVENT_KEY_DOWN);
    }
}

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
                    m.V[0xF] = m.V[X(instruction)] & 0x80 >> 7; // save LSB before shifting
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

        case(0xB): // BNNN: Jump to NNN + V 
            if(JUMP_IMPLEMENTATION == "old") {
                m.PC += NNN(instruction) + m.V[0x0];
            } else {
                m.PC += NNN(instruction) + m.V[X(instruction)];
            }
            break;
        
        case(0xC): // CXNN: VX = Random number 0-255 AND NN
            m.V[X(instruction)] = (rand() % 256) & NN(instruction);
            break;

        case(0xD):{ // DXYN: Draw sprite on VX and VY of length N and set VF = 1 on collision
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
            break;
        }

        case(0xE):
            switch(NN(instruction)) {
                case(0x9E): // EX9E Skip one instruction if VX key is pressed
                    if(m.keypad[m.V[X(instruction)]]) {
                        m.PC += 0x2;
                    }
                break;
                case(0xA1): // EXA1 Skip one instruction if VX key is not pressed
                    if(!m.keypad[m.V[X(instruction)]]) {
                        m.PC += 0x2;
                    }
                break;
            }
            break;
        
        case(0xF):
            switch(NN(instruction)) {
                case(0x07): // FX07: Sets VX to the value of the delay timer
                    m.V[X(instruction)] = m.delay_timer;
                    break;
                case(0x15): // FX15: Sets the delay timer to the value of VX
                    m.delay_timer = m.V[X(instruction)];
                    break;
                case(0x18): // FX18: Sets the sound timer to the value of VX
                    m.sound_timer = m.V[X(instruction)];
                    break;
                case(0x1E):{ // FX1E: Adds VX to I (with carry flag)
                    uint16_t sum = m.V[X(instruction)] + m.I;
                    m.I = sum & 0xFFF;
                    m.V[0xF] = (sum > 0xFFF) ? 1 : 0;
                    break;
                }
                case(0x0A):{ // FX0A: Waits for a key press and saves it in VX
                    bool key_pressed = false;
                    for (uint8_t i = 0; i < 16; i++) {
                        if (m.keypad[i]) {
                            m.V[X(instruction)] = i;
                            key_pressed = true;
                            break;
                        }
                    }
                    if (!key_pressed) m.PC -= 2;
                    break;
                }
                case(0x29): // FX29: Sets I as hex char in VX
                    m.I = 0x50 + (m.V[X(instruction)] * 5);
                    break;
                case(0x33): { // FX33: Decode VX into a binary-coded decimal
                    uint8_t number = m.V[X(instruction)];
                    for(uint8_t i = 0; i < 3; i++){
                        m.memory[m.I + 2 - i] = number % 10;
                        number = number / 10;
                    }
                    break;
                }
                case(0x55): // FX55: Save V0 - VX to I
                    if(STORE_LOAD_IMPLEMENTATION == "new"){
                        for(uint8_t i = 0; i <= X(instruction); i++) {
                            m.memory[m.I + i] = m.V[i];
                        }
                    }else {
                        for(uint8_t i = 0; i <= X(instruction); i++) {
                            m.memory[m.I] = m.V[i];
                            m.I += 1;
                        }
                    }
                    break;
                case(0x65): // FX65: Load V0 - VX from I
                    if(STORE_LOAD_IMPLEMENTATION == "new"){
                        for(uint8_t i = 0; i <= X(instruction); i++) {
                            m.V[i] = m.memory[m.I + i];
                        }
                    }else {
                        for(uint8_t i = 0; i <= X(instruction); i++) {
                            m.V[i] = m.memory[m.I];
                            m.I += 1;
                        }
                    }
                    break;
            }
            break;
    }
}