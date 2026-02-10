
#pragma once
#include <cstdint>
#include <random>

constexpr uint16_t VIDEO_WIDTH = 64;
constexpr uint16_t VIDEO_HEIGHT = 32;

constexpr uint16_t START_ADDRESS = 0x200;
constexpr uint16_t FONTSET_START_ADDRESS = 0x50;
constexpr uint16_t FONTSET_SIZE = 80;

class Chip8
{
public:
    Chip8();

    void LoadROM(const char* filename);
    void Cycle();

    uint8_t  keypad[16]{};
    uint32_t video[VIDEO_WIDTH * VIDEO_HEIGHT]{};

private:

    std::default_random_engine randGen;
    std::uniform_int_distribution<int> randByte;
    uint8_t  registers[16]{};
    uint8_t  memory[4096]{};
    uint16_t index{};
    uint16_t pc{};
    uint16_t opcode{};
    uint16_t stack[16]{};
    uint8_t  sp{};
    uint8_t  delayTimer{};
    uint8_t  soundTimer{};

    using Chip8Func = void (Chip8::*)();
    Chip8Func table[16]{};
    Chip8Func table0[16]{};
    Chip8Func table8[16]{};
    Chip8Func tableE[16]{};
    Chip8Func tableF[256]{};

    // Opcode handlers
    void OP_NULL();
    void OP_00E0();
    void OP_00EE();
    void OP_1nnn();
    void OP_2nnn();
    void OP_3xkk();
    void OP_4xkk();
    void OP_5xy0();
    void OP_6xkk();
    void OP_7xkk();
    void OP_8xy0();
    void OP_8xy1();
    void OP_8xy2();
    void OP_8xy3();
    void OP_8xy4();
    void OP_8xy5();
    void OP_8xy6();
    void OP_8xy7();
    void OP_8xyE();
    void OP_9xy0();
    void OP_Annn();
    void OP_Bnnn();
    void OP_Cxkk();
    void OP_Dxyn();
    void OP_Ex9E();
    void OP_ExA1();
    void OP_Fx07();
    void OP_Fx0A();
    void OP_Fx15();
    void OP_Fx18();
    void OP_Fx1E();
    void OP_Fx29();
    void OP_Fx33();
    void OP_Fx55();
    void OP_Fx65();

    void Table0();
    void Table8();
    void TableE();
    void TableF();

    
};
