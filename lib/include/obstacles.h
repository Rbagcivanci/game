<<<<<<< HEAD
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
=======
#ifndef obstacles_h
#define obstacles_h

typedef struct obstacle Obstacle;
typedef struct obstacleData ObstacleData;

Obstacle *createObstacle(SDL_Renderer *renderer);
void randomizeObstacleLocation(Obstacle *pObstacle, int w, int h);
void setObstaclePosition(Obstacle *pObstacle, int x, int y);
void destroyObstacle(Obstacle *pObstacle);
void updateObstaclePosition(Obstacle *pObstacle);
void sendObstacleData(Obstacle *pObstacle, ObstacleData *pObstacleData);
void updateObstacleWithReceived(Obstacle *pObstacle, ObstacleData *pObstacleData);
SDL_Rect getObstacleRect(Obstacle *pObstacle);
int returnStateObstacle(Obstacle *pObstacle);

#endif //obstacles_h
>>>>>>> 05b5cd4f38921a10c86dc01d9e90af2c9f13314f
