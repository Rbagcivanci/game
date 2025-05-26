#include <SDL.h>
#include <SDL_timer.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_net.h>
#include <time.h>
#include <stdlib.h>
#include "paddle_data.h"
#include "paddle.h"
#include "text.h"
#include "ball.h"
#include "obstacles.h"
#define WINDOW_WIDTH 1300
#define WINDOW_HEIGHT 800
#define MOVEMENT_SPEED 300
#define SPEED 20
#define MIDDLE_FIELD 440
#define BALL_SIZE 10

typedef struct game {
    SDL_Window *pWindow;
    SDL_Renderer *pRenderer;
    SDL_Surface *pBackgroundSurface;
    SDL_Texture *backgroundTexture;
    TTF_Font *pFont;
    Text *pOverText, *pMatchTimeText, *pStartText, *pOngoingText;
    Paddle *pPaddle[MAX_PADDLES];
    Ball *pBall;
    GameState state;
    ServerData serverData;
    bool connected[MAX_PADDLES];
    int teamScores[2];
    int nrOfClients, nrOfPaddles;
    bool hostConnected;
    UDPsocket pSocket;
    UDPpacket *pPacket;
    Uint32 matchTime;
    IPaddress clients[MAX_PADDLES];
}Game;

int initiate(Game *pGame);
void run(Game *pGame);
void renderGame(Game *pGame);
void renderLobby(Game *pGame);
void handleInput(Game *pGame, SDL_Event *pEvent);
void addClient(IPaddress address, IPaddress clients[], int *pNrOfClients, bool connected[]);
void sendGameData(Game *pGame);
void executeCommand(Game *pGame, ClientData clientData);
void setUpGame(Game *pGame);
void closeGame(Game *pGame);

int main(int argc, char *argv[]) {
    Game g = {0};
    if(!initiate(&g)){
        return 1;
    }
    run(&g);
    closeGame(&g);
    return 0;
}


int initiate(Game *pGame){
    srand(time(NULL));
    if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER)!= 0){
        printf("Error: %s\n",SDL_GetError());
        return 0;
    }

    if(TTF_Init()!= 0){
        printf("Error: %s\n",SDL_GetError());
        return 0;
    }

    if(SDLNet_Init() != 0){
        printf("Error: %s\n",SDL_GetError());
        return 0;
    }

    pGame->pWindow = SDL_CreateWindow("Pong Server", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if(!pGame->pWindow){
        printf("Error: %s\n",SDL_GetError());
        return 0;
    }

    pGame->pRenderer = SDL_CreateRenderer(pGame->pWindow, -1, SDL_RENDERER_ACCELERATED);
    if(!pGame->pRenderer){
        printf("Error: %s\n",SDL_GetError());
        return 0;
    }

    pGame->pFont = TTF_OpenFont("../lib/resources/Symtext.ttf", 24);
    if(!pGame->pFont){
        printf("Error: %s\n",SDL_GetError());
        return 0;
    }

    if(!(pGame->pSocket = SDLNet_UDP_Open(2000))){
        printf("SDLNet_UDP_Open: %s\n",SDLNet_GetError());
        closeGame(pGame);
        return 0;
    }

    if(!(pGame->pPacket = SDLNet_AllocPacket(512))){
        printf("SDLNet_AllocPacket: %s\n",SDLNet_GetError());
        closeGame(pGame);
        return 0;
    }

    for (int i = 0; i < MAX_PADDLES; i++){
        pGame->pPaddle[i] = createPaddle(pGame->pRenderer, WINDOW_WIDTH, WINDOW_HEIGHT, i);
    }
    for(int i = 0; i < MAX_PADDLES; i++){
        if(!pGame->pPaddle[i]){
            printf("Error: %s\n",SDL_GetError());
            closeGame(pGame);
            return 0;
        }
    }

    pGame->nrOfPaddles = MAX_PADDLES;
    pGame->pBall = createBall(pGame->pRenderer);
    init_obstacles();
    pGame->pOngoingText = createText(pGame->pRenderer, 255, 255, 255, pGame->pFont, "Game has begun", WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);
    pGame->pOverText = createText(pGame->pRenderer, 255, 255, 255, pGame->pFont, "Game Over", WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);
    pGame->pStartText = createText(pGame->pRenderer, 255, 255, 255, pGame->pFont, "Waiting for clients...", WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);
    if(!pGame->pBall){
        printf("Failed to create ball\n");
        closeGame(pGame);
        return 0;
    }


    if(!pGame->pOverText || !pGame->pStartText || !pGame->pOngoingText){
        printf("Error: %s\n",SDL_GetError());
        closeGame(pGame);
        return 0;
    }

    pGame->teamScores[0] = 0;
    pGame->teamScores[1] = 0;
    pGame->state = START;
    pGame->nrOfClients = 0;

    return 1;
}

