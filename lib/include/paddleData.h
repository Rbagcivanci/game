#ifndef paddle_data_h
#define paddle_data_h

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 960
#define MAX_PLAYERS 2

enum gameState{START, ONGOING, GAME_OVER};
typedef enum gameState GameState;

enum clientCommand{READY, UP, DOWN};
typedef enum clientCommand ClientCommand;

typedef struct clientData{
    ClientCommand command;
    int clientNumber;
} clientData;

typedef struct ballData{
    float velocityY, velocityX;
    int positionX, positionY;
} BallData;

typedef struct paddleData{
    float velocityY;
    int positionX, positionY;
} PaddleData;

typedef struct serverData{
    PaddleData paddles[MAX_PLAYERS];
    BallData ball;
    int clientNr;
    GameState gState;
    bool connected[MAX_PLAYERS];
} serverData;

#endif