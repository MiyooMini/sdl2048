#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include <SDL/SDL_image.h>
#include <time.h>

#include "SDL/SDL_mixer.h"

#define BGM_MUSIC         "sound/backgroud.mp3"
#define WIN_MUSIC         "sound/win.mp3"
#define GAMEOVER_MUSIC    "sound/gameover.mp3"
#define MERGE_MUSIC       "sound/merge.wav"

#define DEFAULT_BGM_FREQ    48000
#define DEFAULT_BGM_FORMAT  AUDIO_S16LSB
#define DEFAULT_BGM_CHAN    2
#define DEFAULT_BGM_CHUNK   4096

int pad = 10; //padding of the table

const int WIDTH = 640;
const int HEIGHT = 480;

SDL_Surface* screen = NULL;
SDL_Surface* screenbuffer = NULL;
SDL_Surface* gameover = NULL;
SDL_Surface* gameoverb = NULL;
TTF_Font* font;
TTF_Font* tinyfont;

SDL_bool done = SDL_FALSE;

int tiles[16];
int score = 0;

int last[16]; //for undo
int lastscore = -1;

bool changed = false;

//colors
Uint32 _2 = 0;
Uint32 _4 = 0;
Uint32 _8 = 0;
Uint32 _16 = 0;
Uint32 _32 = 0;
Uint32 _64 = 0;
Uint32 _128 = 0;
Uint32 _256 = 0;
Uint32 _512 = 0;
Uint32 _1024 = 0;
Uint32 _2048 = 0;
Uint32 _other = 0;


Mix_Music *gBGMMusic = NULL;
Mix_Music *gWin = NULL;
Mix_Music *gGameover = NULL;
Mix_Chunk *gMerge = NULL;

#define ALIGN_CENTER 0
#define ALIGN_LEFT   1
#define ALIGN_RIGHT  2
static int draw_string(SDL_Surface *dst, TTF_Font *font, SDL_Rect &rect, const char *str, SDL_Color &color, int align) {
	SDL_Rect  drect;

	if (!font) {
		return 0;
	}

    //SDL_Surface * text = TTF_RenderUTF8_Solid(font, str, color);
	SDL_Surface * text = TTF_RenderUTF8_Blended(font, str, color);
	
    if(align == ALIGN_CENTER){
		drect.x = rect.x + (rect.w - text->w) / 2;
		drect.y = rect.y + (rect.h - text->h) / 2;

	}else if(align == ALIGN_LEFT){
		drect.x = rect.x;
		drect.y = rect.y;
	}else{
		drect.x = rect.x + rect.w - text->w;
		drect.y = rect.y;
	}
	drect.w = text->w;
	drect.h = text->h;

    SDL_BlitSurface(text, NULL, dst, &drect);
    int textlen = text->w;
    SDL_FreeSurface(text);
	return textlen;
}

