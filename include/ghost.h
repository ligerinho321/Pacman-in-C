#ifndef GHOST_H_INCLUDE
#define GHOST_H_INCLUDE

#include <utils.h>
#include <path_finding.h>

Ghost* create_ghost(SDL_Renderer *renderer,int **wall_matriz,Musics *musics,Status *status,Point_Float pos,int id);
void ghost_draw(Ghost *ghost,SDL_Renderer *renderer);
void ghost_get_next_movement(Ghost *Ghost,Pacman *pacman,int **wall_matriz);
void ghost_movement(Ghost *ghost,Pacman *pacman,int **wall_matriz);
void ghost_update(Ghost *ghost,Pacman *pacman,int **wall_matriz);
void reset_ghost(Ghost *ghost,int **wall_matriz);
void ghost_free(Ghost *ghost);

#endif