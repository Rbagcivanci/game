#ifndef paddle_data_h
#define paddle_data_h

#define WINDOW_WIDTH 1300
#define WINDOW_HEIGHT 800
#define MAX_PADDLES 4
#define MAXOBSTACLES 5

#include <stdbool.h>

enum gameState{START, ONGOING, GAME_OVER};
typedef enum gameState GameState;

enum clientCommand{READY, UP, DOWN, LEFT, RIGHT, RESET_VELOCITY_X, RESET_VELOCITY_Y, RESTRICT_PLAYER};
typedef enum clientCommand ClientCommand;

typedef struct clientData{
    ClientCommand command;
    int clientNumber;
} ClientData;

typedef struct ballData{
    float velocityY, velocityX;
    float positionX, positionY;
} BallData;

typedef struct paddleData{
    float velocityY, velocityX;
    int yPos, xPos;
} PaddleData;

typedef struct obstacleData{
    float positionX, positionY, velocityX, velocityY, angle;
    bool active;
} ObstacleData;

typedef struct serverData{
    PaddleData paddles[MAX_PADDLES];
    ObstacleData obstacles[MAXOBSTACLES];
    BallData ball;
    int clientNr;
    GameState gState;
    bool connected[MAX_PADDLES];
    int teamScores[2];
    bool hostConnected;
} ServerData;

#endif