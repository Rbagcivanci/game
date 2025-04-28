#include <SDL.h>
#include <SDL_timer.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_net.h>
#include <time.h>

#include "paddle_data.h"
#include "paddle.h"
#include "text.h"
#include "ball.h"

#define WINDOW_WIDTH 1300
#define WINDOW_HEIGHT 800
#define SPEED 5
#define MIDDLE_FIELD 440
#define BALL_SIZE 10

typedef struct game {
    SDL_Window *pWindow;
    SDL_Renderer *pRenderer;
    SDL_Surface *pBackgroundSurface;
    SDL_Texture *backgroundTexture;
    TTF_Font *pFont;

    Text *pLobbyText, *pOverText, *pMatchTimeText, *pStartText, *pHostSpotText, *pSpot1Text, *pSpot2Text, *pSpot3Text, *pSpot4Text, *pIpText, *pTeamAText, *pTeamBText, *pGoalsTeamAText, *pGoalsTeamBText, *pEnterIpText, *pOutPutIpText;
    Paddle *pPaddle[MAX_PADDLES];
    Ball *pBall;
    GameState state;
    ServerData serverData;

    bool connected[MAX_PADDLES];
    int teamScores[2];
    int nrOfClients, nrOfPaddles;

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

Uint32 decreaseMatchTime(Uint32 interval, void *param);

void setUpGame(Game *pGame);
void closeGame(Game *pGame);

int main(int argc, char *argv[]) {
    //printf("Starting Pong Server...\n");
    Game g = {0};
    if(!initiate(&g)){
        //printf("Failed to initiate game\n");
        return 1;
    }
    srand(500);
    run(&g);
    closeGame(&g);
    return 0;
}


int initiate(Game *pGame){
    //srand(time(NULL));
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

    pGame->pFont = TTF_OpenFont("C:/Users/ahmed/game/lib/resources/arial.ttf", 24);
    if(!pGame->pFont){
        printf("Error: %s\n",SDL_GetError());
        return 0;
    }

    if(!(pGame->pSocket = SDLNet_UDP_Open(2000))){
        printf("SDLNet_UDP_Open: %s\n",SDLNet_GetError());
        closeGame(pGame);
        return 0;
    }
    //printf("UDP socket opened on port 2000\n"); PASS
    if(!(pGame->pPacket = SDLNet_AllocPacket(512))){
        printf("SDLNet_AllocPacket: %s\n",SDLNet_GetError());
        closeGame(pGame);
        return 0;
    }
    //printf("UDP packet allocated\n"); PASS
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
    pGame->pOverText = createText(pGame->pRenderer, 255, 255, 255, pGame->pFont, "Game Over", WINDOW_WIDTH / 2 - 100, WINDOW_HEIGHT / 2 - 50);
    pGame->pStartText = createText(pGame->pRenderer, 255, 255, 255, pGame->pFont, "Waiting for clients", WINDOW_WIDTH / 2 - 100, WINDOW_HEIGHT / 2 + 100);
    if(!pGame->pBall){
        printf("Failed to create ball\n");
        closeGame(pGame);
        return 0;
    }
    if(!pGame->pOverText || !pGame->pStartText){
        printf("Error: %s\n",SDL_GetError());
        closeGame(pGame);
        return 0;
    }

    pGame->teamScores[0] = 0;
    pGame->teamScores[1] = 0;

    pGame->state = START;
    pGame->nrOfClients = 0;
    //pGame->matchTime = 300000; // 5 minuter, millisekunder

    return 1;
}

void run(Game *pGame){
    int closeRequested = 0;
    SDL_Event event;
    ClientData clientData;

    Uint32 lastTick = SDL_GetTicks();
    Uint32 currentTick;
    float deltaTime;

    SDL_TimerID timerId = 0;

    while(!closeRequested){
        switch(pGame->state){
            case ONGOING:
                sendGameData(pGame);
                if(timerId == 0){
                    timerId = SDL_AddTimer(1000, decreaseMatchTime, &(pGame->matchTime));
                }

                currentTick = SDL_GetTicks();
                deltaTime = (currentTick - lastTick) / 1000.0f;
                lastTick = currentTick;

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
                    updatePaddlePosition(pGame->pPaddle[i], deltaTime);
                    restrictPaddleWithinWindow(pGame->pPaddle[i], WINDOW_WIDTH, WINDOW_HEIGHT);
                }
                updateBallPosition(pGame->pBall);
                for(int i=0; i<=1; i++){
                    if(pGame->teamScores[i] > 5){
                        pGame->state = GAME_OVER;
                    }
                }
                for(int i = 0; i < pGame->nrOfPaddles - 1; i++){
                    for(int j = i + 1; j < pGame->nrOfPaddles; j++){
                        handlePaddleCollision(pGame->pPaddle[i], pGame->pPaddle[j]);
                    }
                }
                for(int i = 0; i < pGame->nrOfPaddles; i++){
                    SDL_Rect paddleRect = getPaddleRect(pGame->pPaddle[i]);
                    SDL_Rect ballRect = getBallRect(pGame->pBall);
                    handlePaddleBallCollision(paddleRect, ballRect, pGame->pBall);
                }

                if(!checkGoal(pGame->pBall)){
                    restrictBallWithinWindow(pGame->pBall);
                } else {
                    for(int i = 0; i < pGame->nrOfPaddles; i++){
                        setStartingPosition(pGame->pPaddle[i], i, WINDOW_WIDTH, WINDOW_HEIGHT);
                    }

                    if(!goalScored(pGame->pBall)){
                        pGame->teamScores[0]++;
                    } else {
                        pGame->teamScores[1]++;
                    }
                }
                /*for (int i = 0; i < MAX_PADDLES; i++){
                    drawPaddle(pGame->pPaddle[i]);
                }*/
                SDL_RenderPresent(pGame->pRenderer);
                //renderGame(pGame);
                break;
            
            case GAME_OVER:
                drawText(pGame->pOverText);
                sendGameData(pGame);
                if(pGame->nrOfClients == MAX_PADDLES){
                    pGame->nrOfClients = 0;
                }
                break;
            case START:
                drawText(pGame->pStartText);
                SDL_RenderPresent(pGame->pRenderer);
                if(SDL_PollEvent(&event) && event.type==SDL_QUIT){
                    closeRequested = 1;
                }
                if(SDLNet_UDP_Recv(pGame->pSocket, pGame->pPacket)==1){
                    addClient(pGame->pPacket->address, pGame->clients, &(pGame->nrOfClients), pGame->connected);
                    if(pGame->nrOfClients == MAX_PADDLES){
                        setUpGame(pGame);
                    }
                }
                break;
        }
    }
    SDL_Delay(1000/60); // 60 FPS
    SDL_RemoveTimer(timerId);
}

