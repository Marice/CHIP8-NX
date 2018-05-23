#include <switch.h>
#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <stdbool.h>
#include <stdio.h>
#include "chip8.h"

int main(int argc, char *argv[])
{
	SDL_Init(SDL_INIT_EVERYTHING);

	//Setup window
	SDL_Window* window = SDL_CreateWindow(nullptr, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP);
	if (!window) { SDL_Quit(); }
	
	//Setup renderer
	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
	if (!renderer) { SDL_Quit(); }

	Chip8 chip8;
	if (!chip8.LoadRom("ufo.c8"))
	{
		return -1;
	}

	SDL_Event events;
	bool running = true;

	// Clear screen
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);

	while(appletMainLoop() && running == true)
	{


		//Scan all the inputs. This should be done once for each frame
        hidScanInput();
	
		u64 kDown = hidKeysDown(CONTROLLER_P1_AUTO);

		if (kDown & KEY_PLUS) {
			
			break;
		}  
		
		// Input translation
		// TODO: Put in table
		if (kDown & KEY_DDOWN ) // 1 -> 1
		{
			chip8.keys[0] = 1;
		}
		if (kDown & KEY_DUP )  // 2 -> 2
		{
			chip8.keys[1] = 1;
		}
		if (kDown & KEY_DLEFT )  // 3 -> 3
		{
			chip8.keys[2] = 1;
		}
		if (kDown & KEY_DRIGHT) // 4 -> C
		{
			chip8.keys[3] = 1;
		}
		if (kDown & KEY_A )  // q -> 4
		{
			chip8.keys[4] = 1;
		}
		if (kDown & KEY_B )  // w -> 5
		{
			chip8.keys[5] = 1;
		}
		if (kDown & KEY_X )// e -> 6
		{
			chip8.keys[6] = 1;
		}
		if (kDown & KEY_Y )// r -> D
		{
			chip8.keys[7] = 1;
		}
	    if (kDown & KEY_L ) // a -> 7
		{
			chip8.keys[8] = 1;
		}
		if (kDown & KEY_R ) // s -> 8
		{
			chip8.keys[9] = 1;
		}
		if (kDown & KEY_ZL )// d -> 9
		{
			chip8.keys[10] = 1;
		}
		if (kDown & KEY_ZR )// f -> E
		{
			chip8.keys[11] = 1;
		}
		if (kDown & KEY_LEFT )// z -> 4
		{
			chip8.keys[12] = 1;
		}
		if (kDown & KEY_RIGHT )// x -> 5
		{
			chip8.keys[13] = 1;
		}
		if (kDown & KEY_UP ) // c -> 6
		{
			chip8.keys[14] = 1;
		}
		if (kDown & KEY_DOWN )// -> F
		{
			chip8.keys[15] = 1;
		}

		chip8.EmulateCycle();

		// chip8 programs don't always render every cycle
		if (chip8.drawFlag)
		{
			// Clear screen
			SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
			SDL_RenderClear(renderer);

			// Draw screen
			SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
			SDL_Rect *destRect = new SDL_Rect;
			destRect->x = 0;
			destRect->y = 0;
			destRect->w = 8;
			destRect->h = 8;


			for (int y = 0; y < 32; y++)
			{
				for (int x = 0; x < 64; x++)
				{
					if (chip8.gfx[(y * 64) + x] == 1)
					{
						destRect->x = x * 8;
						destRect->y = y * 8;

						SDL_RenderFillRect(renderer, destRect);
					}
				}
			}

			delete destRect;

			SDL_RenderPresent(renderer);
			chip8.drawFlag = false;
			chip8.ResetKeys();
		}
	}

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);

	SDL_Quit();
	
	system("pause");

	return 0;
}