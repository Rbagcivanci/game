#include <SDL.h>
#include <SDL_image.h>
#include "paddle.h"
#include "paddle_data.h"
#include "ball.h"
#include <stdlib.h>
#include <stdio.h>

#define WEST_PADDLE_BORDER 0
#define EAST_PADDLE_BORDER 1300
#define NORTH_PADDLE_BORDER 0
#define SOUTH_PADDLE_BORDER 800

#define MOVEMENT_SPEED 300

struct paddle {
    float velocityX, velocityY;
    float xPos, yPos;
    int team;
    Ball *pBall;
    SDL_Renderer *pRenderer;
    SDL_Texture *paddleTexture;
    SDL_Rect paddleRect;
};

Paddle *createPaddle(SDL_Renderer *pGameRenderer, int w, int h, int paddleIndex) {
    Paddle *pPaddle = malloc(sizeof(Paddle));
    if (!pPaddle) {
        fprintf(stderr, "Memory allocation failed for Paddle\n");
        return NULL;
    }
    pPaddle->paddleRect.w = 10;
    pPaddle->paddleRect.h = 100;
    setStartingPosition(pPaddle, paddleIndex, w, h);
    
    SDL_Surface *paddleSurface = IMG_Load("../lib/resources/ball2.png");
    if (!paddleSurface) {
        fprintf(stderr, "Failed to load Paddle image: %s\n", IMG_GetError());
        free(pPaddle);
        return NULL;
    }

    pPaddle->paddleTexture = SDL_CreateTextureFromSurface(pGameRenderer, paddleSurface);
    SDL_FreeSurface(paddleSurface);
    if (!pPaddle->paddleTexture) {
        fprintf(stderr, "Error creating texture: %s\n", SDL_GetError());
        free(pPaddle);
        return NULL;
    }
    return pPaddle;
}

void updatePaddleVelocity(Paddle *pPaddle, float vx, float vy) {
    pPaddle->velocityX = vx;
    pPaddle->velocityY = vy;
}

SDL_Texture *getPaddleTexture(Paddle *pPaddle) {
    return pPaddle->paddleTexture;
}

SDL_Rect getPaddleRect(Paddle *pPaddle) {
    return pPaddle->paddleRect;
}

void updatePaddleVUp(Paddle *pPaddle) {
    pPaddle->velocityY = -MOVEMENT_SPEED;
}

void updatePaddleVDown(Paddle *pPaddle) {
    pPaddle->velocityY = MOVEMENT_SPEED;
}

void updatePaddleVLeft(Paddle *pPaddle) {
    pPaddle->velocityX = -MOVEMENT_SPEED;
}

void updatePaddleVRight(Paddle *pPaddle) {
    pPaddle->velocityX = MOVEMENT_SPEED;
}

void resetPaddleSpeed(Paddle *pPaddle, int x, int y) {
    if (x == 1) pPaddle->velocityX = 0;
    if (y == 1) pPaddle->velocityY = 0;
}

int getPaddleSpeedY(Paddle *pPaddle) {
    return pPaddle->velocityY != 0;
}

int getPaddleSpeedX(Paddle *pPaddle) {
    return pPaddle->velocityX != 0;
}

void updatePaddlePosition(Paddle *pPaddle, float deltaTime) {
    if(deltaTime <= 0) {
        return;
    }
    pPaddle->xPos += pPaddle->velocityX * deltaTime;
    pPaddle->yPos += pPaddle->velocityY * deltaTime;

    pPaddle->paddleRect.x = (int)pPaddle->xPos;
    pPaddle->paddleRect.y = (int)pPaddle->yPos;

    //float newX = pPaddle->paddleRect.x + (pPaddle->velocityX * deltaTime);
    //float newY = pPaddle->paddleRect.y + (pPaddle->velocityY * deltaTime);
    //setPaddlePosition(pPaddle, (int)newX, (int)newY);
}

void setPaddlePosition(Paddle *pPaddle, int x, int y) {
    pPaddle->xPos = x;
    pPaddle->yPos = y;
    pPaddle->paddleRect.x = x;
    pPaddle->paddleRect.y = y;
}

void setStartingPosition(Paddle *pPaddle, int paddleIndex, int w, int h) {
    pPaddle->velocityX = 0;
    pPaddle->velocityY = 0;
    switch(paddleIndex) {
        case 0: pPaddle->paddleRect.x = 100;
                pPaddle->paddleRect.y = 100;
        break;
        case 1: pPaddle->paddleRect.x = w - pPaddle->paddleRect.w - 100;
                pPaddle->paddleRect.y = 100;
        break;
        case 2: pPaddle->paddleRect.x = 100;
                pPaddle->paddleRect.y = h - pPaddle->paddleRect.h - 100;
        break;
        case 3: pPaddle->paddleRect.x = w - pPaddle->paddleRect.w - 100;
                pPaddle->paddleRect.y = h - pPaddle->paddleRect.h - 100;
        break;
    }

    pPaddle->xPos = pPaddle->paddleRect.x;
    pPaddle->yPos = pPaddle->paddleRect.y;
}

