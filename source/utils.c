#include <utils.h>

char* cat_string(char *string,char value){
    if(string){
        int size_string = strlen(string);

        char *new_string = calloc(size_string+2,sizeof(char));

        strcpy(new_string,string);
        new_string[size_string] = value;
        new_string[size_string+1] = '\0';
        free(string);

        return new_string;
    }
    else{

        char *new_string = calloc(2,sizeof(char));
        new_string[0] = value;
        new_string[1] = '\0';

        return new_string;
    }
}

SDL_Texture** load_sprites(SDL_Renderer *renderer,char *path,int origin_sprite_size,int dest_sprite_size,size_t *number_of_sprites){

    SDL_Surface *surface = IMG_Load(path);

    if(!surface){
        printf("falha em carregar path: %s\n",path);
        return NULL;
    }
    
    size_t size = (surface->w/origin_sprite_size)*(surface->h/origin_sprite_size);

    SDL_Texture **textures = calloc(size,sizeof(SDL_Texture*));

    int index = 0;
    SDL_Rect dst_rect = {0,0,dest_sprite_size,dest_sprite_size};

    for(int y=0; y<surface->h; y+=origin_sprite_size){
        for(int x=0; x<surface->w; x+=origin_sprite_size){
            
            SDL_Surface *tile = SDL_CreateRGBSurface(0,dest_sprite_size,dest_sprite_size,32,0xff,0xff00,0xff0000,0xff000000);

            SDL_Rect rect = {x,y,origin_sprite_size,origin_sprite_size};
            SDL_BlitScaled(surface,&rect,tile,&dst_rect);

            textures[index] = SDL_CreateTextureFromSurface(renderer,tile);

            ++index;
            SDL_FreeSurface(tile);
        }
    }

    SDL_FreeSurface(surface);

    if(number_of_sprites) *number_of_sprites = size;
    
    return textures;
}

int** load_layout(char *path){

    FILE *file = fopen(path,"r");
    if(!file){
        printf("falha em carregar path: %s\n",path);
        return NULL;
    }

    char linha[MAX_STRING];
    memset(linha,'\0',MAX_STRING);

    int y = 0;
    int x = 0;

    int **layout = calloc(MAP_HEIGHT,sizeof(int*));

    while(fgets(linha,MAX_STRING-1,file)){
        layout[y] = calloc(MAP_WIDTH,sizeof(int));

        char *p = linha;
        char *value = NULL;
        x = 0;

        while(*p != '\0'){

            if(*p == ',' || *p == '\n'){

                layout[y][x] = atoi(value);
                ++x;

                free(value);
                value = NULL;
            }
            else{
                value = cat_string(value,*p);
            }

            ++p;
        }

        ++y;
    }
    fclose(file);
    return layout;
}

float get_distanci(Point_Float ponto1,Point_Float ponto2){
    return sqrtf((ponto2.x-ponto1.x) * (ponto2.x-ponto1.x) + (ponto2.y-ponto1.y) * (ponto2.y-ponto1.y));
}

SDL_bool collide_rect(Point_Float pos1,Point_Float pos2){

    SDL_Rect rect1 = {(pos1.x+TILE_SIZE*0.5f)-ENTITIES_SIZE*0.5f,
                      (pos1.y+TILE_SIZE*0.5f)-ENTITIES_SIZE*0.5f,
                      ENTITIES_SIZE*0.5f,
                      ENTITIES_SIZE*0.5f
                     };

    SDL_Rect rect2 = {(pos2.x+TILE_SIZE*0.5f)-ENTITIES_SIZE*0.5f,
                      (pos2.y+TILE_SIZE*0.5f)-ENTITIES_SIZE*0.5f,
                      ENTITIES_SIZE*0.5f,
                      ENTITIES_SIZE*0.5f
                     };

    return ((rect1.x + rect1.w > rect2.x && rect1.x < rect2.x + rect2.w) && (rect1.y + rect1.h > rect2.y && rect1.y < rect2.y + rect2.h));
}