void run(Game *pGame){
    int closeRequested = 0;
    SDL_Event event;
    ClientData clientData;

    while(!closeRequested){
        switch(pGame->state){
            case ONGOING:
                SDL_RenderClear(pGame->pRenderer);
                drawText(pGame->pOngoingText);
                sendGameData(pGame);
                while(SDLNet_UDP_Recv(pGame->pSocket, pGame->pPacket)==1){
                    memcpy(&clientData, pGame->pPacket->data, sizeof(ClientData));
                    executeCommand(pGame, clientData);
                }

                if(SDL_PollEvent(&event)){
                    if(event.type == SDL_QUIT){
                        closeRequested = 1;
                    }
                }

                for(int i = 0; i<MAX_PADDLES; i++){
                    updatePaddlePosition(pGame->pPaddle[i]);
                    restrictPaddleWithinWindow(pGame->pPaddle[i], WINDOW_WIDTH, WINDOW_HEIGHT);
                    handlePaddleBallCollision(getPaddleRect(pGame->pPaddle[i]), getBallRect(pGame->pBall), pGame->pBall);
                }
                updateBallPosition(pGame->pBall);
                check_obstacle_collisions(pGame->pBall);
                restrictBallWithinWindow(pGame->pBall);

                for(int i = 0; i < pGame->nrOfPaddles - 1; i++){
                    for(int j = i + 1; j < pGame->nrOfPaddles; j++){
                        handlePaddleCollision(pGame->pPaddle[i], pGame->pPaddle[j]);
                    }
                }
                int goalTeam = goalScored(pGame->pBall);
                if (goalTeam >= 0) {
                    pGame->teamScores[goalTeam]++;
                    for (int i = 0; i < pGame->nrOfPaddles; i++) {
                        setStartingPosition(pGame->pPaddle[i], i, WINDOW_WIDTH, WINDOW_HEIGHT);
                    }
                    reset_obstacles();
                    spawn_obstacles();
                    serveBall(pGame->pBall, (rand() % 2) * 2 - 1);
                }
                sendGameData(pGame); // Skicka uppdaterad poäng till klienter
                for (int i = 0; i <= 2; i++) {
                    if (pGame->teamScores[i] >= 5) {
                        pGame->state = GAME_OVER;
                    }
                }
                SDL_RenderPresent(pGame->pRenderer);
                break;
            
            case GAME_OVER:
                SDL_RenderClear(pGame->pRenderer);
                drawText(pGame->pOverText);
                SDL_RenderPresent(pGame->pRenderer);
                sendGameData(pGame);
                if(pGame->nrOfClients == MAX_PADDLES){
                    pGame->nrOfClients = 0;
                }
                break;
            case START:
                SDL_RenderClear(pGame->pRenderer);
                drawText(pGame->pStartText);
                SDL_RenderPresent(pGame->pRenderer);
                if(SDL_PollEvent(&event) && event.type == SDL_QUIT) closeRequested=1;
                if(SDLNet_UDP_Recv(pGame->pSocket, pGame->pPacket)==1){
                    addClient(pGame->pPacket->address, pGame->clients, &(pGame->nrOfClients), pGame->connected);
                    sendGameData(pGame);
                    if(pGame->nrOfClients == MAX_PADDLES) setUpGame(pGame);
                }
                sendGameData(pGame);
                break;
        }
        SDL_Delay(1000/60);
    }
}

