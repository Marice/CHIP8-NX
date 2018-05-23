// Author: Jeralde Kendall
// Email: dsage84@hotmail.com
// Code is a modified version of Laurence Muller's code. Use at your own risk.
//	located here: http://www.multigesture.net/articles/how-to-write-an-emulator-chip-8-interpreter/

#include "chip8.h"

// Chip 8 font set
unsigned char g_chip8FontSet[80] =
{
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

void Chip8::Initialize()
{
	// Zero arrays
	for (int i = 0; i < 16; i++)
	{
		keys[i] = 0;
		stack[i] = 0;
		registers[i] = 0;
	}

	for (int i = 0; i < 2048; i++)
	{
		gfx[i] = 0;
	}

	for (int i = 0; i < 4096; i++)
	{
		memory[i] = 0;
	}

	// Load font set in to memory
	for (int i = 0; i < 80; i++)
	{
		memory[i] = g_chip8FontSet[i];
	}

	programCounter = 0x200;	// Programs start at the 512 memory location
	opcode = 0;
	indexRegister = 0;
	stackPointer = 0;

	delayTimer = 0;
	audioTimer = 0;

	srand(int(time(NULL)));

	// Clear screen once upon starting
	drawFlag = true;
}

// Cycle at 60hz - 60 opcodes a second
void Chip8::EmulateCycle()
{
	// Get opcode
	// Example:
	//		memory[pc]	 == 0xA2
	//		memory[pc+1] == 0xF0
	//		memory[pc] << 8 | memory[pc+1]
	//		0xA2F0
	opcode = memory[programCounter] << 8 | memory[programCounter + 1];

	// Decode and execute opcode
	switch (opcode & 0xF000)
	{
		case 0x0000:
		{
			switch(opcode & 0x000F)
			{
				case 0x0000: // 0x00E0 - Clear screen
				{
					for (int i = 0; i < 2048; i++)
					{
						gfx[i] = 0;
					}

					drawFlag = true;
					programCounter += 2;
				} break;
				case 0x000E: // 0x00EE - Return from subroutine
				{
					--stackPointer; // Go back one to prevent overwrite
					programCounter = stack[stackPointer];
					programCounter += 2;

				} break;
				default:
				{
					printf("Error: Unknown opcode [0x000]: 0x%X\n", opcode);
				}
			}
		} break;
		case 0x1000: // 0x1NNN - Jumps to address NNN 
		{
			programCounter = opcode & 0x0FFF;
		} break;
		case 0x2000: // 0x2NNN - Calls subroutine at NNN 
		{
			stack[stackPointer] = programCounter; 	// Store current address in stack
			++stackPointer;							// Increment stack pointer
			programCounter =  opcode & 0x0FFF;		// Set program counter to address at NNN
		} break;
		case 0x3000: // 0x3XNN - Skips next instruction of VX equals NN
		{
			if(registers[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF))
			{
				programCounter += 4;
			}
			else
			{
				programCounter += 2;
			}
		} break;
		case 0x4000: // 0x4XNN - Skips next instruction of VX does not equal NN
		{
			if(registers[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF))
			{
				programCounter += 4;
			}
			else
			{
				programCounter += 2;
			}
		} break;
		case 0x5000: // 0x5XY0 - Skips next instruction if VX equals VY
		{
			if(registers[(opcode & 0x0F00) >> 8] == registers[(opcode & 0x00F0) >> 4])
			{
				programCounter += 4;
			}
			else
			{
				programCounter += 2;
			}
		} break;
		case 0x6000: // 0x6XNN - Sets VX to NN
		{
			registers[(opcode & 0x0F00) >> 8] = (opcode & 0x00FF);
			programCounter += 2;
		} break;
		case 0x7000: // 0x7XNN - Adds NN to VX
		{
			registers[(opcode & 0x0F00) >> 8] += (opcode & 0x00FF);
			programCounter += 2;
		} break;
		case 0x8000:
		{
			switch (opcode & 0x000F)
			{
				case 0x0000: // 0x8XY0 - Sets VX to VY
				{
					registers[(opcode & 0x0F00) >> 8] = registers[(opcode & 0x00F0) >> 4];
					programCounter += 2;
				} break;
				case 0x0001: // 0x8XY1 - Sets VX to VX or VY
				{
					registers[(opcode & 0x0F00) >> 8] |= registers[(opcode & 0x00F0) >> 4];
					programCounter += 2;
				} break;
				case 0x0002: // 0x8XY2 - Sets VX to VX and VY
				{
					registers[(opcode & 0x0F00) >> 8] &= registers[(opcode & 0x00F0) >> 4];
					programCounter += 2;
				} break;
				case 0x0003: // 0x8XY3 - Sets VX to VX xor VY
				{
					registers[(opcode & 0x0F00) >> 8] ^= registers[(opcode & 0x00F0) >> 4];
					programCounter += 2;
				} break;
				case 0x0004: // 0x8XY4 - Adds VY to VX. If VF is set 1 if there is a carry, 0 if not
				{
					if (registers[(opcode & 0x0F00) >> 8] > (0xFF - registers[(opcode & 0x00F0) >> 4]))
					{
						registers[0xF] = 1; // Carry
					}
					else
					{
						registers[0xF] = 0; // No carry
					}

					registers[(opcode & 0x0F00) >> 8] += registers[(opcode & 0x00F0) >> 4];
					programCounter += 2;
				} break;
				case 0x0005: // 0x8XY5 - Subtracts VY from VX. If VF is set 0 if there is a borrow, 1 if not
				{
					if(registers[(opcode & 0x00F0) >> 4] < registers[(opcode & 0x0F00) >> 8])
					{
						registers[0xF] = 0; // Borrow
					}
					else
					{
						registers[0xF] = 1; // No borrow
					}

					registers[(opcode & 0x0F00) >> 8] -= registers[(opcode & 0x00F0) >> 4];
					programCounter += 2;
				} break;
				case 0x0006: // 0x8XY6 - Shifts VX right by one. VF is set to the value of the least significant bit of VX before the shift
				{
					registers[0xF] = registers[(opcode & 0x0F00) >> 8] & 0x1;
					registers[(opcode & 0x0F00) >> 8] >>= 1;
					programCounter += 2;
				} break;
				case 0x0007: // 0x8XY7 - Sets VX to VY minus VX. VF is set to 0 when there's a borrow, and 1 when there isn't
				{
					if(registers[(opcode & 0x0F00) >> 8] > registers[(opcode & 0x00F0) >> 4])
					{
						registers[0xF] = 0; // Borrow
					}
					else
					{
						registers[0xF] = 1; // No borrow
					}

					registers[(opcode & 0x0F00) >> 8] = registers[(opcode & 0x00F0) >> 4] - registers[(opcode & 0x0F00) >> 8];
					programCounter += 2;
				} break;
				case 0x000E: // 0x8XYE - Shifts VX left by one. VF is set to the value of the most significant bit of VX before the shift
				{
					registers[0xF] = registers[(opcode & 0x0F00) >> 8] >> 7;
					registers[(opcode & 0x0F00) >> 8] <<= 1;
					programCounter += 2;
				} break;
				default:
				{
					printf("Error: Unknown opcode [0x8000]: 0x%X\n", opcode);
				}
			}
		} break;
		case 0x9000: // 0x9XY0 - Skips to next instruction if X does not equal Y
		{
			if(registers[(opcode & 0x0F00) >> 8] != registers[(opcode & 0x00F0) >> 4])
			{
				programCounter += 4;
			}
			else
			{
				programCounter += 2;
			}
		} break;
		case 0xA000: // 0xANNN - Sets indexRegister to address at NNN
		{
			indexRegister = opcode & 0x0FFF;
			programCounter += 2;
		} break;
		case 0xB000: // 0xBNNN - Jumps to the address NNN plus V0
		{
			programCounter = (opcode & 0x0FFF) + registers[0x0];
		} break;
		case 0xC000: // 0xCXNN - Sets VX to a random number and NN
		{
			registers[(opcode & 0x0F00) >> 8] = (rand() % 0xFF) & (opcode & 0x00FF);
			programCounter += 2;
		} break;
		case 0xD000: // 0xDXYN - Sprites stored in memory at location in index register (I), maximum 8bits wide. 
					 //			 Wraps around the screen. 
					 //			 If when drawn, clears a pixel, register VF is set to 1 otherwise it is zero. 
					 // 		 All drawing is XOR drawing (i.e. it toggles the screen pixels)
		{
			unsigned short x = registers[(opcode & 0x0F00) >> 8];
			unsigned short y = registers[(opcode & 0x00F0) >> 4];
			unsigned short h = opcode & 0x000F;
			unsigned short pixel;
			registers[0xF] = 0;

			for(int yLine = 0; yLine < h; yLine++)
			{
				pixel = memory[indexRegister + yLine];

				for(int xLine = 0; xLine < 8; xLine++)
				{
					if((pixel & (0x80 >> xLine)) != 0)
					{
						if(gfx[(x + xLine + ((y + yLine) * 64))] == 1)
						{
							registers[0xF] = 1;
						}

						gfx[(x + xLine + ((y + yLine) * 64))] ^= 1;
					}
				}
			}

			drawFlag = true;
			programCounter += 2;
		} break;
		case 0xE000:
		{
			switch(opcode & 0x00FF)
			{
				case 0x009E: // 0xEX9E - Skips the next instruction if the key stored in VX is pressed
				{
					if(keys[registers[(opcode & 0x0F00) >> 8]] != 0)
					{
						programCounter += 4;
					}
					else
					{
						programCounter += 2;
					}
				} break;
				case 0x00A1: // 0xEXA1 - Skips the next instruction if the key stored in VX isn't pressed
				{
					if(keys[registers[(opcode & 0x0F00) >> 8]] == 0)
					{
						programCounter += 4;
					}
					else
					{
						programCounter += 2;
					}
				} break;
				default:
				{
					printf("Error: Unknown opcode [0xE000]: 0x%X\n", opcode);
				}
			}
		} break;
		case 0xF000:
		{
			switch(opcode & 0x00FF)
			{
				case 0x0007: // 0xFX07 - Sets VX to the value of the delay timer
				{
					registers[(opcode & 0x0F00) >> 8] = delayTimer;
					programCounter += 2;
				} break;
				case 0x000A: // 0xFX0A - A key press is awaited, and then stored in VX
				{
					bool keyPressed = false;
					for(int i = 0; i < 16; i++)
					{
						if(keys[i] == 1)
						{
							registers[(opcode & 0x0F00) >> 8] = i;
							keyPressed = true;
						}
					}

					if(!keyPressed)
					{
						return;
					}

					programCounter += 2;
				} break;
				case 0x0015: // 0xFX15 - Sets the delay timer to VX
				{
					delayTimer = registers[(opcode & 0x0F00) >> 8];
					programCounter += 2;
				} break;
				case 0x0018: // 0xFX18 - Sets the sound timer to VX
				{
					audioTimer = registers[(opcode & 0x0F00) >> 8];
					programCounter += 2;
				} break;
				case 0x001E: // 0xFX1E - Adds VX to I
				{
					if(indexRegister + registers[(opcode & 0x0F00) >> 8] > 0xFF)
					{
						registers[0xF] = 1;
					}
					else
					{
						registers[0xF] = 0;
					}

					indexRegister += registers[(opcode & 0x0F00) >> 8];
					programCounter += 2;
				} break;
				case 0x0029: // 0xFX29 - Sets I to the location of the sprite for the character in VX. 
							 // 		 Characters 0-F (in hexadecimal) are represented by a 4x5 font.
				{
					indexRegister = registers[(opcode & 0x0F00) >> 8] * 0x5;
					programCounter += 2;
				} break;
				case 0x0033: // 0xFX33 - Stores the Binary-coded decimal representation of VX, with the most 
							 // 		 significant of three digits at the address in I, the middle digit at 
							 // 		 I plus 1, and the least significant digit at I plus 2. (In other words, 
							 //			 take the decimal representation of VX, place the hundreds digit in memory 
							 //			 at location in I, the tens digit at location I+1, and the ones digit at location I+2.)
				{
					memory[indexRegister] = registers[(opcode & 0x0F00) >> 8] / 100;
					memory[indexRegister] = (registers[(opcode & 0x0F00) >> 8] / 10) % 10;
					memory[indexRegister] = (registers[(opcode & 0x0F00) >> 8] % 100) % 10;

					programCounter += 2;
				} break;
				case 0x0055: // 0xFX55 - Stores V0 to VX in memory starting at address I
				{
					for(int i = 0; i <= (opcode & 0x0F00) >> 8; ++i)
					{
						memory[indexRegister + i] = registers[i];
					}

					// On the original interpreter, when the operation is done, I=I+X+1.
					// On current implementations, I is left unchanged.
					indexRegister = indexRegister + ((opcode & 0x0F00) >> 8) + 1;
					programCounter += 2;
				} break;
				case 0x0065: // 0xFX65 - Fills V0 to VX with values from memory starting at address I
				{
					for(int i = 0; i <= (opcode & 0x0F00) >> 8; ++i)
					{
						registers[i] = memory[indexRegister + i];
					}

					// On the original interpreter, when the operation is done, I=I+X+1.
					// On current implementations, I is left unchanged.
					indexRegister = indexRegister + ((opcode & 0x0F00) >> 8) + 1;
					programCounter += 2;
				} break;
				default:
				{
					printf("Error: Unknown opcode [0xF000]: 0x%X\n", opcode);
				}
			}
		} break;
		default:
		{
			printf("Error: Unknown opcode: 0x%X\n", opcode);
		}
	}

	// Update timers
	if (delayTimer > 0)
	{
		--delayTimer;
	}

	if (audioTimer > 0)
	{
		if (audioTimer == 1)
		{
			// TODO: add option for '\a' system beep?
			std::cout << "Beep!\n";
			--audioTimer;
		}
	}
}

bool Chip8::LoadRom(const std::string &filePath)
{
	// Reset emulator
	Initialize();

	// Load file to memory
	std::cout << "Loading: " << filePath << "\n";

	std::ifstream rom;
	rom.open(filePath.c_str(), std::ios::binary | std::ios::ate);
	
	if (!rom.is_open())
	{
		std::cout << "Could not load file.\n";
		return false;
	}
	
	// Get file size
	int size = rom.tellg();
	std::cout << "Size: " << size << " bytes \n";

	// Check to see if too large for memory (512 reserved for interpreter)
	if ((4096 - 512) < size)
	{
		std::cout << "Error: Rom too large\n";
		return false;
	}

	// Seek to beginning of file
	rom.seekg(rom.beg);

	// Create and read data to buffer
	char * buffer = new char[size];
	rom.read(buffer, size);

	// Load to chip8 memory
	for (int i = 0; i < size; i++)
	{
		memory[512 + i] = buffer[i];
	}

	delete[] buffer;

	// Finally, close the file
	rom.close();

	// File successfully loaded
	return true;
}

void Chip8::ResetKeys()
{
	for (int i = 0; i < 16; i++)
	{
		keys[i] = 0;
	}
}
