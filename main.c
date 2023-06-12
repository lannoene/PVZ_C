#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 500
#define MAX_TEXTURES 500
#define MAX_PLANTS 1000
#define MAX_ZOMBIES 500
#define MAX_SUN 1000
#define MOUSE_OUT_OF_BOUNDS -1
#define SUN_SIZE 50
#define TEXT_LOAD_SIZE 100

/*
* Audio channel legend:
* Music = Music channel
* Player sfx = Sfx channel 1
* Zombie sfx = Sfx channel 2
* Plant sfx = Sfx channel 3
*/
static SDL_Window *window = NULL;
static SDL_Surface *screen = NULL;
static SDL_Renderer *rend = NULL;
int plantCount = 0;
int zombieCount = 0;
int coord_click_x = MOUSE_OUT_OF_BOUNDS;
int coord_click_y = MOUSE_OUT_OF_BOUNDS;
int gameFrame = 0;
int sunCount = 0;
int sunSpawnId = 0;

//audio
Mix_Music* music[10] = {0};
Mix_Chunk* sfx[100] = {0};
SDL_Texture* image[MAX_TEXTURES] = {0};

//display text
TTF_Font* font;
SDL_Surface* textSurf;
SDL_Texture* textTexture;
SDL_Rect textRect;

enum imageIdList {
	BG_GRASS = 0,
	IMG_PEASHOOTER,
	IMG_ZOMBIE_NORMAL,
	IMG_SUN
};

enum plantIdList {
	peaShooter = 0
};

enum musicIdList {
	BGM_DAY = 0,

};

enum sfxIdList {
	SFX_SEED_PLANT = 0,
	SFX_SUN_COLLECT
};

struct plantStr {
	SDL_Texture* plantImg;
	int x, y, type;
	bool isPlanted;
} plants[MAX_PLANTS];

struct zombieStr {
	SDL_Texture* plantImg;
	bool isSpawned;
	float x;
	int y, type;
} zombies[MAX_ZOMBIES];

struct sunStr {
	float x, y;
	bool isSpawned;
	int width, height;
} sun[MAX_SUN];

time_t t;
int randomNumber(int range) {
	int rng = rand() % range;
	return rng;
}

void text(char string[], float textSize, int x, int y) {
	textSurf = TTF_RenderText_Solid(font, string, (SDL_Color){0,0,0});
	
	float textMultiple = textSurf->h/textSize;
	
	int textWidth = textSurf->w/textMultiple;
	int textHeight = textSize;
	
	textRect = (SDL_Rect){x, y, textWidth, textHeight};
	textTexture = SDL_CreateTextureFromSurface(rend, textSurf);
	SDL_FreeSurface(textSurf);
	
	SDL_RenderCopy(rend, textTexture, NULL, &textRect);
	SDL_DestroyTexture(textTexture);
}

void plantSeed(int row, int column, int type) {
	plants[plantCount].x = column * 60 + 200;
	plants[plantCount].y = row * 82 + 80;
	plants[plantCount].type = type;
	plants[plantCount].isPlanted = true;
	
	++plantCount;
}

void spawnZombie(int row, int type) {
	zombies[zombieCount].x = 800;
	zombies[zombieCount].y = row * 82 + 80;
	zombies[zombieCount].type = type;
	zombies[zombieCount].isSpawned = true;
	
	++zombieCount;
}

void spawnSun() {
	sun[sunSpawnId].x = randomNumber(750);
	sun[sunSpawnId].y = -50;
	sun[sunSpawnId].width = 50;
	sun[sunSpawnId].height = 50;
	sun[sunSpawnId].isSpawned = true;
	
	++sunSpawnId;
}

void initSprites() {
	SDL_Surface* image_sur;
	image_sur = IMG_Load("romfs/background_grass.png");
	image[BG_GRASS] = SDL_CreateTextureFromSurface(rend, image_sur);
	SDL_FreeSurface(image_sur);
	
	image_sur = IMG_Load("romfs/peashooter.png");
	image[IMG_PEASHOOTER] = SDL_CreateTextureFromSurface(rend, image_sur);
	SDL_FreeSurface(image_sur);
	
	image_sur = IMG_Load("romfs/PVZ_Zombie_Suit.png");
	image[IMG_ZOMBIE_NORMAL] = SDL_CreateTextureFromSurface(rend, image_sur);
	SDL_FreeSurface(image_sur);
	
	image_sur = IMG_Load("romfs/sun.png");
	image[IMG_SUN] = SDL_CreateTextureFromSurface(rend, image_sur);
	SDL_FreeSurface(image_sur);
}

void initEntities() {
	for (int i = 0; i < MAX_PLANTS; ++i) {
		plants[i].isPlanted = false;
	}
	for (int i = 0; i < MAX_ZOMBIES; ++i) {
		zombies[i].isSpawned = false;
	}
	for (int i = 0; i < MAX_SUN; ++i) {
		sun[i].isSpawned = false;
	}
}

void clearMouseInputs() {
	coord_click_x = MOUSE_OUT_OF_BOUNDS;
	coord_click_y = MOUSE_OUT_OF_BOUNDS;
}

void audioInit() {
	Mix_Init(MIX_INIT_MP3 | MIX_INIT_OPUS | MIX_INIT_OGG);
	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 6, 4096);
	
	music[BGM_DAY] = Mix_LoadMUS("romfs/day.ogg");
	sfx[SFX_SEED_PLANT] = Mix_LoadWAV("romfs/plant_action_plant.wav");
	sfx[SFX_SUN_COLLECT] = Mix_LoadWAV("romfs/sun_pick.wav");
}