void init(void)
{   
    TTF_Init();
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) == -1) {
        printf("Couldn't initialize SDL: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }
#ifdef X86    
    screen = SDL_SetVideoMode(WIDTH, HEIGHT,  16, SDL_HWSURFACE | SDL_DOUBLEBUF);
#else    
    screen = SDL_SetVideoMode(WIDTH, HEIGHT,  32, SDL_HWSURFACE | SDL_DOUBLEBUF);
#endif    
    if (screen == NULL) {
        printf("Couldn't initialize display: %s\n", SDL_GetError());
        SDL_Quit();
        exit(EXIT_FAILURE);
    }
#ifdef X86
    screenbuffer = SDL_CreateRGBSurface(SDL_HWSURFACE, WIDTH, HEIGHT, 16, 0xF800, 0x7E0, 0x1F, 0);
#else
    screenbuffer = SDL_CreateRGBSurface(SDL_HWSURFACE, WIDTH, HEIGHT, 32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);
#endif    


    font = TTF_OpenFont("HYZhengYuan-95W.ttf", 18);
    tinyfont = TTF_OpenFont("HYZhengYuan-95W.ttf", 16);

    SDL_ShowCursor(SDL_DISABLE);
    srand(time(NULL)); //seed random

    _2 = SDL_MapRGB(screen->format, 238, 228, 218);
    _4 = SDL_MapRGB(screen->format, 237, 224, 200);
    _8 = SDL_MapRGB(screen->format, 242, 177, 121);
    _16 = SDL_MapRGB(screen->format, 245, 149, 99);
    _32 = SDL_MapRGB(screen->format, 246, 124, 95);
    _64 = SDL_MapRGB(screen->format, 246, 93, 59);
    _128 = SDL_MapRGB(screen->format, 237, 206, 113);
    _256 = SDL_MapRGB(screen->format, 237, 204, 97);
    _512 = SDL_MapRGB(screen->format, 236, 200, 80);
    _1024 = SDL_MapRGB(screen->format, 237, 197, 63);
    _2048 = SDL_MapRGB(screen->format, 236, 193, 45);
    _other = SDL_MapRGB(screen->format, 255, 30, 32);

    gameover = IMG_Load("gameover.png");
    gameoverb = IMG_Load("gameover-black.png");
}

void quit(void)
{
    if(gameover) SDL_FreeSurface(gameover);
    if(gameoverb) SDL_FreeSurface(gameoverb);
    if(screenbuffer) SDL_FreeSurface(screenbuffer);
    if(screen) SDL_FreeSurface(screen);
    TTF_CloseFont(font);
    TTF_CloseFont(tinyfont);
    TTF_Quit();
    SDL_Quit();
}

void draw_rect_bordered(SDL_Rect rect, Uint32 map)
{
    SDL_Rect hrect = { rect.x + 1, rect.y, rect.w - 2, rect.h };
    SDL_Rect vrect = { rect.x, rect.y + 1, rect.w, rect.h - 2 };
    SDL_FillRect(screenbuffer, &hrect, map);
    SDL_FillRect(screenbuffer, &vrect, map);
}

void draw_table()
{
    int x = pad;
    int y = pad;
    int w = HEIGHT - (pad * 2);
    int h = HEIGHT - (pad * 2);
    SDL_Rect outlineRect = { x - 2, y - 2, w + 4, h + 4 };
    SDL_FillRect(screenbuffer, &outlineRect, SDL_MapRGB(screenbuffer->format, 187, 173, 160));
    SDL_Rect rect = { x, y, w, h };
    SDL_FillRect(screenbuffer, &rect, SDL_MapRGB(screenbuffer->format, 204, 192, 179));
    for (int i = 0; i < 4; i++) {
        SDL_Rect hline = { x, y + ((h / 4) * i) - 1, w, 2 };
        SDL_FillRect(screenbuffer, &hline, SDL_MapRGB(screenbuffer->format, 187, 173, 160));
        SDL_Rect vline = { x + ((w / 4) * i) - 1, y, 2, h };
        SDL_FillRect(screenbuffer, &vline, SDL_MapRGB(screenbuffer->format, 187, 173, 160));
    }
}

void draw_tile(int col, int row, int num)
{
    static SDL_Color black={0x10, 0x10, 0x10, 0};
    int x = pad;
    int y = pad;
    int w = HEIGHT - (pad * 2);
    int h = HEIGHT - (pad * 2);
    x += w / 4 * col;
    y += h / 4 * row;

    int tile_pad = 2;
    int color;
    switch (num) {
    case 2:
        color = _2;
        break;
    case 4:
        color = _4;
        break;
    case 8:
        color = _8;
        break;
    case 16:
        color = _16;
        break;
    case 32:
        color = _32;
        break;
    case 64:
        color = _64;
        break;
    case 128:
        color = _128;
        break;
    case 256:
        color = _256;
        break;
    case 512:
        color = _512;
        break;
    case 1024:
        color = _1024;
        break;
    case 2048:
        color = _2048;
        break;
    default:
        color = _other;
    }
    SDL_Rect rect = { x + tile_pad, y + tile_pad, w / 4 - (tile_pad * 2), h / 4 - (tile_pad * 2) };
    draw_rect_bordered(rect, color);
    char s[8];
	sprintf(s, "%d", num);
    draw_string(screenbuffer, font, rect, s, black, ALIGN_CENTER);
}

bool lost()
{
    bool lost = true;
    for (int i = 0; i < 16; i++) {
        if (tiles[i] == 0) {
            lost = false;
        }
    }
    return lost;
}

void add_tile()
{
    int value = 2;
    if ((rand() % 8) == 0) {
        value = 4;
    }
    int tile = 0;
    do {
        tile = rand() % 16;
    } while (tiles[tile] != 0);
    tiles[tile] = value;
}

void reset()
{
    for (int i = 0; i < 16; i++) {
        tiles[i] = 0;
    }
    score = 0;
    add_tile();
    add_tile();
}

void draw_tiles()
{
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            int tile = tiles[i * 4 + j];
            if (tile != 0) {
                draw_tile(j, i, tile);
            }
        }
    }
}

