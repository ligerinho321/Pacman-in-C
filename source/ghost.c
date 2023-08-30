#include <ghost.h>

Ghost* create_ghost(SDL_Renderer *renderer,int **wall_matriz,Musics *musics,Status *status,Point_Float pos,int id){

    Ghost *ghost = calloc(1,sizeof(Ghost));

    ghost->initial_pos = pos;

    ghost->key = SDL_TRUE;

    char path[MAX_STRING];
    memset(path,'\0',MAX_STRING);

    snprintf(path,MAX_STRING-1,"./assets/graphics/ghost/%d/ghost_top.png",id);
    ghost->top_textures = load_sprites(renderer,path,14,ENTITIES_SIZE,&ghost->number_of_textures);

    snprintf(path,MAX_STRING-1,"./assets/graphics/ghost/%d/ghost_down.png",id);
    ghost->down_textures = load_sprites(renderer,path,14,ENTITIES_SIZE,NULL);

    snprintf(path,MAX_STRING-1,"./assets/graphics/ghost/%d/ghost_left.png",id);
    ghost->left_textures = load_sprites(renderer,path,14,ENTITIES_SIZE,NULL);

    snprintf(path,MAX_STRING-1,"./assets/graphics/ghost/%d/ghost_right.png",id);
    ghost->right_textures = load_sprites(renderer,path,14,ENTITIES_SIZE,NULL);

    ghost->vuneravel_0_textures = load_sprites(renderer,"./assets/graphics/ghost/vuneravel/ghost_vuneravel_0.png",14,ENTITIES_SIZE,NULL);
    ghost->vuneravel_1_textures = load_sprites(renderer,"./assets/graphics/ghost/vuneravel/ghost_vuneravel_1.png",14,ENTITIES_SIZE,NULL);

    ghost->recuando_top_texture = load_sprites(renderer,"./assets/graphics/ghost/recuando/recuando_top.png",14,ENTITIES_SIZE,NULL);
    ghost->recuando_down_texture = load_sprites(renderer,"./assets/graphics/ghost/recuando/recuando_down.png",14,ENTITIES_SIZE,NULL);
    ghost->recuando_left_texture = load_sprites(renderer,"./assets/graphics/ghost/recuando/recuando_left.png",14,ENTITIES_SIZE,NULL);
    ghost->recuando_right_texture = load_sprites(renderer,"./assets/graphics/ghost/recuando/recuando_right.png",14,ENTITIES_SIZE,NULL);

    ghost->musics = musics;
    ghost->status = status;

    reset_ghost(ghost,wall_matriz);

    return ghost;
}

void reset_ghost(Ghost *ghost,int **wall_matriz){

    ghost->pos = ghost->initial_pos;
    ghost->next_pos = ghost->initial_pos;

    ghost->direction.x = 0.0f;
    ghost->direction.y = 0.0f;

    ghost->frame_index = 0.0f;

    ghost->visible = SDL_TRUE;
    ghost->recuando = SDL_FALSE;
    ghost->vuneravel = SDL_FALSE;

    if(ghost->path){
        free_path(ghost->path);
        ghost->path = NULL;
    }

    Point_Int start = {ghost->pos.x/TILE_SIZE,ghost->pos.y/TILE_SIZE};
    Point_Int end;

    do{
        end.x = rand() % MAP_WIDTH;
        end.y = rand() % MAP_HEIGHT;
    }while(wall_matriz[end.y][end.x] != -1 || end.x > 10 && end.x < 17 && end.y > 13 && end.y < 17);

    ghost->path = a_star_search(wall_matriz,start,end);
}

void ghost_get_next_movement(Ghost *ghost,Pacman *pacman,int **wall_matriz){

    if(!ghost->path){

        if(ghost->recuando && ghost->pos.x == ghost->initial_pos.x && ghost->pos.y == ghost->initial_pos.y){
            ghost->recuando = SDL_FALSE;
            if(ghost->status->ghosts_vuneravel){
                Mix_PlayMusic(ghost->musics->power_pellet_music,-1);
                ghost->musics->current_music = ghost->musics->power_pellet_music;
            }
            else{
                Mix_PlayMusic(ghost->musics->siren_1_music,-1);
                ghost->musics->current_music = ghost->musics->siren_1_music;
            }
        }

        Point_Int start = {ghost->pos.x/TILE_SIZE,ghost->pos.y/TILE_SIZE};
        Point_Int end;

        if(ghost->recuando){
            end.x = ghost->initial_pos.x/TILE_SIZE;
            end.y = ghost->initial_pos.y/TILE_SIZE;
        }
        else if(get_distanci(pacman->pos,ghost->pos) <= MIN_DISTANCI && !ghost->vuneravel){
            end.x = pacman->pos.x/TILE_SIZE;
            end.y = pacman->pos.y/TILE_SIZE;
        }
        else{
            do{
                end.x = rand() % MAP_WIDTH;
                end.y = rand() % MAP_HEIGHT;
            }while(wall_matriz[end.y][end.x] != -1 || end.x > 10 && end.x < 17 && end.y > 13 && end.y < 17);
        }

        ghost->path = a_star_search(wall_matriz,start,end);

    }

    ghost->next_pos.x = ghost->path->x*TILE_SIZE;
    ghost->next_pos.y = ghost->path->y*TILE_SIZE;

    if(ghost->next_pos.x > ghost->pos.x){
        ghost->direction.x = 1.0f;
        ghost->face = RIGHT;
    }
    else if(ghost->next_pos.x < ghost->pos.x){
        ghost->direction.x = -1.0f;
        ghost->face = LEFT;
    }
    else if(ghost->next_pos.y > ghost->pos.y){
        ghost->direction.y = 1.0f;
        ghost->face = DOWN;
    }
    else if(ghost->next_pos.y < ghost->pos.y){
        ghost->direction.y = -1.0f;
        ghost->face = TOP;
    }

    Path *buffer_path = ghost->path->next;
    free(ghost->path);
    ghost->path = buffer_path;
}

