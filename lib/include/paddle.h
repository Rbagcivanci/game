#ifndef paddle_h
#define paddle_h

typedef struct paddle Paddle;
typedef struct paddleData PaddleData;

Paddle *createPaddle(SDL_Renderer *pRenderer, int window_width, int window_height, int paddleIndex);
SDL_Rect getPaddleRect(Paddle *pPaddle);
SDL_Texture *getPaddleTexture(Paddle *pPaddle);
void updatePaddleVelocity(Paddle *pPaddle, float velocityY);
void updatePaddleVelocityUp(Paddle *pPaddle);
void updatePaddleVelocityDown(Paddle *pPaddle);
int getPaddleSpeedY(Paddle *pPaddle);
void setPaddlePosition(Paddle *pPaddle, int x, int y);
void setStartingPosition(Paddle *pPaddle, int paddleIndex, int w, int h);
void destroyPaddle(Paddle *pPaddle);
void handlePaddleCollision(Paddle *pPaddle1, Paddle *pPaddle2);
void updatePaddlePos(Paddle *pPaddle, float t);
void getPlayerSendData(Paddle *pPaddle, PaddleData *pData);
void updatePlayerWithRecievedData(Paddle *pPaddle, PaddleData *pData);
void restrictPlayerWithinWindow(Paddle *pPaddle, int w, int h);

#endif // PADDLE_H 