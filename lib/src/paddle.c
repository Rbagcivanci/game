#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdlib.h>
#include <stdio.h>
#include "paddle.h"
#include "ball.h"
#include "paddle_data.h"

#define PADDLE_HEIGHT 60
#define PADDLE_WIDTH 10
#define PADDLE_SPEED 5

struct paddle{
    float velocityY;
    int positionX, positionY, team;
    Ball *pBall;
    SDL_Renderer *paddleRenderer;    
    SDL_Texture *paddleTexture;
    SDL_Rect paddleRect; 
};

Paddle *createPaddle(int number, SDL_Renderer *pRenderer, int window_width, int window_height){
    Paddle *pPaddle = malloc(sizeof(Paddle));
    
    if(!pPaddle){
        printf("Error: %s\n",SDL_GetError());
        return NULL;
    }

    pPaddle->paddleRect.w = PADDLE_WIDTH;
    pPaddle->paddleRect.h = PADDLE_HEIGHT;
    setStartingPosition(pPaddle, playerIndex, w, h);

    SDL_Surface *paddleSurface = IMG_Load("");
    if(!paddleSurface){
        printf("Error: %s\n",IMG_GetError());
        free(pPaddle);
        return NULL;
    }

    pPaddle->paddleTexture = SDL_CreateTextureFromSurface();
    if(!pPaddle->paddleTexture){
        printf("Error: %s\n",SDL_GetError());
        SDL_FreeSurface(paddleSurface);
        free(pPaddle);
        return NULL;
    }
    
    return pPaddle;
}

SDL_Rect getPaddleRect(Paddle *pPaddle){
    return pPaddle->paddleRect;
}

SDL_Texture *getPaddleTexture(Paddle *pPaddle){
    return pPaddle->paddleTexture;
}

void updatePaddleVelocity(Paddle *pPaddle, float velocityY){
    pPaddle->velocityY = velocityY;
}

void updatePaddleVelocityUp(Paddle *pPaddle){
    pPaddle->velocityY = -PADDLE_SPEED;
}
void updatePaddleVelocityDown(Paddle *pPaddle){
    pPaddle->velocityY = PADDLE_SPEED;
}

int getPaddleSpeedY(Paddle *pPaddle){
    return pPaddle->velocityY != 0;
}

void setPaddlePosition(Paddle *pPaddle, int x, int y){
    pPaddle->paddleRect.x = x;
    pPaddle->paddleRect.y = y;
}

void updatePaddlePos(Paddle *pPaddle, float t){
    int newX= pPaddle->paddleRect.x + pPaddle->velocityY * t;
    int newY= pPaddle->paddleRect.y + pPaddle->velocityY * t;
    setPaddlePosition(pPaddle, newX, newY);
}

void setStartingPosition(Paddle *pPaddle, int playerIndex, int w, int h){
    switch(playerIndex){
        case 0:
            pPaddle->paddleRect.x = w/4 - pPaddle->paddleRect.w/2;
            pPaddle->paddleRect.y = h/2;
            break;
        case 1:
            pPaddle->paddleRect.x = w/4 - pPaddle->paddleRect.w/2 + w/2;
            pPaddle->paddleRect.y = h/2
            break;
        case 2:
            pPaddle->paddleRect.x = w/5 - pPaddle->paddleRect.w/2;
            pPaddle->paddleRect.y = h/3;
            break;
        case 3:
            pPaddle->paddleRect.x = w- (w/5);
            pPaddle->paddleRect.y = h/3;
            break;
    }
}

void destroyPaddle(Paddle *pPaddle){
    if(pPaddle){
        SDL_DestroyTexture(pPaddle->paddleTexture);
        free(pPaddle);
    }
}

void handlePaddleCollision(Paddle *pPaddle1, Paddle *pPaddle2){
    SDL_Rect rect1 = getPaddleRect(pPaddle1);
    SDL_Rect rect2 = getPaddleRect(pPaddle2);

    if(checkCollsion(rect1, rect2)){

    }
}

void getPlayerSendData(Paddle *pPaddle, PaddleData *pData){
    pPaddleData->velocityY = pPaddle->velocityY;
    pPaddleData->positionX = pPaddle->paddleRect.x;
    pPaddleData->positionY = pPaddle->paddleRect.y;
}

void updatePlayerWithRecievedData(Paddle *pPaddle, PaddleData *pData){
    pPaddle->velocityY = pData->velocityY;
    pPaddle->paddleRect.x = pData->positionX;
    pPaddle->paddleRect.y = pData->positionY;
}