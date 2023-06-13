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
#define MAX_PROJ 9999

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
	IMG_SUN,
	IMG_PEA
};

enum plantIdList {
	ID_PEAHOOTER = 0
};

enum projIdList {
	PROJ_PEA = 0
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
	int x, y, type, width, height;
	bool isPlanted;
	int row, column;
	int fireRate, fireTimer;
} plants[MAX_PLANTS];

struct zombieStr {
	SDL_Texture* plantImg;
	bool isSpawned;
	float x;
	int y, type, row, column, height, width, health;
} zombies[MAX_ZOMBIES];

struct sunStr {
	float x, y;
	bool isSpawned;
	int width, height;
} sun[MAX_SUN];

struct projStr {
	float x, y;
	bool isSpawned;
	int type, height, width, row;
} proj[MAX_PROJ];

time_t t;
int randomNumber(int range) {
	int rng = rand() % range;
	return rng;
}

void text(char string[], int x, int y, float textSize) {
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

bool checkBounds(float inputChecking_x, float inputChecking_y, float inputCheckAgainst_x, float inputCheckAgainst_y, float size) {
	if (inputChecking_x > inputCheckAgainst_x && inputChecking_x < inputCheckAgainst_x + size && inputChecking_y > inputCheckAgainst_y && inputChecking_y < inputCheckAgainst_y + size) {
		return true;
	} else {
		return false;
	}
}

void plantSeed(int row, int column, int type) {
	for (int i = 0; i < MAX_PLANTS; ++i) {
		if (plants[i].isPlanted == false) {
			plants[i].x = column * 60 + 200;
			plants[i].y = row * 82 + 80;
			plants[i].type = type;
			plants[i].isPlanted = true;
			plants[i].row = row;
			plants[i].column = column;
			plants[i].fireTimer = 0;
			
			switch (type) {
				case ID_PEAHOOTER:
					plants[i].fireRate = 200;
					plants[i].width = 50;
					plants[i].height = 50;
				break;
			}
			
			break;
		}
	}
}

void spawnZombie(int row, int type) {
	for (int i = 0; i < MAX_ZOMBIES; ++i) {
		if (zombies[i].isSpawned == false) {
			zombies[i].x = 800;
			zombies[i].y = row * 82 + 80;
			zombies[i].type = type;
			zombies[i].isSpawned = true;
			zombies[i].row = row;
			zombies[i].width = 50;
			zombies[i].height = 50;
			zombies[i].health = 100;
			
			break;
		}
	}
}

void spawnSun() {
	for (int i = 0; i < MAX_SUN; ++i) {
		if (sun[i].isSpawned == false) {
			sun[i].x = randomNumber(750);
			sun[i].y = -50;
			sun[i].width = 50;
			sun[i].height = 50;
			sun[i].isSpawned = true;
			
			break;
		}
	}
}

void spawnProjectile(int x, int y, int row, int type) {
	for (int i = 0; i < MAX_PROJ; ++i) {
		if (proj[i].isSpawned == false) {
			proj[i].x = x;
			proj[i].y = y;
			proj[i].width = 20;
			proj[i].height = 20;
			proj[i].isSpawned = true;
			proj[i].row = row;
			
			break;
		}
	}
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
	
	image_sur = IMG_Load("romfs/pea_projectile.png");
	image[IMG_PEA] = SDL_CreateTextureFromSurface(rend, image_sur);
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
	for (int i = 0; i < MAX_PROJ; ++i) {
		proj[i].isSpawned = false;
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
		if (zombies[i].isSpawned) {
			zombies[i].x -= 0.2;
			if (zombies[i].health <= 0) {
				zombies[i].isSpawned = false;
			}
		} else {
			continue;
		}
	}
	
	//update sun
	for (int i = 0; i < MAX_SUN; ++i) {
		if (sun[i].isSpawned) {
			sun[i].y += 0.5;
			if (checkBounds(coord_click_x, coord_click_y, sun[i].x, sun[i].y, SUN_SIZE)) {
				sun[i].isSpawned = false;
				sunCount += 25;
				Mix_PlayChannel(1, sfx[SFX_SUN_COLLECT], 0);
			}
		} else {
			continue;
		}
	}
	
	//update plants
	for (int i = 0; i < MAX_PLANTS; ++i) {
		if (plants[i].isPlanted) {
			for (int j = 0; j < MAX_ZOMBIES; ++j) {
				if (zombies[j].isSpawned == true && zombies[j].row == plants[i].row) {
					if (plants[i].fireTimer >= plants[i].fireRate) {
						spawnProjectile(plants[i].x, plants[i].y, plants[i].row, PROJ_PEA);
					}
				} else {
					continue;
				}
			}
			if (plants[i].fireTimer >= plants[i].fireRate) {
				plants[i].fireTimer = 0;
			}
			
			plants[i].fireTimer += 1;
		} else {
			continue;
		}
	}
	
	//update projectiles
	for (int i = 0; i < MAX_PROJ; ++i) {
		if (proj[i].isSpawned) {
			proj[i].x += 5;
			for (int j = 0; j < MAX_ZOMBIES; ++j) {
				if (zombies[j].row == proj[i].row && proj[i].x > zombies[j].x && proj[i].x < zombies[j].x + zombies[j].width && zombies[j].isSpawned == true) {
					printf("hit");
					proj[i].isSpawned = false;
					zombies[j].health -= 10;
				}
			}
			
			if (proj[i].x > 800) {
				proj[i].isSpawned = false;
				printf("stray pea removed");
			}
		}
	}
}

void renderEntities() {
	//render plants
	for (int i = 0; i < MAX_PLANTS; ++i) {
		if (plants[i].isPlanted != false) {
			SDL_RenderCopy(rend, image[IMG_PEASHOOTER], NULL, &(SDL_Rect){plants[i].x, plants[i].y, plants[i].width, plants[i].height});
		} else {
			continue;
		}
	}
	
	//render zombies
	for (int i = 0; i < MAX_ZOMBIES; ++i) {
		if (zombies[i].isSpawned != false) {
			SDL_RenderCopy(rend, image[IMG_ZOMBIE_NORMAL], NULL, &(SDL_Rect){zombies[i].x, zombies[i].y, zombies[i].width, zombies[i].height});
		} else {
			continue;
		}
	}
	
	//render sun
	for (int i = 0; i < MAX_SUN; ++i) {
		if (sun[i].isSpawned != false) {
			SDL_RenderCopy(rend, image[IMG_SUN], NULL, &(SDL_Rect){sun[i].x, sun[i].y, SUN_SIZE, SUN_SIZE});
		} else {
			continue;
		}
	}
	
	//render projectiles
	for (int i = 0; i < MAX_PROJ; ++i) {
		if (proj[i].isSpawned != false) {
			SDL_RenderCopy(rend, image[IMG_PEA], NULL, &(SDL_Rect){proj[i].x, proj[i].y, proj[i].width, proj[i].height});
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
		while ((int)(SDL_GetTicks()) % 20 != 0) {
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
							plantSeed((int)row_click, (int)column_click, ID_PEAHOOTER);
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
		
		text(sunCounter, 0, 0, 50);
		
		if (gameFrame % 500 == 0) {
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