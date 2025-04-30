#ifndef Paddle_h
#define Paddle_h

typedef struct paddle Paddle;
typedef struct paddleData PaddleData;

Paddle *createPaddle(SDL_Renderer *pGameRenderer, int w, int h, int paddleIndex);
void setPaddlePosition(Paddle *pPaddle, int x, int y);
void destroyPaddle(Paddle *pPaddle);
void updatePaddleVelocity(Paddle *pPaddle, float vx, float vy);
SDL_Texture *getPaddleTexture(Paddle *pPaddle);
SDL_Rect getPaddleRect(Paddle *pPaddle);
void updatePaddleVUp(Paddle *pPaddle);
void updatePaddleVDown(Paddle *pPaddle);
void updatePaddleVLeft(Paddle *pPaddle);
void updatePaddleVRight(Paddle *pPaddle);
void updatePaddlePosition(Paddle *pPaddle);
void setStartingPosition(Paddle *pPaddle, int paddleIndex, int w, int h);
void resetPaddleSpeed(Paddle *pPaddle, int x, int y);
int getPaddleSpeedY(Paddle *pPaddle);
int getPaddleSpeedX(Paddle *pPaddle);
void restrictPaddleWithinWindow(Paddle *pPaddle, int w, int h);
void handlePaddleCollision(Paddle *pPaddle1, Paddle *pPaddle2);
void getPaddleSendData(Paddle *pPaddle, PaddleData *pPaddleData);
void updatePaddleWithRecievedData(Paddle *pPaddle, PaddleData *pPaddleData);
void drawPaddle(Paddle *pPaddle);

#endif // Paddle_h