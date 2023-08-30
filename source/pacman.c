#include <pacman.h>

Pacman* create_pacman(SDL_Renderer *renderer,Musics *musics,Status *status){

    Pacman *pacman = calloc(1,sizeof(Pacman));

    pacman->initial_pos.x = 13*TILE_SIZE;
    pacman->initial_pos.y = 18*TILE_SIZE;

    pacman->vidas = 3;
    pacman->pontos = 0;

    pacman->run_textures = load_sprites(renderer,"./assets/graphics/pacman_run.png",13,ENTITIES_SIZE,&pacman->number_of_run_textures);
    pacman->death_textures = load_sprites(renderer,"./assets/graphics/pacman_death.png",15,ENTITIES_SIZE,&pacman->number_of_death_textures);

    pacman->musics = musics;
    pacman->status = status;

    reset_pacman(pacman);

    return pacman;
}

void reset_pacman(Pacman *pacman){

    pacman->pos = pacman->initial_pos;
    pacman->next_pos = pacman->next_pos;

    pacman->direction.x = 0.0f;
    pacman->direction.y = 0.0f;

    pacman->buffer_direction = pacman->direction;

    pacman->frame_index = 2.0f;

    pacman->current_texture = pacman->run_textures[2];

    pacman->face = RIGHT;
    pacman->death = SDL_FALSE;
    pacman->dying = SDL_FALSE;
    pacman->key = SDL_TRUE;
}

void get_pacman_face(Pacman *pacman){
    if(pacman->direction.x > 0) pacman->face = RIGHT;
    else if(pacman->direction.x < 0) pacman->face = LEFT;
    else if(pacman->direction.y > 0) pacman->face = DOWN;
    else if(pacman->direction.y < 0) pacman->face = TOP;
}

SDL_bool pacman_is_collide(Pacman *pacman,Point_Float direction,int **wall_matriz){

    int x = (pacman->pos.x/TILE_SIZE)+direction.x;
    int y = (pacman->pos.y/TILE_SIZE)+direction.y;

    if((x == 13 || x == 14) && y == 13) return SDL_TRUE;

    if(x < 0 || x >= MAP_WIDTH) return SDL_FALSE;

    if(wall_matriz[y][x] == -1) return SDL_FALSE;
    
    return SDL_TRUE;
}

void pacman_is_collide_point(Pacman *pacman,int **point_matriz,Ghost **ghosts){

    int index = point_matriz[(int)(pacman->pos.y/TILE_SIZE)][(int)(pacman->pos.x/TILE_SIZE)];
    
    if(index == -1) return;

    pacman->status->number_of_points--;

    if(pacman->key) Mix_PlayChannel(0,pacman->musics->munch_1_sound,0);
    else            Mix_PlayChannel(0,pacman->musics->munch_2_sound,0);

    pacman->key = !pacman->key;

    if(index == 0){
        pacman->pontos += 10;
    }
    else if(index == 2){

        pacman->pontos += 100;

        Mix_PlayMusic(pacman->musics->power_pellet_music,-1);
        pacman->musics->current_music = pacman->musics->power_pellet_music;

        int current_time = SDL_GetTicks();

        pacman->status->ghosts_vuneravel = SDL_TRUE;
        pacman->status->time_ghosts_vuneravel = current_time;

        for(int i=0; i<4; ++i){
            if(!ghosts[i]->recuando){
                ghosts[i]->vuneravel = SDL_TRUE;
                ghosts[i]->time_vuneravel = current_time;
                ghosts[i]->time_end_vuneravel = current_time;
                ghosts[i]->key = SDL_TRUE;
            }
        }
    }

    point_matriz[(int)(pacman->pos.y/TILE_SIZE)][(int)(pacman->pos.x/TILE_SIZE)] = -1;
}

