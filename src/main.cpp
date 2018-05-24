#include <iostream>
#include <cstdint>
#include <string>
#include <fstream>
#include <stdbool.h>
#include <stdio.h>
#include <dirent.h>
#include <switch.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include "chip8.h"

int getInd(char* curFile, int curIndex) {
    DIR* dir;
    struct dirent* ent;

    if(curIndex < 0)
        curIndex = 0;
    
    dir = opendir("/switch/roms/chip8");//Open current-working-directory.
    if(dir==NULL)
    {
        sprintf(curFile, "Failed to open dir!");
        return curIndex;
    }
    else
    {
        int i;
        for(i = 0; i <= curIndex; i++) {
            ent = readdir(dir);
        }
        if(ent)
            sprintf(curFile ,"/switch/roms/chip8/%s", ent->d_name);
        else
            curIndex--;
        closedir(dir);
    }

    return curIndex;
}

void getFile(char* curFile)
{
    gfxInitDefault();
    consoleInit(NULL);

    printf("\x1b[16;10HSelect a file using the up and down keys.");
    printf("\x1b[17;10HPress start to run the rom.");

    sprintf(curFile, "Couldn't find any files in that folder!");
    int curIndex = 0;
    curIndex = getInd(curFile, curIndex);
    printf("\x1b[18;10H%s", curFile);
	
    while(appletMainLoop())
    {
        //Scan all the inputs. This should be done once for each frame
        hidScanInput();

        //hidKeysDown returns information about which buttons have been just pressed (and they weren't in the previous frame)
        u64 kDown = hidKeysDown(CONTROLLER_P1_AUTO);

        if (kDown & KEY_DOWN || kDown & KEY_DDOWN) {
            consoleClear();
            printf("\x1b[16;10HSelect a file using the up and down keys.");
            printf("\x1b[17;10HPress start to run the rom.");
            curIndex++;
            curIndex = getInd(curFile, curIndex);
            printf("\x1b[18;10H%s", curFile);
        }

        if (kDown & KEY_UP || kDown & KEY_DUP) {
            consoleClear();
            printf("\x1b[16;10HSelect a file using the up and down keys.");
            printf("\x1b[17;10HPress start to run the rom.");
            curIndex--;
            curIndex = getInd(curFile, curIndex);
            printf("\x1b[18;10H%s", curFile);
        }


        if (kDown & KEY_PLUS || kDown & KEY_A) {
            break;
        }  
        gfxFlushBuffers();
        gfxSwapBuffers();
        gfxWaitForVsync();
    }
    
    consoleClear();
    gfxExit();
}

int main(int argc, char *argv[])
{
	char filename[255];
	
	getFile(filename);
    std::string romfile = filename;
	
	SDL_Init(SDL_INIT_EVERYTHING);

	//Setup window
	SDL_Window* window = SDL_CreateWindow(nullptr, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP);
	if (!window) { SDL_Quit(); }
	
	//Setup renderer
	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
	if (!renderer) { SDL_Quit(); }

	Chip8 chip8;
	if (!chip8.LoadRom(filename))
	{
		return -1;
	}

	// Clear screen
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);

	while(appletMainLoop())
	{

		//Scan all the inputs. This should be done once for each frame
        hidScanInput();
	
		u64 kDown = hidKeysDown(CONTROLLER_P1_AUTO);
		
		u64 kHeld = hidKeysHeld(CONTROLLER_P1_AUTO);

		if (kHeld & KEY_PLUS) {
			
			break;
		}  
		
		// Input translation
		// TODO: Put in table
		if (kDown & KEY_DDOWN || kHeld & KEY_DDOWN) // 1 -> 1
		{
			chip8.keys[0] = 1;
		}
		if (kDown & KEY_DUP || kHeld & KEY_DUP )  // 2 -> 2
		{
			chip8.keys[1] = 1;
		}
		if (kDown & KEY_DLEFT || kHeld & KEY_DLEFT)  // 3 -> 3
		{
			chip8.keys[2] = 1;
		}
		if (kDown & KEY_DRIGHT || kHeld & KEY_DRIGHT) // 4 -> C
		{
			chip8.keys[3] = 1;
		}
		if (kDown & KEY_A || kHeld & KEY_A)  // q -> 4
		{
			chip8.keys[4] = 1;
		}
		if (kDown & KEY_B  || kHeld & KEY_B)  // w -> 5
		{
			chip8.keys[5] = 1;
		}
		if (kDown & KEY_X || kHeld & KEY_X)// e -> 6
		{
			chip8.keys[6] = 1;
		}
		if (kDown & KEY_Y || kHeld & KEY_Y)// r -> D
		{
			chip8.keys[7] = 1;
		}
	    if (kDown & KEY_L  || kHeld & KEY_L) // a -> 7
		{
			chip8.keys[8] = 1;
		}
		if (kDown & KEY_R  || kHeld & KEY_R) // s -> 8
		{
			chip8.keys[9] = 1;
		}
		if (kDown & KEY_ZL  || kHeld & KEY_ZL)// d -> 9
		{
			chip8.keys[10] = 1;
		}
		if (kDown & KEY_ZR  || kHeld & KEY_ZR)// f -> E
		{
			chip8.keys[11] = 1;
		}
		if (kDown & KEY_LEFT || kHeld & KEY_LEFT )// z -> 4
		{
			chip8.keys[12] = 1;
		}
		if (kDown & KEY_RIGHT  || kHeld & KEY_RIGHT)// x -> 5
		{
			chip8.keys[13] = 1;
		}
		if (kDown & KEY_UP  || kHeld & KEY_UP) // c -> 6
		{
			chip8.keys[14] = 1;
		}
		if (kDown & KEY_DOWN  || kHeld & KEY_DOWN)// -> F
		{
			chip8.keys[15] = 1;
		}

		chip8.EmulateCycle();

		// chip8 programs don't always render every cycle
		if (chip8.drawFlag)
		{
			// Clear screen
			SDL_SetRenderDrawColor(renderer, 40, 40, 60, 255);
			SDL_RenderClear(renderer);

			// Draw screen
			SDL_SetRenderDrawColor(renderer, 255, 255, 200, 255); //yellow color for rendered blocks
			SDL_Rect *destRect = new SDL_Rect;
			destRect->x = 122;
			destRect->y = 84;
			destRect->w = 16;
			destRect->h = 16;


			for (int y = 0; y < 32; y++)
			{
				for (int x = 0; x < 64; x++)
				{
					if (chip8.gfx[(y * 64) + x] == 1)
					{
						destRect->x = 122 + (x * 16);
						destRect->y = 84 + (y * 16);

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

	return 0;
}