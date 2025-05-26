#ifndef paddle_data_h
#define paddle_data_h

#define WINDOW_WIDTH 1300
#define WINDOW_HEIGHT 800
#define MAX_PADDLES 4
#define MAX_OBSTACLES 4

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

typedef struct serverData{
    PaddleData paddles[MAX_PADDLES];
    BallData ball;
    ObstacleData obstacles[MAX_OBSTACLES];
    int clientNr;
    GameState gState;
    bool connected[MAX_PADDLES];
    int teamScores[2];
    bool hostConnected;
} ServerData;

#endif