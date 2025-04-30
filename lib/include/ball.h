#ifndef ball_h
#define ball_h

#include <stdbool.h>

typedef struct ball Ball;
typedef struct ballData BallData;

Ball *createBall(SDL_Renderer *renderer);
SDL_Texture *getBallTexture(Ball *pBall);
SDL_Rect getBallRect(Ball *pBall);
void updateBallPosition(Ball *pBall, float deltaTime);
void destroyBall(Ball *pBall);
void setBallVelocity(Ball *pBall, float velocityX, float velocityY);
void setBallX(Ball *pBall, int x);
void setBallY(Ball *pBall, int y);
int checkCollision(SDL_Rect rect1, SDL_Rect rect2);
int checkGoal(Ball *pBall);
int goalScored(Ball *pBall);
void handlePaddleBallCollision(SDL_Rect paddleRect, SDL_Rect ballRect, Ball *pBall);
void restrictBallWithinWindow(Ball *pBall);
void sendBallData(Ball *pBall, BallData *pBallData);
void updateBallWithRecievedData(Ball *pBall, BallData *pBallData);
void serveBall(Ball *pBall, int direction);

#endif // BALL_H