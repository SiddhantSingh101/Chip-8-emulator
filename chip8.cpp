

//because mem space before is used for interpreter logic and stuff so we start from 200
#include "chip8.h"
#include <fstream>
#include <chrono>
#include <random>
#include <cstring>
#include <iostream>

static const uint8_t fontset[FONTSET_SIZE] =
{
	0xF0,0x90,0x90,0x90,0xF0,
	0x20,0x60,0x20,0x20,0x70,
	0xF0,0x10,0xF0,0x80,0xF0,
	0xF0,0x10,0xF0,0x10,0xF0,
	0x90,0x90,0xF0,0x10,0x10,
	0xF0,0x80,0xF0,0x10,0xF0,
	0xF0,0x80,0xF0,0x90,0xF0,
	0xF0,0x10,0x20,0x40,0x40,
	0xF0,0x90,0xF0,0x90,0xF0,
	0xF0,0x90,0xF0,0x10,0xF0,
	0xF0,0x90,0xF0,0x90,0x90,
	0xE0,0x90,0xE0,0x90,0xE0,
	0xF0,0x80,0x80,0x80,0xF0,
	0xE0,0x90,0x90,0x90,0xE0,
	0xF0,0x80,0xF0,0x80,0xF0,
	0xF0,0x80,0xF0,0x80,0x80
};

Chip8::Chip8()
	: randGen(std::chrono::system_clock::now().time_since_epoch().count()),
	randByte(0, 255)
{
	pc = START_ADDRESS;

	for (int i = 0; i < FONTSET_SIZE; ++i)
		memory[FONTSET_START_ADDRESS + i] = fontset[i];

	for (auto& f : table)  f = &Chip8::OP_NULL;
	for (auto& f : table0) f = &Chip8::OP_NULL;
	for (auto& f : table8) f = &Chip8::OP_NULL;
	for (auto& f : tableE) f = &Chip8::OP_NULL;
	for (auto& f : tableF) f = &Chip8::OP_NULL;

	table[0x0] = &Chip8::Table0;
	table[0x1] = &Chip8::OP_1nnn;
	table[0x2] = &Chip8::OP_2nnn;
	table[0x3] = &Chip8::OP_3xkk;
	table[0x4] = &Chip8::OP_4xkk;
	table[0x5] = &Chip8::OP_5xy0;
	table[0x6] = &Chip8::OP_6xkk;
	table[0x7] = &Chip8::OP_7xkk;
	table[0x8] = &Chip8::Table8;
	table[0x9] = &Chip8::OP_9xy0;
	table[0xA] = &Chip8::OP_Annn;
	table[0xB] = &Chip8::OP_Bnnn;
	table[0xC] = &Chip8::OP_Cxkk;
	table[0xD] = &Chip8::OP_Dxyn;
	table[0xE] = &Chip8::TableE;
	table[0xF] = &Chip8::TableF;

	// Table 0
	table0[0x0] = &Chip8::OP_00E0; // CLS
	table0[0xE] = &Chip8::OP_00EE; // RET

	// Table 8
	table8[0x0] = &Chip8::OP_8xy0;
	table8[0x1] = &Chip8::OP_8xy1;
	table8[0x2] = &Chip8::OP_8xy2;
	table8[0x3] = &Chip8::OP_8xy3;
	table8[0x4] = &Chip8::OP_8xy4;
	table8[0x5] = &Chip8::OP_8xy5;
	table8[0x6] = &Chip8::OP_8xy6;
	table8[0x7] = &Chip8::OP_8xy7;
	table8[0xE] = &Chip8::OP_8xyE;

	// Table E
	tableE[0x1] = &Chip8::OP_ExA1;
	tableE[0xE] = &Chip8::OP_Ex9E;

	// Table F (Note: These use the last TWO digits 0xXX)
	tableF[0x07] = &Chip8::OP_Fx07;
	tableF[0x0A] = &Chip8::OP_Fx0A;
	tableF[0x15] = &Chip8::OP_Fx15;
	tableF[0x18] = &Chip8::OP_Fx18;
	tableF[0x1E] = &Chip8::OP_Fx1E;
	tableF[0x29] = &Chip8::OP_Fx29;
	tableF[0x33] = &Chip8::OP_Fx33;
	tableF[0x55] = &Chip8::OP_Fx55;
	tableF[0x65] = &Chip8::OP_Fx65;
}