void updateEntities() {
	//update zombies
	for (int i = 0; i < MAX_ZOMBIES; ++i) {
		if (zombies[i].isSpawned != false) {
			zombies[i].x -= 0.2;
		} else {
			continue;
		}
	}
	
	//update sun
	for (int i = 0; i < MAX_ZOMBIES; ++i) {
		if (sun[i].isSpawned != false) {
			sun[i].y += 0.5;
			if (coord_click_x > sun[i].x && coord_click_x < sun[i].x + SUN_SIZE && coord_click_y > sun[i].y && coord_click_y < sun[i].y + SUN_SIZE) {
				sun[i].isSpawned = false;
				sunCount += 25;
				Mix_PlayChannel(1, sfx[SFX_SUN_COLLECT], 0);
			}
		} else {
			continue;
		}
	}
}

void renderEntities() {
	//render plants
	for (int i = 0; i < MAX_PLANTS; ++i) {
		if (plants[i].isPlanted != false) {
			SDL_RenderCopy(rend, image[IMG_PEASHOOTER], NULL, &(SDL_Rect){plants[i].x, plants[i].y, 50, 50});
		} else {
			continue;
		}
	}
	
	//render zombies
	for (int i = 0; i < MAX_ZOMBIES; ++i) {
		if (zombies[i].isSpawned != false) {
			SDL_RenderCopy(rend, image[IMG_ZOMBIE_NORMAL], NULL, &(SDL_Rect){zombies[i].x, zombies[i].y, 50, 50});
		} else {
			continue;
		}
	}
	
	//render sun
	for (int i = 0; i < MAX_SUN; ++i) {
		if (sun[i].isSpawned != false) {
			SDL_RenderCopy(rend, image[IMG_SUN], NULL, &(SDL_Rect){sun[i].x, sun[i].y, 50, 50});
		} else {
			continue;
		}
	}
}


int main(int argc, char *argv[]) {
	SDL_Init(SDL_INIT_EVERYTHING);
	window = SDL_CreateWindow("Plants Vs. Zombies", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
	screen = SDL_GetWindowSurface(window);
	rend = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	//setting rng
	srand(time(NULL));

	initSprites();
	initEntities();
	audioInit();
	TTF_Init();
	font = TTF_OpenFont("romfs/Arial.ttf", TEXT_LOAD_SIZE);
	
	//start playing bg music
	Mix_PlayMusic(music[BGM_DAY], -1);
	
	bool run = true;
	while (run) {
		while (SDL_GetTicks() % 20 != 0) {
			//hold
		}
		
		coord_click_x = MOUSE_OUT_OF_BOUNDS;//will be set to inbounds if user clicks
		coord_click_y = MOUSE_OUT_OF_BOUNDS;
		
		SDL_SetRenderDrawColor(rend, 0xff, 0xff, 0xff, SDL_ALPHA_OPAQUE);
		SDL_RenderClear(rend);

		//bg image
		SDL_RenderCopy(rend, image[BG_GRASS], NULL, &(SDL_Rect){-50, 0, 1100, 500});
		
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			switch(event.type) {
				case SDL_QUIT:
				run = false;
				case SDL_KEYDOWN:
					switch (event.key.keysym.scancode) {
						case SDL_SCANCODE_ESCAPE:
						 run = false;
						break;
						default:
						break;
					}
				break;
				case SDL_MOUSEBUTTONDOWN://handle mouse input
					if (SDL_BUTTON_LEFT == event.button.button) {
						coord_click_x = event.button.x;
						coord_click_y = event.button.y;
						
						
						float row_click = (float)(coord_click_y - 80)/82;//ints can't store values past the ones place, and as such, those values are cut off, automatically flooring this number.
						float column_click = (float)(coord_click_x - 200)/60;
						printf("row: %f column: %f %d\n", row_click, column_click, (coord_click_x - 200)/60);
						if (row_click >= 0 && row_click < 5 && column_click >= 0 && column_click < 9) {
							Mix_PlayChannel(1, sfx[SFX_SEED_PLANT], 0);
							plantSeed((int)row_click, (int)column_click, peaShooter);
						}
					}
				break;
				case SDL_WINDOWEVENT:
					if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
						int width = event.window.data1;
                        int height = event.window.data2;
						
						SDL_SetWindowSize(window, WINDOW_WIDTH, WINDOW_HEIGHT);

						printf("resized width: %d height %d\n", width, height);
					}
				break;
			break;
			}
		}
		
		SDL_SetRenderDrawColor(rend, 0x00, 0x00, 0x00, SDL_ALPHA_OPAQUE);
		
		updateEntities();
		renderEntities();
		
		char sunCounter[40];
		snprintf(sunCounter, sizeof(sunCounter), "Sun: %d", sunCount);
		
		text(sunCounter, 50, 0, 0);
		
		if (gameFrame % 100 == 0) {
			spawnZombie(randomNumber(5), 0);
			spawnSun();
		}
		
		SDL_RenderPresent(rend);
		SDL_GL_SwapWindow(window);
		
		gameFrame++;
	}
	for (int i = 0; i < MAX_TEXTURES; ++i) {
		if (image[i] != NULL) {
			SDL_DestroyTexture(image[i]);
		} else {
			break;
		}
	}
	SDL_DestroyWindow(window);
	TTF_CloseFont(font);
	Mix_Quit();
	SDL_Quit();
	TTF_Quit();
	
	return 0;
}