void draw_bar_elements(SDL_Renderer *renderer,Pacman *pacman,TTF_Font *font,SDL_Texture *live_texture,SDL_Rect bar_rect){

    char buffer[MAX_STRING];
    memset(buffer,'\0',MAX_STRING);

    SDL_Color text_color = {255,255,255,255};

    SDL_Rect points_rect;
    SDL_Rect live_rect;
    SDL_Rect lives_rect;

    //draw points
    //-----------------------------------------------------------------------------------------
    snprintf(buffer,MAX_STRING-1,"points: %d",pacman->pontos);

    SDL_Surface *surface_points = TTF_RenderText_Solid(font,buffer,text_color);
    SDL_Texture *texture_points = SDL_CreateTextureFromSurface(renderer,surface_points);

    points_rect.x = SPACING;
    points_rect.y = 7;
    points_rect.w = surface_points->w;
    points_rect.h = surface_points->h;

    SDL_RenderCopy(renderer,texture_points,NULL,&points_rect);

    SDL_FreeSurface(surface_points);
    SDL_DestroyTexture(texture_points);
    //------------------------------------------------------------------------------------------

    //draw lives
    //-------------------------------------------------------------------------------------------
    snprintf(buffer,MAX_STRING-1,"lives:");

    SDL_Surface *surface_lives = TTF_RenderText_Solid(font,buffer,text_color);
    SDL_Texture *texture_lives = SDL_CreateTextureFromSurface(renderer,surface_lives);

    live_rect.x = bar_rect.w*0.5f;
    live_rect.y = 7;
    live_rect.w = surface_lives->w;
    live_rect.h = surface_lives->h;

    SDL_RenderCopy(renderer,texture_lives,NULL,&live_rect);

    SDL_FreeSurface(surface_lives);
    SDL_DestroyTexture(texture_lives);

    for(int i=0; i<pacman->vidas; ++i){

        lives_rect.x = live_rect.x + live_rect.w + i * TILE_SIZE + SPACING;
        lives_rect.y = live_rect.y;
        lives_rect.w = TILE_SIZE;
        lives_rect.h = TILE_SIZE;

        SDL_RenderCopy(renderer,live_texture,NULL,&lives_rect);
    }

}

void free_path(Path *path){
    while(path){
        Path *buffer_path = path->next;
        free(path);
        path = buffer_path;
    }
}

int** load_point_matriz(int **point_matriz,size_t *number_of_points){

    if(point_matriz){
        for(int y=0; y<MAP_HEIGHT; ++y){
            free(point_matriz[y]);
        }
        free(point_matriz);
    }

    *number_of_points = 0;

    FILE *file = fopen("./assets/mapa/mapa_points.txt","r");

    char linha[MAX_STRING];
    memset(linha,'\0',MAX_STRING);

    int y = 0;
    int x = 0;

    point_matriz = calloc(MAP_HEIGHT,sizeof(int*));

    while(fgets(linha,MAX_STRING-1,file)){
        point_matriz[y] = calloc(MAP_WIDTH,sizeof(int));

        char *p = linha;
        char *value = NULL;
        x = 0;

        while(*p != '\0'){

            if(*p == ',' || *p == '\n'){

                point_matriz[y][x] = atoi(value);
                if(point_matriz[y][x] != -1) *number_of_points += 1;
                ++x;

                free(value);
                value = NULL;
            }
            else{
                value = cat_string(value,*p);
            }

            ++p;
        }

        ++y;
    }
    
    fclose(file);

    return point_matriz;
}

Musics* load_game_musics(){

    Musics *musics = calloc(1,sizeof(Musics));

    musics->siren_1_music = Mix_LoadMUS("./assets/sounds/siren_1.wav");
    musics->start_music = Mix_LoadMUS("./assets/sounds/game_start.wav");
    musics->power_pellet_music = Mix_LoadMUS("./assets/sounds/power_pellet.wav");
    musics->retreating_music = Mix_LoadMUS("./assets/sounds/retreating.wav");
    musics->pacman_death_music = Mix_LoadMUS("./assets/sounds/death.wav");

    musics->eat_ghost_sound = Mix_LoadWAV("./assets/sounds/eat_ghost.wav");
    musics->munch_1_sound = Mix_LoadWAV("./assets/sounds/munch_1.wav");
    musics->munch_2_sound = Mix_LoadWAV("./assets/sounds/munch_2.wav");

    musics->current_music = NULL;
    musics->current_chunk = NULL;

    return musics;

}

