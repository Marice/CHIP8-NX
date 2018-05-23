// Author: Jeralde Kendall
// Email: dsage84@hotmail.com
// Code is a modified version of Laurence Muller's code. Use at your own risk.
//	located here: http://www.multigesture.net/articles/how-to-write-an-emulator-chip-8-interpreter/

#pragma once

// Chip 8 Emulator
#include <iostream>
#include <fstream>
#include <string>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

class Chip8
{
	public:
		Chip8(void) { Initialize(); }
	
		void EmulateCycle(void);
		bool LoadRom(const std::string &filePath);
		void ResetKeys();

		uint8_t gfx[2048];	// 64x32 pixels
		uint8_t keys[16];	// Hex keypad

		bool drawFlag;

	private:
		void Initialize(void);

		uint16_t programCounter;
		uint16_t opcode;
		uint16_t indexRegister;
		uint16_t stackPointer;

		uint8_t registers[16];	// V0-VF, VF is a carry flag			
		uint16_t stack[16];
		uint8_t memory[4096];

		uint8_t delayTimer;
		uint8_t audioTimer;
};
