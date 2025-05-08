#include <SDL.h>
#include <SDL_image.h>
#include <stdlib.h>
#include <math.h>
#include "obstacles.h"
#include "ball.h"
#include "paddle_data.h"

#define WINDOW_WIDTH 1300
#define WINDOW_HEIGHT 800
#define PADDLE_WIDTH 10
#define PADDLE_HEIGHT 100
#define BALL_SIZE 10
#define OBSTACLESPEED 20
#define MAXOBSTACLES 5

struct obstacle{
    SDL_Texture *texture;
    SDL_Rect obstacleRect;
    float velocityX, velocityY, angle;
    Uint32 spawnTime;
    bool active;
};

Obstacle *createObstacle(SDL_Renderer *renderer){
    Obstacle *pObstacle = malloc(sizeof(Obstacle));
    if(!pObstacle){
        fprintf(stderr, "Failed to allocate memory for obstacle.\n");
        return NULL;
    }

    SDL_Surface *surface = IMG_Load("../lib/resources/ball2.png");
    if(!surface){
        fprintf(stderr, "Failed to load obstacle img: %s\n", IMG_GetError());
        free(pObstacle);
        return NULL;
    }

    pObstacle->texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    if(!pObstacle->texture){
        fprintf(stderr, "Error creating obstacle texture: %s\n", SDL_GetError());
        free(pObstacle);
        return NULL;
    }
    pObstacle->obstacleRect.w = PADDLE_WIDTH;
    pObstacle->obstacleRect.h = PADDLE_HEIGHT;
    pObstacle->velocityX = 0;
    pObstacle->velocityY = 0;
    pObstacle->angle = 0;
    pObstacle->spawnTime = 0;
    pObstacle->active = false;
    
    return pObstacle;
}

SDL_Rect getObstacleRect(Obstacle *pObstacle){
    return pObstacle->obstacleRect;
}

void destroyObstacle(Obstacle *pObstacle){
    if(pObstacle){
        if(pObstacle->texture){
            SDL_DestroyTexture(pObstacle->texture);
        }
        free(pObstacle);
    }
}

void randomizeObstacleLocation(Obstacle *pObstacle, int w, int h){
    pObstacle->obstacleRect.x = w/2 - pObstacle->obstacleRect.w / 2;
    pObstacle->obstacleRect.y = rand()%(h - pObstacle->obstacleRect.h);
    pObstacle->angle = (float) (rand()%360);
    pObstacle->velocityX = ((float) rand()/RAND_MAX -0.5f) * 2 * OBSTACLESPEED;
    pObstacle->velocityY = ((float)rand()/ RAND_MAX - 0.5f) * 2 * OBSTACLESPEED;
    pObstacle->spawnTime = SDL_GetTicks();
    pObstacle->active = true;
}

void setObstaclePosition(Obstacle *pObstacle, int x, int y){
    pObstacle->obstacleRect.x = x;
    pObstacle->obstacleRect.y = y;
}

void updateObstaclePosition(Obstacle *pObstacle){
    if(!pObstacle->active) return;

    pObstacle->obstacleRect.x += pObstacle->velocityX;
    pObstacle->obstacleRect.y += pObstacle->velocityY;

    //Restrict within window

    if(pObstacle->obstacleRect.x < 0){
        pObstacle->obstacleRect.x = 0;
        pObstacle->velocityX = -pObstacle->velocityX;
    }

    if(pObstacle->obstacleRect.x + pObstacle->obstacleRect.w > WINDOW_WIDTH){
        pObstacle->obstacleRect.x = WINDOW_WIDTH - pObstacle->obstacleRect.w;
        pObstacle->velocityX = -pObstacle->velocityX;
    }

    if(pObstacle->obstacleRect.y < 0){
        pObstacle->obstacleRect.y = 0;
        pObstacle->velocityY = -pObstacle->velocityY;
    }

    if(pObstacle->obstacleRect.y + pObstacle->obstacleRect.h > WINDOW_HEIGHT){
        pObstacle->obstacleRect.y = WINDOW_HEIGHT - pObstacle->obstacleRect.h;
        pObstacle->velocityY = -pObstacle->velocityY;
    }
}

int returnStateObstacle(Obstacle *pObstacle){
    if(pObstacle->active){
        return 1;
    }
    return 0;
}

void sendObstacleData(Obstacle *pObstacle, ObstacleData *pObstacleData){
    pObstacleData->positionX = pObstacle->obstacleRect.x;
    pObstacleData->positionY = pObstacle->obstacleRect.y;
    pObstacleData->velocityX = pObstacle->velocityX;
    pObstacleData->velocityY = pObstacle->velocityY;
    pObstacleData->angle = pObstacle->angle;
    pObstacleData->active = pObstacle->active;
}

void updateObstacleWithReceived(Obstacle *pObstacle, ObstacleData *pObstacleData){
    pObstacle->obstacleRect.x= pObstacleData->positionX;
    pObstacle->obstacleRect.y = pObstacleData->positionY;
    pObstacle->velocityX = pObstacleData->velocityX;
    pObstacle->velocityY = pObstacleData->velocityY;
    pObstacle->angle = pObstacleData->angle;
    pObstacle->active = pObstacleData->active; 
}