void Chip8::LoadROM(char const* filename)
{

	std::ifstream file(filename, std::ios::binary | std::ios::ate);

	if (file.is_open())
	{

		std::streampos size = file.tellg();
		char* buffer = new char[size];
		if (size > (4096 - START_ADDRESS))
		{
			// ROM too large
			delete[] buffer;
			return;

			
		}


		file.seekg(0, std::ios::beg);
		file.read(buffer, size);
		file.close();


		for (long i = 0; i < size; ++i)
		{
			memory[START_ADDRESS + i] = buffer[i];
		}

		// Free the buffer
		delete[] buffer;
	}
};
//16 characters each of which are represesnted with bytes, hence 5x16 = 80.



//constructor for chip8 to initialize the value of program counter and loading fonts into memory(to emulate real CPU behavior we load the font data)


//opcode
//cls 00E0 (clear display)

void Chip8::OP_00E0() {
	memset(video, 0, sizeof(video));
}
//RET(return from subroutine)

void Chip8::OP_00EE()
{
	--sp;
	pc = stack[sp];

}
//Jp addr
//jump t0 address no stack incrementation or change straight pc jump 0x0FFFu is a mask to keep first 12 bits only ie 0000 1111 1111 1111
void Chip8::OP_1nnn()
{
	uint16_t address = opcode & 0x0FFFu;
	pc = address;
}
//Call addr
// jump to address with change and incrememnt in stack and pc
void Chip8::OP_2nnn()
{
	uint16_t address = opcode & 0x0FFFu;
	stack[sp] = pc;
	++sp;
	pc = address;
}
//Vx == k instruction skipper;
//3xkk format = 0000 xxxx(register address) kkkk kkkk(comparision byte)
void Chip8::OP_3xkk()
{
	uint16_t Vx = (opcode & 0x0F00u) >> 8u;
	uint16_t byte = opcode & 0x00FFu;
	if (registers[Vx] == byte)
	{
		pc += 2;
	}
}
//skip next instruction if Vx != byte
void Chip8::OP_4xkk()
{
	uint16_t Vx = (opcode & 0x0F00u) >> 8u;
	uint16_t byte = opcode & 0x00FFu;
	if (registers[Vx] != byte)
	{
		pc += 2;
	}
}
//skip next instruction if Vx==Vy
void Chip8::OP_5xy0()
{
	uint16_t Vx = (opcode & 0x0F00u) >> 8u;
	uint16_t Vy = (opcode & 0x00F0u) >> 4u;
	if (registers[Vx] == registers[Vy])
	{
		pc += 2;
	}
}
//set Vx == kk
void Chip8::OP_6xkk()
{
	uint16_t Vx = (opcode & 0x0F00u) >> 8u;
	uint16_t byte = opcode & 0x00FFu;
	registers[Vx] = byte;

}
//set Vx = Vx+kk
void Chip8::OP_7xkk()
{
	uint16_t Vx = (opcode & 0x0F00u) >> 8u;
	uint16_t byte = opcode & 0x00FFu;
	registers[Vx] += byte;

}
//register Vx=Vy
void Chip8::OP_8xy0()
{
	uint16_t Vx = (opcode & 0x0F00u) >> 8u;
	uint16_t Vy = (opcode & 0x00F0u) >> 4u;
	registers[Vx] = registers[Vy];

}
//OR operation between registers
void Chip8::OP_8xy1()
{
	uint16_t Vx = (opcode & 0x0F00u) >> 8u;
	uint16_t Vy = (opcode & 0x00F0u) >> 4u;
	registers[Vx] |= registers[Vy];

}
//AND operation between registers
void Chip8::OP_8xy2()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	registers[Vx] &= registers[Vy];
}
//XOR bw registers
void Chip8::OP_8xy3()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	registers[Vx] ^= registers[Vy];
}
//set Vx = Vx + Vy and set Carry accordingly
void Chip8::OP_8xy4()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	uint16_t sum = registers[Vx] + registers[Vy];

	if (sum > 255U)
	{
		registers[0xF] = 1;
	}
	else
	{
		registers[0xF] = 0;
	}

	registers[Vx] = sum & 0xFFu;
}
//set Vx=Vx-Vy set carry accordingly
void Chip8::OP_8xy5()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;



	if (registers[Vy] > registers[Vx])
	{
		registers[0xF] = 1;
	}
	else
	{
		registers[0xF] = 0;
	}

	registers[Vx] -= registers[Vy];
}
//divide by two and save if division was even or odd in Vf
void Chip8::OP_8xy6()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	// Save least-significant bit
	registers[0xF] = registers[Vx] & 0x01u;

	// Shift right
	registers[Vx] >>= 1;
}
//Vy-Vx
void Chip8::OP_8xy7()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	if (registers[Vy] > registers[Vx])
	{
		registers[0xF] = 1;
	}
	else
	{
		registers[0xF] = 0;
	}

	registers[Vx] = registers[Vy] - registers[Vx];
}
//multiply Vx by two and save msb in VF
void Chip8::OP_8xyE()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	// Save MSB in VF
	registers[0xF] = (registers[Vx] & 0x80u) >> 7u;

	registers[Vx] <<= 1;
}
//skip if register Vx != Vy
void Chip8::OP_9xy0()
{
	uint16_t Vx = (opcode & 0x0F00u) >> 8u;
	uint16_t Vy = (opcode & 0x00F0u) >> 4u;
	if (registers[Vx] != registers[Vy])
	{
		pc += 2;
	}
}
//Set I = nnn.
void Chip8::OP_Annn()
{
	uint16_t address = opcode & 0x0FFFu;

	index = address;
}
//Bnnn - JP V0, addr