void restrictPaddleWithinWindow(Paddle *pPaddle, int width, int height) {
    if (pPaddle->paddleRect.x < 0) {
        setPaddlePosition(pPaddle, 0, pPaddle->paddleRect.y);
    }
    if (pPaddle->paddleRect.x + pPaddle->paddleRect.w > width) {
        setPaddlePosition(pPaddle, width - pPaddle->paddleRect.w, pPaddle->paddleRect.y);
    }
    if (pPaddle->paddleRect.y < 0) {
        setPaddlePosition(pPaddle, pPaddle->paddleRect.x, 0);
    }
    if (pPaddle->paddleRect.y + pPaddle->paddleRect.h > height) {
        setPaddlePosition(pPaddle, pPaddle->paddleRect.x, height - pPaddle->paddleRect.h);
    }
}

void getPaddleSendData(Paddle *pPaddle, PaddleData *pPaddleData){
    pPaddleData->velocityX = pPaddle->velocityX;
    pPaddleData->velocityY = pPaddle->velocityY;
    pPaddleData->yPos = pPaddle->paddleRect.y;
    pPaddleData->xPos = pPaddle->paddleRect.x;
    //sendBallData(pPaddle->pBall, &(pPaddleData->ball)); //Skicka bollen till servern
}

void updatePaddleWithRecievedData(Paddle *pPaddle, PaddleData *pPaddleData){
    pPaddle->velocityX = pPaddleData->velocityX;
    pPaddle->velocityY = pPaddleData->velocityY;
    pPaddle->paddleRect.y = pPaddleData->yPos;
    pPaddle->paddleRect.x = pPaddleData->xPos;
    //updateBallWithRecievedData(pPaddle->pBall, &(pPaddleData->ball)); //Uppdatera bollen med data från servern
}

void destroyPaddle(Paddle *pPaddle) {
    if (pPaddle != NULL) {
        if (pPaddle->paddleTexture != NULL) {
            SDL_DestroyTexture(pPaddle->paddleTexture);
            pPaddle->paddleTexture = NULL;
        }
        free(pPaddle);
    }
}

void handlePaddleCollision(Paddle *pPaddle1, Paddle *pPaddle2) {
    SDL_Rect rect1 = getPaddleRect(pPaddle1);
    SDL_Rect rect2 = getPaddleRect(pPaddle2);
    
    if (checkCollision(rect1, rect2)) {
        // Calculate overlap in both dimensions
        int overlapX;
        if(rect1.x<rect2.x) overlapX = (rect1.x + rect1.w - rect2.x);
        else overlapX = (rect2.x + rect2.w - rect1.x);

        int overlapY;
        if(rect1.y<rect2.y) 
            overlapY = (rect1.y + rect1.h - rect2.y);
        else 
            overlapY = (rect2.y + rect2.h - rect1.y);

        // Resolve collision based on the lesser overlap
        if (overlapX < overlapY) {
            int shift = overlapX / 2 + 1;  // Added +1 for anti-sticking
            if (rect1.x < rect2.x) {
                setPaddlePosition(pPaddle1, rect1.x - shift, rect1.y);
                setPaddlePosition(pPaddle2, rect2.x + shift, rect2.y);
            } else {
                setPaddlePosition(pPaddle1, rect1.x + shift, rect1.y);
                setPaddlePosition(pPaddle2, rect2.x - shift, rect2.y);
            }
        } else {
            int shift = overlapY / 2 + 1;  // Added +1 for anti-sticking
            if (rect1.y < rect2.y) {
                setPaddlePosition(pPaddle1, rect1.x, rect1.y - shift);
                setPaddlePosition(pPaddle2, rect2.x, rect2.y + shift);
            } else {
                setPaddlePosition(pPaddle1, rect1.x, rect1.y + shift);
                setPaddlePosition(pPaddle2, rect2.x, rect2.y - shift);
            }
        }
    }
}

void drawPaddle(Paddle *pPaddle) {
    if (pPaddle != NULL && pPaddle->paddleTexture != NULL) {
        SDL_RenderCopyEx(pPaddle->pRenderer, pPaddle->paddleTexture, NULL, &(pPaddle->paddleRect), 0, NULL, SDL_FLIP_NONE);
    }
}