int* fill_zeros(int line[4])
{
Label : {
    for (int i = 0; i < 4; i++) {
        int tile = line[i];
        int next_tile = line[i + 1];
        if (i < 3 && tile == 0 && next_tile != 0) {
            line[i] = next_tile;
            line[i + 1] = 0;
            changed = true;
            goto Label;
        }
    }
}
    return line;
}
int* merge_line(int line[4])
{
    //fill zeros
    line = fill_zeros(line);
    int oldtile = 0;
    for (int i = 0; i < 4; i++) {
        int tile = line[i];
        if (oldtile != 0 && oldtile == tile) {
            line[i - 1] = oldtile * 2;
            line[i] = 0;
            tile = 0;
            changed = true;
            score += oldtile * 2;
        }
        oldtile = tile;
    }
    //fill zeros again
    line = fill_zeros(line);
    if(gMerge) Mix_PlayChannel(1, gMerge, 0);
    return line;
}
void left()
{
    int tiles_backup[16];
    for (int i = 0; i < 16; i++) {
        tiles_backup[i] = tiles[i];
    }
    lastscore = score;
    changed = false;
    for (int i = 0; i < 4; i++) {
        int line[4];
        for (int j = 0; j < 4; j++) {
            line[j] = tiles[i * 4 + j];
        }
        int* merged = merge_line(line);
        for (int k = 0; k < 4; k++) {
            int mtile = merged[k];
            tiles[i * 4 + k] = mtile;
        }
    }

    if (!lost() && changed) {
		memcpy(last, tiles_backup, sizeof(last));
        add_tile();
    }
}
void right()
{
    int tiles_backup[16];
    for (int i = 0; i < 16; i++) {
        tiles_backup[i] = tiles[i];
    }
    lastscore = score;
    changed = false;
    for (int i = 0; i < 4; i++) {
        int line[4];
        for (int j = 0; j < 4; j++) {
            line[j] = tiles[i * 4 + (3 - j)];
        }
        int* merged = merge_line(line);
        for (int k = 0; k < 4; k++) {
            int mtile = merged[k];
            tiles[i * 4 + (3 - k)] = mtile;
        }
    }

    if (!lost() && changed) {
        memcpy(last, tiles_backup, sizeof(last));
        add_tile();
    }
}
void up()
{
    int tiles_backup[16];
    for (int i = 0; i < 16; i++) {
        tiles_backup[i] = tiles[i];
    }
    lastscore = score;
    changed = false;
    for (int j = 0; j < 4; j++) {
        int line[4];
        for (int i = 0; i < 4; i++) {
            line[i] = tiles[i * 4 + j];
        }
        int* merged = merge_line(line);
        for (int k = 0; k < 4; k++) {
            int mtile = merged[k];
            tiles[k * 4 + j] = mtile;
        }
    }

    if (!lost() && changed) {
        memcpy(last, tiles_backup, sizeof(last));
        add_tile();
    }
}
void down()
{
    int tiles_backup[16];
    for (int i = 0; i < 16; i++) {
        tiles_backup[i] = tiles[i];
    }
    lastscore = score;
    changed = false;
    for (int j = 0; j < 4; j++) {
        int line[4];
        for (int i = 0; i < 4; i++) {
            line[i] = tiles[(12 - i * 4) + j];
        }
        int* merged = merge_line(line);
        for (int k = 0; k < 4; k++) {
            int mtile = merged[k];
            tiles[(12 - k * 4) + j] = mtile;
        }
    }

    if (!lost() && changed) {
        memcpy(last, tiles_backup, sizeof(last));
        add_tile();
    }
}
void draw_bg()
{
    SDL_FillRect(screenbuffer, NULL, SDL_MapRGB(screenbuffer->format, 0, 0, 0)); //black background
}