/*void renderLobby(Game *pGame){
    SDL_RenderClear(pGame->pRenderer);

    char lobbyText[50], hostSpotText[50], spot1Text[50], spot2Text[50], spot3Text[50], spot4Text[50];

    snprintf(lobbyText, sizeof(lobbyText), "Lobby!");

    if(pGame->connected[0] == true){
        snprintf(hostSpotText, sizeof(hostSpotText), "Host is connected!");
        snprintf(spot1Text, sizeof(spot1Text), "Player 1 is connected!");
    } else {
        snprintf(hostSpotText, sizeof(hostSpotText), "Host is not connected...");
        snprintf(spot1Text, sizeof(spot1Text), "Spot 1 is available...");
    }
    if (pGame->connected[1] == true){
        snprintf(spot2Text, sizeof(spot2Text), "Player 2 is connected!");
    } else {
        snprintf(spot2Text, sizeof(spot2Text), "Spot 2 is available...");
    }
    if (pGame->connected[2] == true){
        snprintf(spot3Text, sizeof(spot3Text), "Player 3 is connected!");
    } else {
        snprintf(spot3Text, sizeof(spot3Text), "Spot 3 is available...");
    }
    if (pGame->connected[3] == true){
        snprintf(spot4Text, sizeof(spot4Text), "Player 4 is connected!");
    } else {
        snprintf(spot4Text, sizeof(spot4Text), "Spot 4 is available...");
    }

    pGame->pLobbyText = createText(pGame->pRenderer, 255, 255, 255, pGame->pFont, lobbyText, WINDOW_WIDTH / 2 - 100, WINDOW_HEIGHT / 2 - 100);
    pGame->pHostSpotText = createText(pGame->pRenderer, 255, 255, 255, pGame->pFont, hostSpotText, WINDOW_WIDTH / 2 - 100, WINDOW_HEIGHT / 2 - 70);
    pGame->pSpot1Text = createText(pGame->pRenderer, 255, 255, 255, pGame->pFont, spot1Text, WINDOW_WIDTH / 2 - 100, WINDOW_HEIGHT / 2 - 40);
    pGame->pSpot2Text = createText(pGame->pRenderer, 255, 255, 255, pGame->pFont, spot2Text, WINDOW_WIDTH / 2 - 100, WINDOW_HEIGHT / 2 - 10);
    pGame->pSpot3Text = createText(pGame->pRenderer, 255, 255, 255, pGame->pFont, spot3Text, WINDOW_WIDTH / 2 - 100, WINDOW_HEIGHT / 2 + 20);
    pGame->pSpot4Text = createText(pGame->pRenderer, 255, 255, 255, pGame->pFont, spot4Text, WINDOW_WIDTH / 2 - 100, WINDOW_HEIGHT / 2 + 50);

    drawText(pGame->pLobbyText);
    drawText(pGame->pHostSpotText);
    drawText(pGame->pSpot1Text);
    drawText(pGame->pSpot2Text);
    drawText(pGame->pSpot3Text);
    drawText(pGame->pSpot4Text);
    destroyText(pGame->pLobbyText);
    destroyText(pGame->pHostSpotText);
    destroyText(pGame->pSpot1Text);
    destroyText(pGame->pSpot2Text);
    destroyText(pGame->pSpot3Text);
    destroyText(pGame->pSpot4Text);
    SDL_RenderPresent(pGame->pRenderer);
}*/

