#include <utils.h>
#include <pacman.h>
#include <ghost.h>
#include <path_finding.h>

int main(int nArgs, char **args){

    SDL_Init(SDL_INIT_EVERYTHING);
    TTF_Init();

    Mix_OpenAudio(44100,AUDIO_S16SYS,1,1024);
    Mix_Volume(0,MIX_MAX_VOLUME * 0.5f);
    Mix_VolumeMusic(MIX_MAX_VOLUME * 0.8f);

    srand(time(NULL));

    int bar_h = 30;
    int game_h = MAP_HEIGHT*TILE_SIZE;

    int screen_w = MAP_WIDTH*TILE_SIZE;
    int screen_h = game_h+bar_h;

    SDL_Rect bar_rect = {0,0,screen_w,bar_h};
    SDL_Rect game_rect = {0,bar_h,screen_w,game_h};

    SDL_Rect rect;

    SDL_Window *window = SDL_CreateWindow("Pacman",SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,screen_w,screen_h,SDL_WINDOW_SHOWN);
    SDL_Renderer *renderer = SDL_CreateRenderer(window,-1,SDL_RENDERER_ACCELERATED);
    TTF_Font *font = TTF_OpenFont("./assets/font/retro_gaming.ttf",TEXT_LOWER_SIZE);

    Status *status = init_status();

    Musics *musics = load_game_musics();

    Textures *textures = load_textures(status,renderer,font);

    int **point_matriz = load_point_matriz(NULL,&status->number_of_points);
    int **wall_matriz = load_layout("./assets/mapa/mapa_walls.txt");

    Pacman *pacman = create_pacman(renderer,musics,status);

    Ghost *ghosts[4];
    for(int i=0; i<4; i++){
        Point_Float pos = {(12.0f+i)*TILE_SIZE,15.0f*TILE_SIZE};
        ghosts[i] = create_ghost(renderer,wall_matriz,musics,status,pos,i);
    }

    Mix_PlayMusic(musics->start_music,1);
    musics->current_music = musics->start_music;

    while(status->run){

        int current_time = SDL_GetTicks();

        SDL_Event event;
        SDL_PollEvent(&event);

        if(event.type == SDL_QUIT){
            status->run = SDL_FALSE;
        }

        SDL_SetRenderDrawColor(renderer,0,0,0,255);
        SDL_RenderClear(renderer);

        draw_bar_elements(renderer,pacman,font,textures->live,bar_rect);

        SDL_RenderSetViewport(renderer,&game_rect);

        for(int y=0; y<MAP_HEIGHT; ++y){
            for(int x=0; x<MAP_WIDTH; ++x){

                SDL_Rect rect = {x*TILE_SIZE,y*TILE_SIZE,TILE_SIZE,TILE_SIZE};

                if(wall_matriz[y][x] > -1){

                    if(status->key_wall_texture) SDL_RenderCopy(renderer,textures->wall_1[wall_matriz[y][x]],NULL,&rect);
                    else                 SDL_RenderCopy(renderer,textures->wall_0[wall_matriz[y][x]],NULL,&rect);
                }
                if(point_matriz[y][x] > -1){
                    SDL_RenderCopy(renderer,textures->points[point_matriz[y][x]],NULL,&rect);
                }
            }
        }

        rect.x = 13*TILE_SIZE, rect.y = 13*TILE_SIZE, rect.w = TILE_SIZE, rect.h = TILE_SIZE;
        SDL_RenderCopy(renderer,textures->barrier[0],NULL,&rect);
        rect.x = 14*TILE_SIZE;
        SDL_RenderCopy(renderer,textures->barrier[1],NULL,&rect);

        if(!status->game_over){

            if(!status->eat_ghost){
                if(!status->start && !status->game_win) pacman_update(pacman,&event,wall_matriz,point_matriz,ghosts);
                pacman_draw(pacman,renderer);

            }
            
            if(!status->start && !status->pacman_death){
                
                for(int i=0; i<4; ++i){
                    
                    if(collide_rect(pacman->pos,ghosts[i]->pos)){
                        
                        if(ghosts[i]->vuneravel){
                            
                            pacman->pontos += 200;
                            
                            ghosts[i]->vuneravel = SDL_FALSE;
                            ghosts[i]->recuando = SDL_TRUE;
                            ghosts[i]->visible = SDL_FALSE;

                            free_path(ghosts[i]->path);
                            ghosts[i]->path = NULL;
                        
                            status->eat_ghost = SDL_TRUE;
                            Mix_PlayChannel(0,musics->eat_ghost_sound,0);
                            musics->current_chunk = musics->eat_ghost_sound;
                        }
                        
                        if(!ghosts[i]->recuando && !ghosts[i]->vuneravel && !status->pacman_death){
                            
                            pacman->death = SDL_TRUE;
                            pacman->dying = SDL_TRUE;
                            pacman->frame_index = 0;

                            Mix_PlayMusic(musics->pacman_death_music,0);
                            musics->current_music = musics->pacman_death_music;
                            
                            status->ghosts_vuneravel = SDL_FALSE;
                            status->pacman_death = SDL_TRUE;
                        }
                    }

                    if(!status->pacman_death){
                        
                        if(!status->eat_ghost && !status->game_win){
                            
                            ghost_update(ghosts[i],pacman,wall_matriz);
                            ghosts[i]->visible = SDL_TRUE;
                        }
                        
                        ghost_draw(ghosts[i],renderer);
                    }
                }
            }
        }

        if(!Mix_Playing(0)) musics->current_chunk = NULL;
        if(!Mix_PlayingMusic()) musics->current_music = NULL;

        if(status->number_of_points == 0){

            if(!status->game_win){

                status->game_win = SDL_TRUE;
                status->ghosts_vuneravel = SDL_FALSE;
                status->eat_ghost = SDL_FALSE;

                status->time_game_win = current_time;
                status->time_blink = current_time;
                Mix_HaltMusic();
                Mix_HaltChannel(0);
            }
            else if(current_time - status->time_blink >= BLINK_TIME){
                status->key_wall_texture = !status->key_wall_texture;
                status->time_blink = current_time;
            }
        }

        if(status->ghosts_vuneravel){

            if(current_time - status->time_ghosts_vuneravel >= VUNERAVEL_TIME){
                status->ghosts_vuneravel = SDL_FALSE;
                
                Mix_PlayMusic(musics->siren_1_music,-1);
                musics->current_music = musics->siren_1_music;
            }
        }

        if(status->eat_ghost){

            SDL_QueryTexture(textures->points_200,NULL,NULL,&rect.w,&rect.h);
            rect.x = (pacman->pos.x+TILE_SIZE*0.5f)-(rect.w*0.5f);
            rect.y = pacman->pos.y;
                
            SDL_RenderCopy(renderer,textures->points_200,NULL,&rect);
                
            if(!musics->current_chunk){
                status->eat_ghost = SDL_FALSE;
            }
        }

        if(status->start){

            SDL_QueryTexture(textures->ready,NULL,NULL,&rect.w,&rect.h);
            rect.x = 14*TILE_SIZE - rect.w*0.5f;
            rect.y = 15*TILE_SIZE;

            SDL_RenderCopy(renderer,textures->ready,NULL,&rect);

            if(!musics->current_music){

                status->start = SDL_FALSE;
                Mix_PlayMusic(musics->siren_1_music,-1);
                musics->current_music = musics->siren_1_music;
            }
        }

        if(status->pacman_death && !musics->current_music || status->game_win && current_time - status->time_game_win >= WIN_TIME){
            
            reset_pacman(pacman);

            for(int i=0; i<4; ++i){
                reset_ghost(ghosts[i],wall_matriz);
            }

            if(status->pacman_death) --pacman->vidas;

            if(pacman->vidas == 0){
                pacman->vidas = 3;
                pacman->pontos = 0;

                status->game_over = SDL_TRUE;
                status->time_game_over = current_time;
            }
            else{
                status->start = SDL_TRUE;

                Mix_PlayMusic(musics->start_music,1);
                musics->current_music = musics->start_music;
            }

            if(status->game_over || status->game_win) point_matriz = load_point_matriz(point_matriz,&status->number_of_points);

            status->pacman_death = SDL_FALSE;
            status->game_win = SDL_FALSE;
            status->key_wall_texture = SDL_TRUE;

        }

        if(status->game_over){

            SDL_QueryTexture(textures->game_over,NULL,NULL,&rect.w,&rect.h);
            rect.x = 14*TILE_SIZE - rect.w*0.5f;
            rect.y = 15*TILE_SIZE;

            SDL_RenderCopy(renderer,textures->game_over,NULL,&rect);

            if(current_time - status->time_game_over >= GAME_OVER_TIME){
                status->game_over = SDL_FALSE;

                status->start = SDL_TRUE;
                Mix_PlayMusic(musics->start_music,0);
                musics->current_music = musics->start_music;
            }
        }

        SDL_RenderPresent(renderer);
        SDL_RenderSetViewport(renderer,NULL);

        status->delta_time = (current_time - status->previous_time) / 1000.0f;
        status->previous_time = current_time;
    }



    for(int y=0; y<MAP_HEIGHT; ++y){
        free(wall_matriz[y]);
        free(point_matriz[y]);
    }
    free(wall_matriz);
    free(point_matriz);

    
    for(int i=0; i<4; ++i){
        ghost_free(ghosts[i]);
        free(ghosts[i]);
    }

    pacman_free(&pacman);

    free_game_musics(&musics);

    free_textures(&textures,status);

    free(status);

    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    Mix_CloseAudio();
    TTF_Quit();
    SDL_Quit();
    
    return 0;
}