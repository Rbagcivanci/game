#ifndef OBSTACLES_H
#define OBSTACLES_H

#include <SDL.h>
#include "paddle_data.h"
#include "ball.h"

#define MAX_OBSTACLES 4

typedef struct
{
    ShapeType type;
    float x, y;
    float width, height;
    SDL_Color color;
    bool active;
} Obstacle;


void init_obstacles(void);
void spawn_obstacles(void);
void render_obstacles(SDL_Renderer *renderer);
void check_obstacle_collisions(Ball *ball);
void reset_obstacles(void);
void send_obstacle_data(ObstacleData *data);
void update_obstacles_with_received_data(ObstacleData *data);
#endif