//Jump to location nnn + V0. relative jump
void Chip8::OP_Bnnn()
{
	uint16_t address = opcode & 0x0FFFu;

	pc = registers[0] + address;
}
//Cxkk - RND Vx, byte
//Set Vx = random byte AND kk.
void Chip8::OP_Cxkk()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t byte = opcode & 0x00FFu;

	registers[Vx] = static_cast<uint8_t>(randByte(randGen)) & byte;
}
//display n byte sprite at coords vx and vy sprite pixel XOR with screen pixel to check collision
void Chip8::OP_Dxyn()
{
	// 0x0F00u >> 8 gives us the 'x' in DxyN
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;
	uint8_t height = opcode & 0x000Fu;

	// Modulo prevents crashing if registers contain values > screen dimensions
	uint8_t xPos = registers[Vx] % VIDEO_WIDTH;
	uint8_t yPos = registers[Vy] % VIDEO_HEIGHT;

	// Reset collision register
	registers[0xF] = 0;

	for (unsigned int row = 0; row < height; ++row)
	{
		// If the sprite goes off the bottom of the screen, stop drawing this sprite
		if (yPos + row >= VIDEO_HEIGHT) break;

		// Fetch the 8-bit row of the sprite from memory
		uint8_t spriteByte = memory[index + row];

		for (unsigned int col = 0; col < 8; ++col)
		{
			// If the sprite goes off the right edge, stop drawing this row
			if (xPos + col >= VIDEO_WIDTH) break;

			// Check if the specific bit (pixel) in the sprite byte is 'on'
			// (0x80 >> col) scans bits from left to right: 10000000, 01000000, etc.
			uint8_t spritePixel = spriteByte & (0x80u >> col);

			// Get a pointer to the current screen pixel
			uint32_t* screenPixel = &video[(yPos + row) * VIDEO_WIDTH + (xPos + col)];

			if (spritePixel)
			{
				// Chip-8 uses XOR drawing. 
				// If the screen pixel is already on (0xFFFFFFFF), we have a collision.
				if (*screenPixel == 0xFFFFFFFF)
				{
					registers[0xF] = 1;
				}

				// XOR the pixel: If it was on, it turns off (0x0). If off, it turns on (0xFFFFFFFF).
				*screenPixel ^= 0xFFFFFFFF;
			}
		}
	}
}//Skip next instruction if key with the value of Vx is pressed.
void Chip8::OP_Ex9E()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	uint8_t key = registers[Vx];

	if (keypad[key])
	{
		pc += 2;
	}
}
//skip next instruction if right key is NOT pressed
void Chip8::OP_ExA1()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	uint8_t key = registers[Vx];

	if (!keypad[key])
	{
		pc += 2;
	}
}
//set vx = delay timer
void Chip8::OP_Fx07()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	registers[Vx] = delayTimer;
}

