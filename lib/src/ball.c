#include <SDL.h>
#include "paddle_data.h"
#include "ball.h"
#include <SDL_image.h>
#include <stdbool.h> 

#define WINDOW_WIDTH 1300
#define WINDOW_HEIGHT 800
#define MIDDLE_FIELD 440
#define GOAL_TOP 0
#define GOAL_BOTTOM 800
#define BALL_SPEED 5
#define BALL_SIZE 10

struct ball {
    SDL_Texture *ballTexture;
    SDL_Rect ballRect;
    SDL_Surface *ballSurface;
    float velocityY, velocityX;
};

Ball *createBall(SDL_Renderer *renderer) {
    Ball *pBall = malloc(sizeof(Ball)); //Allokera minne till boll
    if (!pBall){
        fprintf(stderr, "Failed to allocate memory for pBall.\n");
        return NULL;
    }

    pBall->ballSurface = IMG_Load("C:/Users/ahmed/game/lib/resources/ball2.png"); // Surface med bild
    if (!pBall->ballSurface) { //Om bild inte lyckas skapas --> felmeddelande, släpp minne och returnera NULL
        fprintf(stderr, "Error loading pBall texture: %s\n", SDL_GetError());
        free(pBall);
        return NULL;
    }

    pBall->ballTexture = SDL_CreateTextureFromSurface(renderer, pBall->ballSurface); //Skapa textur från ytan
    SDL_FreeSurface(pBall->ballSurface); //Släpp ytan efter att texturen skapats
    if (!pBall->ballTexture) { //Om texturen inte lyckas skapas --> felmeddelande, släpp minne och returnera NULL
        fprintf(stderr, "Error creating pBall texture: %s\n", SDL_GetError());
        free(pBall);
        return NULL;
    }

    pBall->ballRect.w = 10; //Sätt storlek på rektangeln
    pBall->ballRect.h = 10;
    pBall->ballRect.x = WINDOW_WIDTH / 2 - pBall->ballRect.w / 2; //Sätt position på rektangeln
    pBall->ballRect.y = MIDDLE_FIELD - pBall->ballRect.h / 2;
    pBall->velocityY = BALL_SPEED; 
    pBall->velocityX = BALL_SPEED; //Sätt hastighet på bollen

    return pBall;
}

void updateBallPosition(Ball *pBall) {
    pBall->ballRect.x += pBall->velocityX / 60;
    pBall->ballRect.y += pBall->velocityY / 60; //Uppdatera position på bollen
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

bool checkGoal(Ball *pBall){
    SDL_Rect ballRect = getBallRect(pBall); //Hämta rektangel för bollen
    if(ballRect.x < 0 || ballRect.x > WINDOW_WIDTH - BALL_SIZE){
        return true;
    }
    return false; //Inga mål
}

bool goalScored(Ball *pBall){
    SDL_Rect ballRect = getBallRect(pBall); //Hämta rektangel för bollen
    
    if (ballRect.x < 0) {
        printf("Goal on left side!\n");
        setBallX(pBall, WINDOW_WIDTH / 2 - BALL_SIZE / 2); // Återställ till mitten
        setBallY(pBall, WINDOW_HEIGHT / 2 - BALL_SIZE / 2);
        setBallVelocity(pBall, BALL_SPEED, BALL_SPEED); // Sätt konstant hastighet
        return true; // Mål på vänster sida
    }
    // Om bollen träffar höger mål
    if (ballRect.x > WINDOW_WIDTH - BALL_SIZE) {
        printf("Goal on right side!\n");
        setBallX(pBall, WINDOW_WIDTH / 2 - BALL_SIZE / 2); // Återställ till mitten
        setBallY(pBall, WINDOW_HEIGHT / 2 - BALL_SIZE / 2);
        setBallVelocity(pBall, -BALL_SPEED, BALL_SPEED); // Sätt konstant hastighet
        return true; // Mål på höger sida
    }

    return false; // Inga mål
}

void updateBallWithRecievedData(Ball *pBall, BallData *pBallData){
    pBall->velocityX = pBallData->velocityX; //Uppdatera hastighet på bollen
    pBall->velocityY = pBallData->velocityY;
    pBall->ballRect.x = pBallData->positionX; //Uppdatera position på bollen
    pBall->ballRect.y = pBallData->positionY;
}

void sendBallData(Ball *pBall, BallData *pBallData){
    pBallData->velocityX = pBall->velocityX; //Skicka hastighet på bollen
    pBallData->velocityY = pBall->velocityY;
    pBallData->positionX = pBall->ballRect.x; //Skicka position på bollen
    pBallData->positionY = pBall->ballRect.y;
}

void restrictBallWithinWindow(Ball *pBall){
    SDL_Rect ballRect = getBallRect(pBall); //Hämta rektangel för bollen
    
    // Om bollen träffar vänster vägg
    if (ballRect.x < 0) {
        setBallX(pBall, 0); // Flytta bollen utanför väggen
        setBallVelocity(pBall, -pBall->velocityX, pBall->velocityY); // Vänd X-riktning
    }

    // Om bollen träffar höger vägg
    if (ballRect.x + BALL_SIZE > WINDOW_WIDTH) {
        setBallX(pBall, WINDOW_WIDTH - BALL_SIZE); // Flytta bollen utanför väggen
        setBallVelocity(pBall, -pBall->velocityX, pBall->velocityY); // Vänd X-riktning
    }

    // Om bollen träffar taket
    if (ballRect.y < 0) {
        setBallY(pBall, 0); // Flytta bollen utanför taket
        setBallVelocity(pBall, pBall->velocityX, -pBall->velocityY); // Vänd Y-riktning
    }

    // Om bollen träffar golvet
    if (ballRect.y + BALL_SIZE > WINDOW_HEIGHT) {
        setBallY(pBall, WINDOW_HEIGHT - BALL_SIZE); // Flytta bollen utanför golvet
        setBallVelocity(pBall, pBall->velocityX, -pBall->velocityY); // Vänd Y-riktning
    }
}

void handlePaddleBallCollision(SDL_Rect paddleRect, SDL_Rect ballRect, Ball *pBall){
    if(checkCollision(paddleRect, ballRect)){
        pBall->velocityX = -pBall->velocityX; //Om bollen träffar paddeln --> vänd riktning
    }
    updateBallPosition(pBall); //Uppdatera position på bollen
}