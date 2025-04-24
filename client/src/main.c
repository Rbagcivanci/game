#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <SDL_net.h>
#include <SDL_mixer.h>
#include <SDL_timer.h>

#include "paddle.h"
#include "ball.h"
#include "paddle_data.h"
#include "text.h"

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 960
#define PADDLE_SPEED 5

typedef struct game {
    SDL_Window *pWindow;
    SDL_Renderer *pRenderer;
    SDL_Surface *pBackgroundSurface, *pIpSurface;
    SDL_Texture *pBackgroundTexture, *pIpTexture;
    TTF_Font *pFont;

    Text *pLobbyText, *pEnterIpText, *pIpText;
    Paddle *pPaddle[MAX_PADDLES];
    Ball *pBall;
    Gamestate state;

    int teamA;
    int teamB;
    int nrOfPaddles, paddleNr;
}Game;

int initiate(Game *pGame);
void run();
void closeGame();

int main(int argc, char *argv[]) {
    if (initiate() != 0) {
        return -1;
    }
    run();
    closeGame();
    return 0;
}

int initiate(Game *pGame){
    if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER) != 0){
        printf("Error: %s\n",SDL_GetError());
        return 0;
    }

    if(TTF_Init() != 0){
        printf("Error: %s\n",TTF_GetError());
        SDL_Quit();
        return 0;
    }
    if(SDLNet_Init() != 0){
        printf("Error: %s\n",SDLNet_GetError());
        TTF_Quit();
        SDL_Quit();
        return 0;
    }
    
    pGame->pWindow = SDL_CreateWindow("Pong Client", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if(!pGame->pWindow){
        printf("Error: %s\n",SDL_GetError());
        TTF_Quit();
        SDL_Quit();
        return 0;
    }
}