void pacman_movement(Pacman *pacman,int **point_matriz,int **wall_matriz,Ghost **ghosts){

    float delta_x = fabsf(pacman->next_pos.x-pacman->pos.x);
    float delta_y = fabsf(pacman->next_pos.y-pacman->pos.y);

    if(delta_x > PACMAN_VELOCITY * pacman->status->delta_time){
        pacman->pos.x += pacman->direction.x * PACMAN_VELOCITY * pacman->status->delta_time;
    }
    else if(delta_y > PACMAN_VELOCITY * pacman->status->delta_time){
        pacman->pos.y += pacman->direction.y * PACMAN_VELOCITY * pacman->status->delta_time;
    }
    else{
        pacman->pos.x = pacman->next_pos.x;
        pacman->pos.y = pacman->next_pos.y;

        if(pacman->pos.x < 0){
            pacman->pos.x = (MAP_WIDTH-1)*TILE_SIZE;
        }
        else if(pacman->pos.x >= MAP_WIDTH*TILE_SIZE){
            pacman->pos.x = 0;
        }

        pacman_is_collide_point(pacman,point_matriz,ghosts);

        if((pacman->buffer_direction.x || pacman->buffer_direction.y) && !pacman_is_collide(pacman,pacman->buffer_direction,wall_matriz)){
            pacman->direction = pacman->buffer_direction;

            pacman->next_pos.x = pacman->pos.x + pacman->direction.x * TILE_SIZE;
            pacman->next_pos.y = pacman->pos.y + pacman->direction.y * TILE_SIZE;
        }
        else if(!pacman_is_collide(pacman,pacman->direction,wall_matriz)){

            pacman->next_pos.x = pacman->pos.x + pacman->direction.x * TILE_SIZE;
            pacman->next_pos.y = pacman->pos.y + pacman->direction.y * TILE_SIZE;
        }
        else{
            pacman->direction.x = 0.0f;
            pacman->direction.y = 0.0f;
        }
    }
}

void pacman_input(Pacman *pacman,SDL_Event *event,int **wall_matriz){

    const Uint8 *keys = SDL_GetKeyboardState(NULL);

    Point_Float new_direction = {0.0f,0.0f};

    if(keys[SDL_SCANCODE_W]){
        new_direction.y = -1.0f;
    }
    else if(keys[SDL_SCANCODE_S]){
        new_direction.y = 1.0f;
    }
    else if(keys[SDL_SCANCODE_A]){
        new_direction.x = -1.0f;
    }
    else if(keys[SDL_SCANCODE_D]){
        new_direction.x = 1.0f;
    }

    if(!new_direction.x && !new_direction.y) return;

    if(!pacman->direction.x && !pacman->direction.y && !pacman_is_collide(pacman,new_direction,wall_matriz)){
        pacman->direction = new_direction;
        pacman->next_pos.x = pacman->pos.x + pacman->direction.x * TILE_SIZE;
        pacman->next_pos.y = pacman->pos.y + pacman->direction.y * TILE_SIZE;
    }
    else{
        pacman->buffer_direction = new_direction;
    }

}

void pacman_draw(Pacman *pacman,SDL_Renderer *renderer){

    get_pacman_face(pacman);

    SDL_Rect rect = {(pacman->pos.x + TILE_SIZE*0.5f) - ENTITIES_SIZE*0.5f,
                     (pacman->pos.y + TILE_SIZE*0.5f) - ENTITIES_SIZE*0.5f,
                     ENTITIES_SIZE,
                     ENTITIES_SIZE
                    };
    
    if(pacman->death && pacman->dying){
        pacman->frame_index += pacman->status->delta_time * PACMAN_DEATH_SPEED;

        if(pacman->frame_index >= pacman->number_of_death_textures){
            pacman->frame_index = 0;
            pacman->dying = SDL_FALSE;
            pacman->current_texture = NULL;
        }

        if(pacman->current_texture){
            pacman->current_texture = pacman->death_textures[(int)pacman->frame_index];
            SDL_RenderCopy(renderer,pacman->current_texture,NULL,&rect);
        }
    }

    if(!pacman->death){

        if(pacman->direction.x || pacman->direction.y){
            pacman->frame_index += pacman->status->delta_time * PACMAN_FRAME_SPEED;
            
            if(pacman->frame_index >= pacman->number_of_run_textures){
                pacman->frame_index = 0;
            }
            pacman->current_texture = pacman->run_textures[(int)pacman->frame_index];
        }
        SDL_RenderCopyEx(renderer,pacman->current_texture,NULL,&rect,pacman->face,NULL,SDL_FLIP_NONE);
    }
    
}

void pacman_update(Pacman *pacman,SDL_Event *event,int **wall_matriz,int **point_matriz,Ghost **ghosts){
    
    if(pacman->death) return;

    pacman_input(pacman,event,wall_matriz);
    pacman_movement(pacman,point_matriz,wall_matriz,ghosts);
}

void pacman_free(Pacman **pacman){

    for(int i=0; i<(*pacman)->number_of_run_textures; ++i){
        SDL_DestroyTexture((*pacman)->run_textures[i]);
    }

    for(int i=0; i<(*pacman)->number_of_death_textures; ++i){
        SDL_DestroyTexture((*pacman)->death_textures[i]);
    }

    free((*pacman)->death_textures);
    free((*pacman)->run_textures);

    free(*pacman);
}