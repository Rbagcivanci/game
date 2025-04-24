#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include "ball.h"

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 960
#define BALL_SPEED 5
#define BALL_SIZE 10

struct ball {
    SDL_Texture *ballTexture;
    SDL_Rect ballRect;
    SDL_Surface *ballSurface;
    float velocityX, velocityY;
};

Ball *createBall(SDL_Renderer *renderer) {
    Ball *pBall = malloc(sizeof(Ball)); //Allokera minne till boll
    if (!pBall){
        printf("Error: %s\n", SDL_GetError());
        return NULL;
    }

    pBall->surface = IMG_Load("../lib/resources/ball.png"); // Surface med bild
    if (!pBall->ballSurface) { //Om bild inte lyckas skapas --> felmeddelande, släpp minne och returnera NULL
        printf("Error: %s\n", SDL_GetError());
        free(pBall);
        return NULL;
    }

    pBall->ballTexture = SDL_CreateTextureFromSurface(renderer, pBall->ballSurface); //Skapa textur från ytan
    SDL_FreeSurface(pBall->ballSurface); //Släpp ytan efter att texturen skapats
    if (!pBall->ballTexture) { //Om texturen inte lyckas skapas --> felmeddelande, släpp minne och returnera NULL
        printf("Error: %s\n", SDL_GetError());
        free(pBall);
        return NULL;
    }

    pBall->ballRect.w = 10; //Sätt storlek på rektangeln
    pBall->ballRect.h = 10;
    pBall->ballRect.x = pBall->ballRect.w / 2; //Sätt position på rektangeln
    pBall->ballRect.y = pBall->ballRect.h / 2;
    pBall->velocityX = BALL_SPEED + rand() % (2+1); //Slumptal mellan 5 och 7 på hastighet
    pBall->velocityY = BALL_SPEED + rand() % (2+1); 

    return pBall;
}

SDL_Texture *getBallTexture(Ball *pBall) {
    return pBall->ballTexture;
}

SDL_Rect getBallRect(Ball *pBall) {
    return pBall->ballRect;
}

void setBallVelocity(Ball *pBall, float velocityX, float velocityY) {
    pBall->velocityX = velocityX; //Sätt hastighet på bollen
    pBall->velocityY = velocityY;
}

void setBallX(Ball *pBall, int x) {
    pBall->ballRect.x = x; //Sätt position på bollen
}

void setBallY(Ball *pBall, int y) {
    pBall->ballRect.y = y; //Sätt position på bollen
}

void destroyBall(Ball *pBall){
    SDL_DestroyTexture(pBall->ballTexture); //Släpp texturen
    free(pBall); //Släpp minnet
}

int checkCollision(SDL_Rect rect1, SDL_Rect rect2){
    return SDL_HasIntersection(&rect1, &rect2); //Kolla om boll kolliderar med annat
}

int checkGoal(Ball *pBall){
    SDL_Rect ballRect = getBallRect(pBall); //Hämta rektangel för bollen
    if(ballRect.x < 0 || ballRect.x > WINDOW_WIDTH - BALL_SIZE){
        return 1;
    }
    return 0; //Inga mål
}

int goalScored(Ball *pBall){
    SDL_Rect ballRect = getBallRect(pBall); //Hämta rektangel för bollen
    if(ballRect.x < 0){
        setBallX(pBall, WINDOW_WIDTH / 2 - BALL_SIZE / 2); //Sätt bollen i mitten av skärmen
        setBallY(pBall, WINDOW_HEIGHT / 2 - BALL_SIZE / 2); //Sätt bollen i mitten av skärmen
        setBallVelocity(pBall, BALL_SPEED, BALL_SPEED); //Sätt hastighet på bollen
        return 0; //Vänster mål
    } 
    else if (ballRect.x > WINDOW_WIDTH - BALL_SIZE){
        setBallX(pBall, WINDOW_WIDTH / 2 - BALL_SIZE / 2); //Sätt bollen i mitten av skärmen
        setBallY(pBall, WINDOW_HEIGHT / 2 - BALL_SIZE / 2); //Sätt bollen i mitten av skärmen
        setBallVelocity(pBall, BALL_SPEED, BALL_SPEED); //Sätt hastighet på bollen
        return 1; //Höger mål
    }
}