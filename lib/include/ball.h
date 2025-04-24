#ifndef ball_h
#define ball_h

typedef struct ball Ball;

Ball *createBall(SDL_Renderer *renderer);
SDL_Texture *getBallTexture(Ball *pBall);
SDL_Rect getBallRect(Ball *pBall);

#endif // BALL_H