void ghost_movement(Ghost *ghost,Pacman *pacman,int **wall_matriz){

    if(!ghost->direction.x && !ghost->direction.y){
        ghost_get_next_movement(ghost,pacman,wall_matriz);
    }

    float velocity;

    if(ghost->vuneravel){
        velocity = GHOST_LOWER_VELOCITY;
    }   
    else if(ghost->recuando){
        velocity = GHOST_FAST_VELOCITY;
    }
    else{
        velocity = GHOST_VELOCITY;
    }

    float delta_x = fabsf(ghost->next_pos.x-ghost->pos.x);
    float delta_y = fabsf(ghost->next_pos.y-ghost->pos.y);

    if(delta_x > velocity * ghost->status->delta_time){
        ghost->pos.x += ghost->direction.x * velocity * ghost->status->delta_time;
    }
    else if(delta_y > velocity * ghost->status->delta_time){
        ghost->pos.y += ghost->direction.y * velocity * ghost->status->delta_time;
    }
    else{
        ghost->pos = ghost->next_pos;
        ghost->direction.x = 0.0f;
        ghost->direction.y = 0.0f;
    }
}

void ghost_draw(Ghost *ghost,SDL_Renderer *renderer){

    if(!ghost->visible) return;

    ghost->frame_index += GHOST_FRAME_SPEED * ghost->status->delta_time;

    if(ghost->frame_index >= ghost->number_of_textures){
        ghost->frame_index = 0.0f;
    }

    if(ghost->vuneravel){
        if(ghost->key){
            ghost->current_texture = ghost->vuneravel_0_textures[(int)ghost->frame_index];
        }
        else{
            ghost->current_texture = ghost->vuneravel_1_textures[(int)ghost->frame_index];
        }
    }
    else if(ghost->recuando){
        switch(ghost->face){
            case TOP:
                ghost->current_texture = ghost->recuando_top_texture[0];
                break;
            case DOWN:
                ghost->current_texture = ghost->recuando_down_texture[0];
                break;
            case LEFT:
                ghost->current_texture = ghost->recuando_left_texture[0];
                break;
            case RIGHT:
                ghost->current_texture = ghost->recuando_right_texture[0];
                break;
        }
    }
    else{
        switch(ghost->face){
            case TOP:
                ghost->current_texture = ghost->top_textures[(int)ghost->frame_index];
                break;
            case DOWN:
                ghost->current_texture = ghost->down_textures[(int)ghost->frame_index];
                break;
            case LEFT:
                ghost->current_texture = ghost->left_textures[(int)ghost->frame_index];
                break;
            case RIGHT:
                ghost->current_texture = ghost->right_textures[(int)ghost->frame_index];
                break;
            }
    }

    SDL_Rect rect = {(ghost->pos.x + TILE_SIZE*0.5f) - ENTITIES_SIZE*0.5f,
                     (ghost->pos.y + TILE_SIZE*0.5f) - ENTITIES_SIZE*0.5f,
                     ENTITIES_SIZE,
                     ENTITIES_SIZE};

    SDL_RenderCopy(renderer,ghost->current_texture,NULL,&rect);
}

void ghost_update(Ghost *ghost,Pacman *pacman,int **wall_matriz){
    ghost_movement(ghost,pacman,wall_matriz);

    if(ghost->recuando && (!Mix_PlayingMusic() || ghost->musics->current_music != ghost->musics->retreating_music)){
        Mix_PlayMusic(ghost->musics->retreating_music,0);
        ghost->musics->current_music = ghost->musics->retreating_music;
    }

    if(ghost->vuneravel){
        int current_time = SDL_GetTicks();

        if(current_time - ghost->time_vuneravel >= VUNERAVEL_TIME){
            ghost->vuneravel = SDL_FALSE;
        }
        else if(current_time - ghost->time_vuneravel >= VUNERAVEL_TIME*0.7f && current_time - ghost->time_end_vuneravel >= BLINK_TIME){
            ghost->key = !ghost->key;
            ghost->time_end_vuneravel = current_time;
        }
    }

}

void ghost_free(Ghost *ghost){

    free_path(ghost->path);

    for(int i=0; i<ghost->number_of_textures; ++i){

        SDL_DestroyTexture(ghost->top_textures[i]);
        SDL_DestroyTexture(ghost->down_textures[i]);
        SDL_DestroyTexture(ghost->left_textures[i]);
        SDL_DestroyTexture(ghost->right_textures[i]);

        SDL_DestroyTexture(ghost->vuneravel_0_textures[i]);
        SDL_DestroyTexture(ghost->vuneravel_1_textures[i]);
    }

    free(ghost->vuneravel_0_textures);
    free(ghost->vuneravel_1_textures);

    free(ghost->top_textures);
    free(ghost->down_textures);
    free(ghost->left_textures);
    free(ghost->right_textures);


    SDL_DestroyTexture(ghost->recuando_top_texture[0]);
    SDL_DestroyTexture(ghost->recuando_down_texture[0]);
    SDL_DestroyTexture(ghost->recuando_left_texture[0]);
    SDL_DestroyTexture(ghost->recuando_right_texture[0]);

    free(ghost->recuando_top_texture);
    free(ghost->recuando_down_texture);
    free(ghost->recuando_left_texture);
    free(ghost->recuando_right_texture);
}