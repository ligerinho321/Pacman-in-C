#ifndef PACMAN_H_INCLUDE
#define PACMAN_H_INCLUDE

#include <utils.h>

Pacman* create_pacman(SDL_Renderer *renderer,Musics *game_musics,Status *status);

void reset_pacman(Pacman *pacman);

SDL_bool pacman_is_collide(Pacman *pacman,Point_Float direction,int **wall_matriz);

void pacman_is_collide_point(Pacman *pacman,int **point_matriz,Ghost **ghosts);

void pacman_movement(Pacman *pacman,int **point_matriz,int **wall_matriz,Ghost **ghosts);

void pacman_input(Pacman *pacman,SDL_Event *event,int **wall_matriz);

void pacman_draw(Pacman *pacman,SDL_Renderer *renderer);

void pacman_update(Pacman *pacman,SDL_Event *event,int **wall_matriz,int **point_matriz,Ghost **ghosts);

void pacman_free(Pacman **pacman);

#endif