/*void renderGame(Game *pGame){
    SDL_RenderClear(pGame->pRenderer);
    SDL_RenderCopy(pGame->pRenderer, pGame->backgroundTexture, NULL, NULL);

    int minutes = pGame->matchTime / 60000;
    int seconds = (pGame->matchTime % 60000) / 1000;

    char timeString[10], goalsTeamAString[10], goalsTeamBString[10];

    sprintf(timeString, "%02d:%02d", minutes, seconds);
    snprintf(goalsTeamAString, sizeof(goalsTeamAString), "%d", pGame->teamScores[0]);
    snprintf(goalsTeamBString, sizeof(goalsTeamBString), "%d", pGame->teamScores[1]);

    pGame->pMatchTimeText = createText(pGame->pRenderer, 255, 255, 255, pGame->pFont, timeString, WINDOW_WIDTH - 200, 20);
    pGame->pGoalsTeamAText = createText(pGame->pRenderer, 255, 255, 255, pGame->pFont, goalsTeamAString, WINDOW_WIDTH - 200, 50);
    pGame->pGoalsTeamBText = createText(pGame->pRenderer, 255, 255, 255, pGame->pFont, goalsTeamBString, WINDOW_WIDTH - 200, 80);

    if(!pGame->pMatchTimeText || !pGame->pGoalsTeamAText || !pGame->pGoalsTeamBText){
        printf("Error: %s\n",SDL_GetError());
        closeGame(pGame);
    }

    drawText(pGame->pMatchTimeText);
    drawText(pGame->pGoalsTeamAText);
    drawText(pGame->pGoalsTeamBText);
    destroyText(pGame->pMatchTimeText);
    destroyText(pGame->pGoalsTeamAText);
    destroyText(pGame->pGoalsTeamBText);

    for(int i = 0; i < MAX_PADDLES; i++){
        Paddle *pPaddle = pGame->pPaddle[i];
        SDL_Rect paddleRect = getPaddleRect(pPaddle);
        SDL_Texture *paddleTexture = getPaddleTexture(pPaddle);
        SDL_RenderCopy(pGame->pRenderer, paddleTexture, NULL, &paddleRect);
    }

    SDL_Rect ballRect = getBallRect(pGame->pBall);
    SDL_Texture *ballTexture = getBallTexture(pGame->pBall);
    SDL_RenderCopy(pGame->pRenderer, ballTexture, NULL, &ballRect);
    SDL_RenderPresent(pGame->pRenderer);
    SDL_Delay(1000/60); // 60 FPS
}*/

void setUpGame(Game *pGame){
    for(int i = 0; i < pGame->nrOfPaddles; i++){
        setStartingPosition(pGame->pPaddle[i], i, WINDOW_WIDTH, WINDOW_HEIGHT);
    }
    setBallX(pGame->pBall, WINDOW_WIDTH/2 - BALL_SIZE/2);
    setBallY(pGame->pBall, WINDOW_HEIGHT/2 - BALL_SIZE/2);
    float initialVelocityX = ((rand() % 2 == 0) ? -1 : 1) * SPEED;
    float initialVelocityY = ((rand() % 2 == 0) ? -1 : 1) * SPEED;
    pGame->nrOfPaddles = MAX_PADDLES;
    pGame->state = ONGOING;
}

void sendGameData(Game *pGame){
    pGame->serverData.gState = pGame->state;

    for(int i = 0; i < MAX_PADDLES; i++){
        getPaddleSendData(pGame->pPaddle[i], &(pGame->serverData.paddles[i]));
    }
    for(int i = 0; i < pGame->nrOfPaddles; i++){
        pGame->serverData.connected[i] = pGame->connected[i];
    }

    sendBallData(pGame->pBall, &(pGame->serverData.ball));
    
    for (int i = 0; i < MAX_PADDLES; i++){
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

Uint32 decreaseMatchTime(Uint32 interval, void *param){
    Uint32 *pMatchTime = (Uint32 *)param;
    if(*pMatchTime > 0){
        *pMatchTime -= 1000; // i millisekunder
    }
    return interval; // Continue the timer
}

void closeGame(Game *pGame){
    for(int i = 0; i < MAX_PADDLES; i++){
        if(pGame->pPaddle[i]){
            destroyPaddle(pGame->pPaddle[i]);
        }
    }
    if(pGame->pBall) destroyBall(pGame->pBall);
    if(pGame->pLobbyText) destroyText(pGame->pLobbyText);
    if(pGame->pSpot1Text) destroyText(pGame->pSpot1Text);
    if(pGame->pSpot2Text) destroyText(pGame->pSpot2Text);
    if(pGame->pSpot3Text) destroyText(pGame->pSpot3Text);
    if(pGame->pSpot4Text) destroyText(pGame->pSpot4Text);
    if(pGame->pHostSpotText) destroyText(pGame->pHostSpotText);
    if(pGame->pMatchTimeText) destroyText(pGame->pMatchTimeText);
    if(pGame->pFont) TTF_CloseFont(pGame->pFont);

    SDLNet_Quit();
    TTF_Quit();
    SDL_Quit();
}