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