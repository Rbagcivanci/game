<<<<<<< HEAD
#include <stdlib.h>
#include <time.h>
#include <SDL.h>
=======
#include <SDL.h>
#include <SDL_image.h>
#include <stdlib.h>
#include <math.h>
>>>>>>> 05b5cd4f38921a10c86dc01d9e90af2c9f13314f
#include "obstacles.h"
#include "ball.h"
#include "paddle_data.h"

<<<<<<< HEAD
static Obstacle obstacles[MAX_OBSTACLES];

void init_obstacles(void)
{
    srand(time(NULL));
    for(int i = 0; i < MAX_OBSTACLES; i++)
    {
        obstacles[i].active = false;
    }
}

void spawn_obstacles(void)
{
    for(int i = 0; i < MAX_OBSTACLES; i++)
    {
        Obstacle *obs = &obstacles[i];
        obs -> type = rand() % 3;
        obs -> x = rand() % (WINDOW_WIDTH) + 5;
        obs -> y = rand() % (WINDOW_HEIGHT - 30) + 25;
        obs -> width = 50 + (rand() % 10);
        obs -> height = 50 + (rand() % 10);
        obs -> color =(SDL_Color){rand() % 255, rand() % 255, rand() % 255, 255};
        obs -> active = true;
    }
}

void render_obstacles(SDL_Renderer *renderer) {
    for (int i = 0; i < MAX_OBSTACLES; i++) {
        Obstacle *obs = &obstacles[i];
        if (!obs->active) continue;

        SDL_SetRenderDrawColor(renderer, obs->color.r, obs->color.g, obs->color.b, obs->color.a);

        switch (obs->type) {
            case SHAPE_RECTANGLE: {
                SDL_Rect rect = {(int)(obs->x - obs->width / 2), (int)(obs->y - obs->height / 2),
                                 (int)obs->width, (int)obs->height};
                SDL_RenderFillRect(renderer, &rect);
                break;
            }
            case SHAPE_CIRCLE: {
                int radius = (int)(obs->width / 2);
                for (int w = -radius; w <= radius; w++) {
                    for (int h = -radius; h <= radius; h++) {
                        if (w * w + h * h <= radius * radius) {
                            SDL_RenderDrawPoint(renderer, (int)obs->x + w, (int)obs->y + h);
                        }
                    }
                }
                break;
            }
        }
    }
}

void check_obstacle_collisions(Ball *ball) 
{
    SDL_Rect ballRect = getBallRect(ball);
    for (int i = 0; i < MAX_OBSTACLES; i++) {
        Obstacle *obs = &obstacles[i];
        if (!obs->active) continue;

        if (obs->type == SHAPE_RECTANGLE) {
            SDL_Rect rect = {(int)(obs->x - obs->width / 2), (int)(obs->y - obs->height / 2),
                             (int)obs->width, (int)obs->height};
            if (checkCollision(rect, ballRect)) {
                // Simple bounce: reverse velocity based on collision side
                float dx = (ballRect.x + ballRect.w / 2) - obs->x;
                float dy = (ballRect.y + ballRect.h / 2) - obs->y;
                if (fabs(dx) > fabs(dy)) {
                    ball->velocityX = -ball->velocityX;
                } else {
                    ball->velocityY = -ball->velocityY;
                }
            }
        } 

        }
}


void reset_obstacles(void) {
    for (int i = 0; i < MAX_OBSTACLES; i++) {
        obstacles[i].active = false;
    }
}

void send_obstacle_data(ObstacleData *data) {
    for (int i = 0; i < MAX_OBSTACLES; i++) {
        data[i].type = obstacles[i].type;
        data[i].x = obstacles[i].x;
        data[i].y = obstacles[i].y;
        data[i].width = obstacles[i].width;
        data[i].height = obstacles[i].height;
        data[i].r = obstacles[i].color.r;
        data[i].g = obstacles[i].color.g;
        data[i].b = obstacles[i].color.b;
        data[i].a = obstacles[i].color.a;
        data[i].active = obstacles[i].active;
    }
}

void update_obstacles_with_received_data(ObstacleData *data) 
{
    for (int i = 0; i < MAX_OBSTACLES; i++) 
    {
        obstacles[i].type = data[i].type;
        obstacles[i].x = data[i].x;
        obstacles[i].y = data[i].y;
        obstacles[i].width = data[i].width;
        obstacles[i].height = data[i].height;
        obstacles[i].color = (SDL_Color){data[i].r, data[i].g, data[i].b, data[i].a};
        obstacles[i].active = data[i].active;
    }
}
=======
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





>>>>>>> 05b5cd4f38921a10c86dc01d9e90af2c9f13314f