void setUpGame(Game *pGame){
    int value = (rand() % 2) * 2 - 1;
    for(int i = 0; i < pGame->nrOfPaddles; i++){
        setStartingPosition(pGame->pPaddle[i], i, WINDOW_WIDTH, WINDOW_HEIGHT);
    }
    setBallX(pGame->pBall, WINDOW_WIDTH/2 - BALL_SIZE/2);
    setBallY(pGame->pBall, WINDOW_HEIGHT/2 - BALL_SIZE/2);
    float initialVelocityX = SPEED;
    float initialVelocityY = SPEED;
    setBallVelocity(pGame->pBall, initialVelocityX, initialVelocityY);
    serveBall(pGame->pBall, value);
    spawn_obstacles();
    pGame->nrOfPaddles = MAX_PADDLES;
    pGame->state = ONGOING;
}

void sendGameData(Game *pGame){
    pGame->serverData.gState = pGame->state;
    pGame->serverData.teamScores[0] = pGame->teamScores[0];
    pGame->serverData.teamScores[1] = pGame->teamScores[1];
    pGame->serverData.hostConnected = true; // Servern är host

    for(int i = 0; i < MAX_PADDLES; i++){
        getPaddleSendData(pGame->pPaddle[i], &(pGame->serverData.paddles[i]));
    }

    for(int i=0; i<pGame->nrOfPaddles; i++){
        pGame->serverData.connected[i]=pGame->connected[i];
    }

    sendBallData(pGame->pBall, &(pGame->serverData.ball));
    send_obstacle_data(pGame -> serverData.obstacles);
    
    for (int i = 0; i < pGame->nrOfClients; i++){
        pGame->serverData.clientNr = i;
        memcpy(pGame->pPacket->data, &(pGame->serverData), sizeof(ServerData));
        pGame->pPacket->len = sizeof(ServerData);
        pGame->pPacket->address = pGame->clients[i];
        SDLNet_UDP_Send(pGame->pSocket, -1, pGame->pPacket);
    }
}

void addClient(IPaddress address, IPaddress clients[], int *pNrOfClients, bool connected[]){
    for(int i = 0; i < *pNrOfClients; i++){
        if(address.host == clients[i].host && address.port == clients[i].port){
            return;
        }
    }
    clients[*pNrOfClients] = address;
    connected[*pNrOfClients] = true;
    (*pNrOfClients)++;
}

void executeCommand(Game *pGame, ClientData clientData){
    switch(clientData.command){
        case UP:
            updatePaddleVUp(pGame->pPaddle[clientData.clientNumber]);
            break;
        case DOWN:
            updatePaddleVDown(pGame->pPaddle[clientData.clientNumber]);
            break;
        case LEFT:
            updatePaddleVLeft(pGame->pPaddle[clientData.clientNumber]);
            break;
        case RIGHT:
            updatePaddleVRight(pGame->pPaddle[clientData.clientNumber]);
            break;
        case RESET_VELOCITY_Y:
            resetPaddleSpeed(pGame->pPaddle[clientData.clientNumber], 0, 1);
            break;
        case RESET_VELOCITY_X:
            resetPaddleSpeed(pGame->pPaddle[clientData.clientNumber], 1, 0);
            break;
        case RESTRICT_PLAYER:
            restrictPaddleWithinWindow(pGame->pPaddle[clientData.clientNumber], WINDOW_WIDTH, WINDOW_HEIGHT);
            break;
    }
}

void closeGame(Game *pGame){
    for(int i = 0; i < MAX_PADDLES; i++){
        if(pGame->pPaddle[i]){
            destroyPaddle(pGame->pPaddle[i]);
        }
    }
    if(pGame->pBall) destroyBall(pGame->pBall);
    if(pGame->pRenderer) SDL_DestroyRenderer(pGame->pRenderer);
    if(pGame->pWindow) SDL_DestroyWindow(pGame->pWindow);
    if(pGame->pOverText) destroyText(pGame->pOverText);
    if(pGame->pStartText) destroyText(pGame->pStartText);
    if(pGame->pOngoingText) destroyText(pGame->pOngoingText);
    if(pGame->pMatchTimeText) destroyText(pGame->pMatchTimeText);
    if(pGame->pFont) TTF_CloseFont(pGame->pFont);
    if(pGame->pPacket) SDLNet_FreePacket(pGame->pPacket);
    if(pGame->pSocket) SDLNet_UDP_Close(pGame->pSocket);

    SDLNet_Quit();
    TTF_Quit();
    SDL_Quit();
}