void free_game_musics(Musics **musics){

    Mix_FreeMusic((*musics)->siren_1_music);
    Mix_FreeMusic((*musics)->start_music);
    Mix_FreeMusic((*musics)->power_pellet_music);
    Mix_FreeMusic((*musics)->retreating_music);
    Mix_FreeMusic((*musics)->pacman_death_music);

    Mix_FreeChunk((*musics)->eat_ghost_sound);
    Mix_FreeChunk((*musics)->munch_1_sound);
    Mix_FreeChunk((*musics)->munch_2_sound);

    free(*musics);
}

Status* init_status(){

    Status *status = calloc(1,sizeof(Status));

    status->start = SDL_TRUE;
    status->eat_ghost = SDL_FALSE;
    status->pacman_death = SDL_FALSE;
    status->game_over = SDL_FALSE;
    status->game_win = SDL_FALSE;
    status->run = SDL_TRUE;
    status->key_wall_texture = SDL_TRUE;
    status->key_munch_sound = SDL_TRUE;
    status->ghosts_vuneravel = SDL_FALSE;

    status->time_ghosts_vuneravel = 0;
    status->time_game_win = 0;
    status->time_blink = 0;
    status->time_game_over = 0;
    status->number_of_points = 0;
    status->number_of_point_textures = 0;
    status->number_of_wall_textures = 0;

    status->previous_time = SDL_GetTicks();
    status->delta_time = 0.0f;

    return status;
}

Textures* load_textures(Status *status,SDL_Renderer *renderer,TTF_Font *font){

    Textures *textures = calloc(1,sizeof(Textures));
    SDL_Color green = {0,255,0,255};
    SDL_Color white = {255,255,255,255};

    textures->wall_0 = load_sprites(renderer,"./assets/graphics/walls_0.png",8,TILE_SIZE,&status->number_of_wall_textures);
    textures->wall_1 = load_sprites(renderer,"./assets/graphics/walls_1.png",8,TILE_SIZE,NULL);
    textures->points = load_sprites(renderer,"./assets/graphics/pontos.png",8,TILE_SIZE,&status->number_of_point_textures);
    textures->barrier = load_sprites(renderer,"./assets/graphics/barreira.png",8,TILE_SIZE,&status->number_of_barrier_textures);

    SDL_Surface *surface_200_points = TTF_RenderText_Solid(font,"200",green);
    textures->points_200 = SDL_CreateTextureFromSurface(renderer,surface_200_points);

    TTF_SetFontSize(font,TEXT_NORMAL_SIZE);

    SDL_Surface *surface_ready = TTF_RenderText_Solid(font,"ready!",white);
    textures->ready = SDL_CreateTextureFromSurface(renderer,surface_ready);

    SDL_Surface *surface_game_over = TTF_RenderText_Solid(font,"game over",white);
    textures->game_over = SDL_CreateTextureFromSurface(renderer,surface_game_over);

    SDL_Surface *surface_live = IMG_Load("./assets/graphics/live.png");
    textures->live = SDL_CreateTextureFromSurface(renderer,surface_live);

    SDL_FreeSurface(surface_200_points);
    SDL_FreeSurface(surface_live);
    SDL_FreeSurface(surface_ready);
    SDL_FreeSurface(surface_game_over);

    return textures;

}

void free_textures(Textures **textures,Status *status){

    for(int i=0; i<status->number_of_wall_textures; ++i){
        SDL_DestroyTexture((*textures)->wall_0[i]);
        SDL_DestroyTexture((*textures)->wall_1[i]);
    }
    free((*textures)->wall_0);
    free((*textures)->wall_1);

    for(int i=0; i<status->number_of_point_textures; ++i){
        SDL_DestroyTexture((*textures)->points[i]);
    }
    free((*textures)->points);

    for(int i=0; i<status->number_of_barrier_textures; ++i){
        SDL_DestroyTexture((*textures)->barrier[i]);
    }
    free((*textures)->barrier);

    SDL_DestroyTexture((*textures)->live);
    SDL_DestroyTexture((*textures)->points_200);
    SDL_DestroyTexture((*textures)->ready);
    SDL_DestroyTexture((*textures)->game_over);

    free(*textures);
}