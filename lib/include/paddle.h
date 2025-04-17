#ifndef PADDLE_H
#define PADDLE_H 

typedef struct paddle Paddle;

Paddle *createPaddle(int number, SDL_Renderer *pRenderer, int window_width, int window_height);
SDL_Rect getPaddleRect(Paddle *pPaddle);
SDL_Texture *getPaddleTexture(Paddle *pPaddle);
void updatePaddleVelocity(Paddle *pPaddle, float velocityY);
void updatePaddleVelocityUp(Paddle *pPaddle);
void updatePaddleVelocityDown(Paddle *pPaddle);
int getPaddleSpeedY(Paddle *pPaddle);
void setPaddlePosition(Paddle *pPaddle, int x, int y);
void setStartingPosition(Paddle *pPaddle, int playerIndex, int w, int h);
void destroyPaddle(Paddle *pPaddle);
void handlePaddleCollision(Paddle *pPaddle1, Paddle *pPaddle2);
void updatePaddlePos(Paddle *pPaddle, float t);

#endif // PADDLE_H 