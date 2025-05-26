#include <SDL.h>
#include "paddle_data.h"
#include "ball.h"
#include <SDL_image.h>
#include <stdbool.h> 

#define WINDOW_WIDTH 1300
#define WINDOW_HEIGHT 800
#define MIDDLE_FIELD 440
#define GOAL_TOP 0
#define GOAL_BOTTOM 800
#define BALL_SIZE 10
#define SPEED  5.1f


Ball *createBall(SDL_Renderer *renderer) {
    Ball *pBall = malloc(sizeof(Ball));
    if (!pBall){
        fprintf(stderr, "Failed to allocate memory for pBall.\n");
        return NULL;
    }

    pBall->ballSurface = IMG_Load("../lib/resources/ballPong.png");
    if (!pBall->ballSurface) {
        fprintf(stderr, "Error loading pBall texture: %s\n", SDL_GetError());
        free(pBall);
        return NULL;
    }

    pBall->ballTexture = SDL_CreateTextureFromSurface(renderer, pBall->ballSurface);
    SDL_FreeSurface(pBall->ballSurface);
    if (!pBall->ballTexture) {
        fprintf(stderr, "Error creating pBall texture: %s\n", SDL_GetError());
        free(pBall);
        return NULL;
    }

    pBall->ballRect.w = BALL_SIZE;
    pBall->ballRect.h = BALL_SIZE;
    pBall->ballRect.x = WINDOW_WIDTH / 2 - pBall->ballRect.w / 2;
    pBall->ballRect.y = MIDDLE_FIELD - pBall->ballRect.h / 2;
    pBall->velocityY = SPEED; 
    pBall->velocityX = SPEED;
    pBall->ballX = WINDOW_WIDTH / 2 - BALL_SIZE / 2;
    pBall->ballY = WINDOW_HEIGHT / 2 - BALL_SIZE / 2;

    return pBall;
}

void updateBallPosition(Ball *pBall) {
    pBall->ballRect.x += pBall->velocityX;
    pBall->ballRect.y += pBall->velocityY;
}

SDL_Texture *getBallTexture(Ball *pBall) {
    return pBall->ballTexture;
}

SDL_Rect getBallRect(Ball *pBall) {
    return pBall->ballRect;
}

void setBallVelocity(Ball *pBall, float velocityX, float velocityY) {
    pBall->velocityX = velocityX;
    pBall->velocityY = velocityY;
}

void setBallX(Ball *pBall, int x) {
    pBall->ballRect.x = x;
}

void setBallY(Ball *pBall, int y) {
    pBall->ballRect.y = y;
}

void destroyBall(Ball *pBall){
    SDL_DestroyTexture(pBall->ballTexture);
    free(pBall);
}

int checkCollision(SDL_Rect rect1, SDL_Rect rect2){
    return SDL_HasIntersection(&rect1, &rect2); 
}

int goalScored(Ball *pBall) {
    SDL_Rect ballRect = getBallRect(pBall);
    if (ballRect.x < 0) return 1; // Team B
    if (ballRect.x > WINDOW_WIDTH - BALL_SIZE) return 0;
    return -1;
}

void updateBallWithRecievedData(Ball *pBall, BallData *pBallData){
    pBall->velocityX = pBallData->velocityX;
    pBall->velocityY = pBallData->velocityY;
    pBall->ballRect.x = pBallData->positionX;
    pBall->ballRect.y = pBallData->positionY;
}

void sendBallData(Ball *pBall, BallData *pBallData){
    pBallData->velocityX = pBall->velocityX;
    pBallData->velocityY = pBall->velocityY;
    pBallData->positionX = pBall->ballRect.x;
    pBallData->positionY = pBall->ballRect.y;
}

void restrictBallWithinWindow(Ball *pBall) {
    SDL_Rect ballRect = getBallRect(pBall);

    if (ballRect.y < 0) {
        setBallY(pBall, 0);
        setBallVelocity(pBall, pBall->velocityX, -pBall->velocityY);
    }
    if (ballRect.y + BALL_SIZE > WINDOW_HEIGHT) {
        setBallY(pBall, WINDOW_HEIGHT - BALL_SIZE);
        setBallVelocity(pBall, pBall->velocityX, -pBall->velocityY);
    }
    if (ballRect.x < 0 || ballRect.x + BALL_SIZE > WINDOW_WIDTH) {
    }
}

void handlePaddleBallCollision(SDL_Rect paddleRect, SDL_Rect ballRect, Ball *pBall) {
    if (checkCollision(paddleRect, ballRect)) {
        float relativeIntersectY = (paddleRect.y + paddleRect.h / 2.0f) - (ballRect.y + ballRect.h / 2.0f);
        float normalizedRelativeIntersectY = relativeIntersectY / (paddleRect.h / 2.0f);
        float bounceAngle = normalizedRelativeIntersectY * (45.0f * (M_PI / 180.0f));
        float ballSpeed = sqrt(pBall->velocityX * pBall->velocityX + pBall->velocityY * pBall->velocityY);
        ballSpeed = ballSpeed < SPEED ? SPEED : ballSpeed * 1.1;
        pBall->velocityX = (paddleRect.x < WINDOW_WIDTH / 2 ? 1 : -1) * ballSpeed * cos(bounceAngle);
        pBall->velocityY = -ballSpeed * sin(bounceAngle);

        if (paddleRect.x < WINDOW_WIDTH / 2) {
            setBallX(pBall, paddleRect.x + paddleRect.w + 1);
        } else {
            setBallX(pBall, paddleRect.x - ballRect.w - 1);
        }
    }
}
void serveBall(Ball *pBall, int direction){
    setBallX(pBall, WINDOW_WIDTH / 2 - BALL_SIZE / 2);
    setBallY(pBall, WINDOW_HEIGHT / 2 - BALL_SIZE / 2);
    float velocityX = direction * SPEED;
    float velocityY = direction * SPEED;
    setBallVelocity(pBall, velocityX, velocityY);
}