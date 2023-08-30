#ifndef HEADER_H_INCLUDE
#define HEADER_H_INCLUDE

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <math.h>
#include <structs.h>
#include <time.h>

#define MAX_STRING 260

#define MAP_WIDTH 28
#define MAP_HEIGHT 32
#define WALL_SIZE 8
#define TILE_SIZE 20
#define ENTITIES_SIZE 30
#define TEXT_NORMAL_SIZE 16
#define TEXT_LOWER_SIZE 13
#define SPACING 5

#define GHOST_VELOCITY 95.0f
#define GHOST_LOWER_VELOCITY 45.0f
#define GHOST_FAST_VELOCITY 300.0f
#define GHOST_FRAME_SPEED 5.0f

#define PACMAN_VELOCITY 110.0f
#define PACMAN_FRAME_SPEED 10.0f
#define PACMAN_DEATH_SPEED 3.5f;

#define MIN_DISTANCI TILE_SIZE*6
#define VUNERAVEL_TIME 8000
#define BLINK_TIME 200
#define GAME_OVER_TIME 3000
#define WIN_TIME 4000

#endif