//wait for key input to set register value as input key number
void Chip8::OP_Fx0A()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	if (keypad[0])
	{
		registers[Vx] = 0;
	}
	else if (keypad[1])
	{
		registers[Vx] = 1;
	}
	else if (keypad[2])
	{
		registers[Vx] = 2;
	}
	else if (keypad[3])
	{
		registers[Vx] = 3;
	}
	else if (keypad[4])
	{
		registers[Vx] = 4;
	}
	else if (keypad[5])
	{
		registers[Vx] = 5;
	}
	else if (keypad[6])
	{
		registers[Vx] = 6;
	}
	else if (keypad[7])
	{
		registers[Vx] = 7;
	}
	else if (keypad[8])
	{
		registers[Vx] = 8;
	}
	else if (keypad[9])
	{
		registers[Vx] = 9;
	}
	else if (keypad[10])
	{
		registers[Vx] = 10;
	}
	else if (keypad[11])
	{
		registers[Vx] = 11;
	}
	else if (keypad[12])
	{
		registers[Vx] = 12;
	}
	else if (keypad[13])
	{
		registers[Vx] = 13;
	}
	else if (keypad[14])
	{
		registers[Vx] = 14;
	}
	else if (keypad[15])
	{
		registers[Vx] = 15;
	}
	//method to repeat instruction till key is pressed
	else
	{
		pc -= 2;
	}
}
//set soundtimer = Vx
void Chip8::OP_Fx18()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	soundTimer = registers[Vx];
}
//set delay timer = vx
void Chip8::OP_Fx15()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	delayTimer = registers[Vx];
}
//set Index = index + Vx
void Chip8::OP_Fx1E()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	index += registers[Vx];
}
//set index to start address of particular sprite for digit
void Chip8::OP_Fx29()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t digit = registers[Vx];

	index = FONTSET_START_ADDRESS + (5 * digit);
}
//store decimal vals of three digit nos in mem address I I+1 and I+2
void Chip8::OP_Fx33()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t value = registers[Vx];

	// Ones-place
	memory[index + 2] = value % 10;
	value /= 10;

	// Tens-place
	memory[index + 1] = value % 10;
	value /= 10;

	// Hundreds-place
	memory[index] = value % 10;
}
//store v0 to vx in mem
void Chip8::OP_Fx55()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	for (uint8_t i = 0; i <= Vx; ++i)
	{
		memory[index + i] = registers[i];
	}
}
//read from memory values of registers from v0 to vx
void Chip8::OP_Fx65()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	for (uint8_t i = 0; i <= Vx; ++i)
	{
		registers[i] = memory[index + i];
	}
}

//function pointer table
void Chip8::Table0()
{
	(this->*table0[opcode & 0x000F])();
}

void Chip8::Table8()
{
	(this->*table8[opcode & 0x000F])();
}

void Chip8::TableE()
{
	(this->*tableE[opcode & 0x000F])();
}

void Chip8::TableF()
{
	(this->*tableF[opcode & 0x00FF])();
}

void Chip8::OP_NULL() {}


//fetch decode execute fn
void Chip8::Cycle()
{
	// Fetch
	opcode = (memory[pc] << 8u) | memory[pc + 1];

	// Increment the PC before we execute anything
	pc += 2;

	// Decode and Execute
	((*this).*(table[(opcode & 0xF000u) >> 12u]))();

	// Decrement the delay timer if it's been set
	if (delayTimer > 0)
	{
		--delayTimer;
	}

	// Decrement the sound timer if it's been set
	if (soundTimer > 0)
	{
		--soundTimer;
	}
}