void draw_gameover(){
    if(gameoverb && gameover){
        int tile_pad = 2;
        SDL_Rect rect, srect;
        rect.x = (WIDTH - gameoverb->w)/2;
        rect.y = (HEIGHT - gameoverb->h)/2;
        SDL_BlitSurface(gameover, NULL, screenbuffer, &rect);
        srect.x = 0;
        srect.y = 0;
        srect.w = HEIGHT - pad + 2 * tile_pad;
        srect.h = gameoverb->h;
        SDL_BlitSurface(gameoverb, &srect, screenbuffer, &rect);
    }
}

char buf[32]={0};
void draw_score(int score)
{
    static SDL_Color white={0xF0, 0xF0, 0xF0, 0};
    SDL_Rect rect;
    rect.x = 0;
    rect.y = 0;
    rect.w = WIDTH;    
    rect.h = TTF_FontHeight(font);
    snprintf(buf, sizeof(buf), "%d", score);
    draw_string(screenbuffer, font, rect, buf, white, ALIGN_RIGHT);
    rect.y += rect.h;
    draw_string(screenbuffer, font, rect, "v1.2", white, ALIGN_RIGHT);
}

void handle_keydown(SDLKey key)
{
    switch (key) {
    case SDLK_8:
    case SDLK_UP:
        up();
        break;
    case SDLK_2:
    case SDLK_DOWN:
        down();
        break;
    case SDLK_4:
    case SDLK_LEFT:
        left();
        break;
    case SDLK_6:
    case SDLK_RIGHT:
        right();
        break;
    case SDLK_ESCAPE:
        done = SDL_TRUE;
        break;
    case SDLK_BACKSPACE:
        reset();
        break;
    case SDLK_LSHIFT:
        //undo
        memcpy(tiles, last, sizeof(tiles));
        score = lastscore;
        break;
    default:
        break;
    }
}
int main(void)
{
    init();
    add_tile();
    add_tile();
    if( Mix_OpenAudio(DEFAULT_BGM_FREQ, DEFAULT_BGM_FORMAT, DEFAULT_BGM_CHAN, DEFAULT_BGM_CHUNK) < 0){
        printf("open audio error: %s\n", Mix_GetError());
    }
    
    gMerge = Mix_LoadWAV(MERGE_MUSIC);
    gWin = Mix_LoadMUS(WIN_MUSIC);
    gGameover = Mix_LoadMUS(GAMEOVER_MUSIC);
    gBGMMusic = Mix_LoadMUS(BGM_MUSIC);

    if(gBGMMusic == NULL){
        printf("load %s = %p error: %s\n", GAMEOVER_MUSIC, gBGMMusic, Mix_GetError());
    }

    if(gBGMMusic) {
        if(Mix_PlayMusic(gBGMMusic, -1) < 0){
            printf("play %s error: %s\n", GAMEOVER_MUSIC, Mix_GetError());
        }
    }

    while (!done) {
        SDL_Event event;
        draw_bg();
        draw_table();
        draw_tiles();
        draw_score(score);
        if(lost()) {
            if(gGameover) Mix_PlayMusic(gGameover, 1);
            draw_gameover();
        }
        SDL_BlitSurface(screenbuffer, NULL, screen, NULL);
        SDL_Flip(screen);
        SDL_WaitEvent(&event);
        switch (event.type) {
        case SDL_KEYDOWN:
            handle_keydown(event.key.keysym.sym);
            break;
        default:
            break;
        }
    }
    
    if(gBGMMusic) Mix_FreeMusic(gBGMMusic);
    if(gMerge) Mix_FreeChunk(gMerge);
    if(gWin) Mix_FreeMusic(gWin);
    if(gGameover) Mix_FreeMusic(gGameover);

    Mix_CloseAudio();

    quit();
    return EXIT_SUCCESS;
}
