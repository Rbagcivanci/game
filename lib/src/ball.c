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

struct ball {
    SDL_Texture *ballTexture;
    SDL_Rect ballRect;
    SDL_Surface *ballSurface;
    float velocityY, velocityX;
    float ballX, ballY;
};

Ball *createBall(SDL_Renderer *renderer) {
    Ball *pBall = malloc(sizeof(Ball)); //Allokera minne till boll
    if (!pBall){
        fprintf(stderr, "Failed to allocate memory for pBall.\n");
        return NULL;
    }

    pBall->ballSurface = IMG_Load("../lib/resources/ball2.png"); // Surface med bild
    if (!pBall->ballSurface) { //Om bild inte lyckas skapas --> felmeddelande, släpp minne och returnera NULL
        fprintf(stderr, "Error loading pBall texture: %s\n", SDL_GetError());
        free(pBall);
        return NULL;
    }

    pBall->ballTexture = SDL_CreateTextureFromSurface(renderer, pBall->ballSurface); //Skapa textur från ytan
    SDL_FreeSurface(pBall->ballSurface); //Släpp ytan efter att texturen skapats
    if (!pBall->ballTexture) { //Om texturen inte lyckas skapas --> felmeddelande, släpp minne och returnera NULL
        fprintf(stderr, "Error creating pBall texture: %s\n", SDL_GetError());
        free(pBall);
        return NULL;
    }

    pBall->ballRect.w = BALL_SIZE; //Sätt storlek på rektangeln
    pBall->ballRect.h = BALL_SIZE;
    pBall->ballRect.x = WINDOW_WIDTH / 2 - pBall->ballRect.w / 2; //Sätt position på rektangeln
    pBall->ballRect.y = MIDDLE_FIELD - pBall->ballRect.h / 2;
    pBall->velocityY = SPEED; 
    pBall->velocityX = SPEED; //Sätt hastighet på bollen
    pBall->ballX = WINDOW_WIDTH / 2 - BALL_SIZE / 2;
    pBall->ballY = WINDOW_HEIGHT / 2 - BALL_SIZE / 2;

    return pBall;
}

void updateBallPosition(Ball *pBall, float deltaTime) {
    pBall->ballX += pBall->velocityX * deltaTime;
    pBall->ballY += pBall->velocityY *deltaTime;
    pBall->ballRect.x = (int)pBall->ballX;
    pBall->ballRect.y = (int)pBall->ballY;
}

SDL_Texture *getBallTexture(Ball *pBall) {
    return pBall->ballTexture;
}

SDL_Rect getBallRect(Ball *pBall) {
    return pBall->ballRect;
}

void setBallVelocity(Ball *pBall, float velocityX, float velocityY) {
    pBall->velocityX = velocityX; //Sätt hastighet på bollen
    pBall->velocityY = velocityY;
}

void setBallX(Ball *pBall, int x) {
    pBall->ballRect.x = x; //Sätt position på bollen
}

void setBallY(Ball *pBall, int y) {
    pBall->ballRect.y = y; //Sätt position på bollen
}

void destroyBall(Ball *pBall){
    SDL_DestroyTexture(pBall->ballTexture); //Släpp texturen
    free(pBall); //Släpp minnet
}

int checkCollision(SDL_Rect rect1, SDL_Rect rect2){
    return SDL_HasIntersection(&rect1, &rect2); //Kolla om boll kolliderar med annat
}

/*int checkGoal(Ball *pBall){
    SDL_Rect ballRect = getBallRect(pBall); //Hämta rektangel för bollen
    if(ballRect.x < 0 || ballRect.x > WINDOW_WIDTH - BALL_SIZE){
        return 1;
    }
    return 0; //Inga mål
}*/

int goalScored(Ball *pBall) {
    SDL_Rect ballRect = getBallRect(pBall);

    if (ballRect.x < 0) {
        //printf("Goal on left side! (Team B scores)\n");
        //setBallX(pBall, WINDOW_WIDTH / 2 - BALL_SIZE / 2);
        //setBallY(pBall, WINDOW_HEIGHT / 2 - BALL_SIZE / 2);
        //serveBall(pBall, -1); // Serve mot vänster
        return 1; // Team B
    }
    if (ballRect.x > WINDOW_WIDTH - BALL_SIZE) {
        //printf("Goal on right side! (Team A scores)\n");
        //setBallX(pBall, WINDOW_WIDTH / 2 - BALL_SIZE / 2);
        //setBallY(pBall, WINDOW_HEIGHT / 2 - BALL_SIZE / 2);
        //serveBall(pBall, 1); // Serve mot höger
        return 0; // Team A
    }
    return -1; // Inget mål
}

void updateBallWithRecievedData(Ball *pBall, BallData *pBallData){
    pBall->velocityX = pBallData->velocityX; //Uppdatera hastighet på bollen
    pBall->velocityY = pBallData->velocityY;
    pBall->ballRect.x = pBallData->positionX; //Uppdatera position på bollen
    pBall->ballRect.y = pBallData->positionY;
}

void sendBallData(Ball *pBall, BallData *pBallData){
    pBallData->velocityX = pBall->velocityX; //Skicka hastighet på bollen
    pBallData->velocityY = pBall->velocityY;
    pBallData->positionX = pBall->ballRect.x; //Skicka position på bollen
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
    // Hantera sidoväggar separat för att undvika målkonflikt
    if (ballRect.x < 0 || ballRect.x + BALL_SIZE > WINDOW_WIDTH) {
        // Målhantering sker i goalScored, så här gör vi inget
    }
}

void handlePaddleBallCollision(SDL_Rect paddleRect, SDL_Rect ballRect, Ball *pBall) {
    if (checkCollision(paddleRect, ballRect)) {
        // Beräkna träffpunktens relativa position på paddeln (-1 till 1)
        float relativeIntersectY = (paddleRect.y + paddleRect.h / 2.0f) - (ballRect.y + ballRect.h / 2.0f);
        float normalizedRelativeIntersectY = relativeIntersectY / (paddleRect.h / 2.0f);
        // Justera bollens Y-hastighet baserat på träffpunkten
        float bounceAngle = normalizedRelativeIntersectY * (45.0f * (M_PI / 180.0f)); // Max 45 graders vinkel
        float ballSpeed = sqrt(pBall->velocityX * pBall->velocityX + pBall->velocityY * pBall->velocityY);
        ballSpeed = ballSpeed < SPEED ? SPEED : ballSpeed * 1.1; // Öka hastighet något vid träff
        pBall->velocityX = (paddleRect.x < WINDOW_WIDTH / 2 ? 1 : -1) * ballSpeed * cos(bounceAngle);
        pBall->velocityY = -ballSpeed * sin(bounceAngle);

        // Justera position för att undvika fastnande
        if (paddleRect.x < WINDOW_WIDTH / 2) { // Vänster paddel
            setBallX(pBall, paddleRect.x + paddleRect.w + 1);
        } else { // Höger paddel
            setBallX(pBall, paddleRect.x - ballRect.w - 1);
        }
    }
}
void serveBall(Ball *pBall, int direction){
    setBallX(pBall, WINDOW_WIDTH / 2 - BALL_SIZE / 2);
    setBallY(pBall, WINDOW_HEIGHT / 2 - BALL_SIZE / 2);
    float velocityX = direction * SPEED; // direction: 1 för höger, -1 för vänster
    float velocityY = direction * SPEED;
    setBallVelocity(pBall, velocityX, velocityY);
}