#include <stdlib.h>
#include <time.h>
#include <SDL.h>
#include "obstacles.h"
#include "ball.h"
#include "paddle_data.h"

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
