#ifndef paddle_data_h
#define paddle_data_h

#define WINDOW_WIDTH 1300
#define WINDOW_HEIGHT 800
#define MAX_PADDLES 4
<<<<<<< HEAD
#define MAX_OBSTACLES 4
=======
#define MAXOBSTACLES 5
>>>>>>> 05b5cd4f38921a10c86dc01d9e90af2c9f13314f

#include <stdbool.h>

enum gameState{START, ONGOING, GAME_OVER};
typedef enum gameState GameState;

enum clientCommand{READY, UP, DOWN, LEFT, RIGHT, RESET_VELOCITY_X, RESET_VELOCITY_Y, RESTRICT_PLAYER};
typedef enum clientCommand ClientCommand;

typedef enum{
    SHAPE_RECTANGLE,
    SHAPE_CIRCLE,
    SHAPE_TRIANGLE
} ShapeType;

typedef struct {
    ShapeType type;
    float x, y, width, height;
    Uint8 r, g, b, a;
    bool active;
} ObstacleData;

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
    ObstacleData obstacles[MAX_OBSTACLES];
    int clientNr;
    GameState gState;
    bool connected[MAX_PADDLES];
    int teamScores[2];
    bool hostConnected;
} ServerData;

#endif