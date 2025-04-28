#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <SDL.h>
#include <SDL_main.h>
#include <SDL_timer.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_net.h>

#include "paddle_data.h"
#include "paddle.h"
#include "ball.h"
#include "text.h"

typedef struct game {
    SDL_Window *pWindow;
    SDL_Renderer *pRenderer;
    SDL_Surface *pBackgroundSurface, *pIpSurface;
    SDL_Texture *backgroundTexture, *pIpTexture;
    TTF_Font *pFont;

    Text *pLobbyText, *pEnterIpText, *pOutPutIpText, *pIpText, *pTeamAText, *pTeamBText, *pDrawText, *pGameOverText, *pMatchTimeText, *pGoalsTeamAText, *pGoalsTeamBText, *pHostSpotText, *pSpot1Text, *pSpot2Text, *pSpot3Text, *pSpot4Text;
    Paddle *pPaddle[MAX_PADDLES];
    Ball *pBall;
    GameState state;
    ClientData clients[MAX_PADDLES];

    bool connected[MAX_PADDLES];
    int hostConnected;

    char pIp[50];

    int teamA;
    int teamB;
    int nrOfPaddles, paddleNr;

    UDPsocket pSocket;
    IPaddress serverAddress;
    UDPpacket *pPacket;

    Uint32 matchTime;
}Game;

int initiate(Game *pGame);
void run(Game *pGame);
void renderGame(Game *pGame);
void renderLobby(Game *pGame);
void handleInput(Game *pGame, SDL_Event *pEvent);
void updateWithServerData(Game *pGame);
Uint32 decreaseMatchTime(Uint32 interval, void *param);
void getInputIp(Game *pGame);

void handleGameOverText(Game *pGame);
void closeGame(Game *pGame);

int main(int argc, char *argv[]) {
    //printf("Starting Pong Client...\n");
    Game g = {0};
    if(!initiate(&g)){
        return 1;
    }
    run(&g);
    closeGame(&g);
    return 0;
}

int initiate(Game *pGame){
    //printf("Initiating...\n");
    if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER)!= 0){
        printf("Error: %s\n",SDL_GetError());
        return 0;
    }

    if(TTF_Init()!= 0){
        printf("Error: %s\n",TTF_GetError());
        SDL_Quit();
        return 0;
    }

    if(SDLNet_Init() != 0){
        printf("SDLNet_Init: %s\n",SDLNet_GetError());
        TTF_Quit();
        SDL_Quit();
        return 0;
    }

    if(!(IMG_Init(IMG_INIT_PNG))){
        printf("Error: %s\n",SDL_GetError());
        closeGame(pGame);
        return 0;
    }
    
    pGame->pWindow = SDL_CreateWindow("Pong Client", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if(!pGame->pWindow){
        printf("Error: %s\n",SDL_GetError());
        closeGame(pGame);
        return 0;
    }

    pGame->pRenderer = SDL_CreateRenderer(pGame->pWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if(!pGame->pRenderer){
        printf("Error: %s\n",SDL_GetError());
        closeGame(pGame);
        return 0;
    }

    pGame->pFont = TTF_OpenFont("C:/Users/ahmed/game/lib/resources/arial.ttf", 50);
    if(!pGame->pFont){
        printf("Error: %s\n",TTF_GetError());
        closeGame(pGame);
        return 0;
    }

    if (!(pGame->pSocket = SDLNet_UDP_Open(0))) {
        printf("SDLNet_UDP_Open misslyckades: %s\n", SDLNet_GetError());
        return 0;
    }

    if(SDLNet_ResolveHost(&(pGame->serverAddress), "127.0.0.1", 2000)) {
        printf("SDLNet_ResolveHost(127.0.0.1 2000): %s\n", SDLNet_GetError());
        return 0;
    }

    if(!(pGame->pPacket = SDLNet_AllocPacket(512))){
        printf("SDLNet_AllocPacket: %s\n",SDLNet_GetError());
        return 0;
    }

    pGame->pPacket->address.host = pGame->serverAddress.host;
    pGame->pPacket->address.port = pGame->serverAddress.port;

    pGame->pBackgroundSurface = IMG_Load("C:/Users/ahmed/game/lib/resources/bakgrundMain.png");
    pGame->pIpSurface = IMG_Load("C:/Users/ahmed/game/lib/resources/bakgrundMain.png");
    if(!pGame->pBackgroundSurface || !pGame->pIpSurface){
        printf("Error: %s\n",SDL_GetError());
        closeGame(pGame);
        return 0;
    }

    pGame->backgroundTexture = SDL_CreateTextureFromSurface(pGame->pRenderer, pGame->pBackgroundSurface);
    pGame->pIpTexture = SDL_CreateTextureFromSurface(pGame->pRenderer, pGame->pIpSurface);
    SDL_FreeSurface(pGame->pBackgroundSurface);
    SDL_FreeSurface(pGame->pIpSurface);
    if(!pGame->backgroundTexture || !pGame->pIpTexture){
        printf("Error: %s\n",SDL_GetError());
        closeGame(pGame);
        return 0;
    }

    for (int i = 0; i < MAX_PADDLES; i++){
        pGame->pPaddle[i] = createPaddle( pGame->pRenderer, WINDOW_WIDTH, WINDOW_HEIGHT, i);
        if(!pGame->pPaddle[i]){
            fprintf(stderr , "Failed to create paddle %d\n", i+1);
            return 0;
        }
    }

    for(int i = 0; i < MAX_PADDLES; i++){
        pGame->connected[i] = false;
    }

    pGame->nrOfPaddles = MAX_PADDLES;

    pGame->pBall = createBall(pGame->pRenderer);
    if(!pGame->pBall){
        printf("Failed to create ball\n");
        closeGame(pGame);
        return 0;
    }

    pGame->pTeamAText = createText(pGame->pRenderer, 255, 255, 255, pGame->pFont, "Team A Won", WINDOW_WIDTH / 2 - 100, WINDOW_HEIGHT / 2 - 50);
    pGame->pTeamBText = createText(pGame->pRenderer, 255, 255, 255, pGame->pFont, "Team B Won", WINDOW_WIDTH / 2 - 100, WINDOW_HEIGHT / 2 - 50);
    pGame->pDrawText = createText(pGame->pRenderer, 255, 255, 255, pGame->pFont, "Draw", WINDOW_WIDTH / 2 - 100, WINDOW_HEIGHT / 2 - 50);
    pGame->pGameOverText = createText(pGame->pRenderer, 255, 255, 255, pGame->pFont, "Game Over", WINDOW_WIDTH / 2 - 100, 
    WINDOW_HEIGHT / 2 - 50);
    if(!pGame->pTeamAText || !pGame->pTeamBText || !pGame->pDrawText || !pGame->pGameOverText){
        printf("Error: %s\n",SDL_GetError());
        closeGame(pGame);
        return 0;
    }

    pGame->teamA = 0;
    pGame->teamB = 0;
    pGame->state = START;
    pGame->matchTime = 300000; // 5 minuter, millisekunder

    return 1;
}

void run(Game *pGame){
    int closeRequested = 0;
    SDL_Event event;
    ClientData clientData;

    Uint32 lastTick = SDL_GetTicks();
    Uint32 currentTick;
    //currentTick = SDL_GetTicks();
    float deltaTime;
    SDL_TimerID timerId = 0;
    int joining = 0;
    pGame->hostConnected = 0;
    strcpy(pGame->pIp, "Null");

    while(!closeRequested){
        switch(pGame->state){
            case ONGOING:
                //printf("Ongoing\n");
                if(timerId == 0){
                    timerId = SDL_AddTimer(1000, decreaseMatchTime, pGame);
                }

                currentTick = SDL_GetTicks();
                deltaTime = (currentTick - lastTick) / 1000.0f;
                lastTick = currentTick;
                while(SDLNet_UDP_Recv(pGame->pSocket, pGame->pPacket)){
                    updateWithServerData(pGame);
                }

                while(SDL_PollEvent(&event)){
                    handleInput(pGame, &event);
                    if(event.type == SDL_QUIT){
                        closeRequested = 1;
                    } else handleInput(pGame, &event);
                }

                for(int i = 0; i < MAX_PADDLES; i++){
                    updatePaddlePosition(pGame->pPaddle[i], deltaTime);
                    restrictPaddleWithinWindow(pGame->pPaddle[i], WINDOW_WIDTH, WINDOW_HEIGHT);
                }

                for (int i = 0; i < pGame->nrOfPaddles - 1; i++){
                    for(int j = i + 1; j < pGame->nrOfPaddles; j++){
                        handlePaddleCollision(pGame->pPaddle[i], pGame->pPaddle[j]);
                    }
                }

                for (int i = 0; i < pGame->nrOfPaddles; i++){
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

                    if(goalScored(pGame->pBall)){
                        pGame->teamA++;
                    } else {
                        pGame->teamB++;
                    }

                    for(int i = 0; i < pGame->nrOfPaddles; i++){
                        setStartingPosition(pGame->pPaddle[i], i, WINDOW_WIDTH, WINDOW_HEIGHT);
                    }
                }
                renderGame(pGame);
                break;
            
            case GAME_OVER:
                Uint32 gameOverTime = SDL_GetTicks();
                handleGameOverText(pGame);

                while(!closeRequested){
                    while(SDL_PollEvent(&event)){
                        if(event.type == SDL_QUIT){
                            closeRequested = 1;
                            closeGame(pGame);
                        }
                    }
                }
                break;
            
            case START:
                renderLobby(pGame);
                if(SDL_PollEvent(&event)){
                    if(event.type == SDL_QUIT){
                        closeRequested = 1;
                    } 
                    
                    else if(event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_RETURN){
                        getInputIp(pGame);
                        
                        if(SDLNet_ResolveHost(&(pGame->serverAddress), pGame->pIp, 2000)) {
                            printf("SDLNet_ResolveHost(%s 2000): %s\n", pGame->pIp, SDLNet_GetError());
                            closeGame(pGame);
                        }

                        pGame->pPacket->address.host = pGame->serverAddress.host;
                        pGame->pPacket->address.port = pGame->serverAddress.port;

                        if(SDLNet_UDP_Recv(pGame->pSocket, pGame->pPacket)){
                            pGame->hostConnected = 1;
                            updateWithServerData(pGame);
                        }
                    }
                    else if(!joining || pGame->hostConnected == 0){
                        joining = 1;
                        clientData.command = READY;
                        clientData.clientNumber = -1;
                        memcpy(pGame->pPacket->data, &clientData, sizeof(ClientData));
                        pGame->pPacket->len = sizeof(ClientData);
                        for(int i = 0; i < 20; i++){
                            SDLNet_UDP_Send(pGame->pSocket, -1, pGame->pPacket);
                        }
                    }
                }
                if(SDLNet_UDP_Recv(pGame->pSocket, pGame->pPacket)){
                    updateWithServerData(pGame);
                    if(pGame->state == ONGOING){
                        joining = 0;
                    }
                }
                break;

        }
    }
    SDL_Delay(1000/120); // 60 FPS
    SDL_RemoveTimer(timerId);
}

void renderLobby(Game *pGame){
    SDL_RenderClear(pGame->pRenderer);

    char lobbyText[50], hostSpotText[50], spot1Text[50], spot2Text[50], spot3Text[50], spot4Text[50], inputIpText[50];

    if(pGame->hostConnected){
        snprintf(lobbyText, sizeof(lobbyText), "Lobby! You are paddle: %d", pGame->paddleNr + 1);
        snprintf(hostSpotText, sizeof(hostSpotText), "Host is connected!");
    } 
    else {
        sprintf(lobbyText, "Lobby!");
        snprintf(hostSpotText, sizeof(hostSpotText), "Host is not connected...");
    }

    if (pGame->connected[0] == true){
        snprintf(spot1Text, sizeof(spot1Text), "Player 1 is connected!");
    } else {
        snprintf(hostSpotText, sizeof(hostSpotText), "Host is not connected...");
        snprintf(spot1Text, sizeof(spot1Text), "Spot 1 is avaliable...");
    }

    if (pGame->connected[1] == true){
        snprintf(spot2Text, sizeof(spot2Text), "Player 2 is connected!");
    } else {
        snprintf(spot2Text, sizeof(spot2Text), "Spot 2 is avaliable...");
    }

    if (pGame->connected[2] == true){
        snprintf(spot3Text, sizeof(spot3Text), "Player 3 is connected!");
    } else {
        snprintf(spot3Text, sizeof(spot3Text), "Spot 3 is avaliable...");
    }

    if (pGame->connected[3] == true){
        snprintf(spot4Text, sizeof(spot4Text), "Player 4 is connected!");
    } else {
        snprintf(spot4Text, sizeof(spot4Text), "Spot 4 is avaliable...");
    }

    snprintf(inputIpText, sizeof(inputIpText), "Press Space to enter IP");


    pGame->pLobbyText = createText(pGame->pRenderer, 255, 255, 255, pGame->pFont, lobbyText, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2 - 75);

    pGame->pHostSpotText = createText(pGame->pRenderer, 255, 255, 255, pGame->pFont, hostSpotText, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2 + 25);
    pGame->pSpot1Text = createText(pGame->pRenderer, 255, 255, 255, pGame->pFont, spot1Text, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2 + 75);
    pGame->pSpot2Text = createText(pGame->pRenderer, 255, 255, 255, pGame->pFont, spot2Text, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2 + 125);
    pGame->pSpot3Text = createText(pGame->pRenderer, 255, 255, 255, pGame->pFont, spot3Text, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2 + 175);
    pGame->pSpot4Text = createText(pGame->pRenderer, 255, 255, 255, pGame->pFont, spot4Text, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2 + 225);

    pGame->pEnterIpText = createText(pGame->pRenderer, 255, 255, 255, pGame->pFont, inputIpText, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2 + 275);
    pGame->pOutPutIpText = createText(pGame->pRenderer, 255, 255, 255, pGame->pFont, pGame->pIp, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2 + 325);

    drawText(pGame->pOutPutIpText);
    drawText(pGame->pLobbyText);
    drawText(pGame->pEnterIpText);
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
    destroyText(pGame->pEnterIpText);
    destroyText(pGame->pOutPutIpText);

    SDL_RenderPresent(pGame->pRenderer);
}

void renderGame(Game *pGame){
    SDL_RenderClear(pGame->pRenderer);
    SDL_RenderCopy(pGame->pRenderer, pGame->backgroundTexture, NULL, NULL);

    int minutes = pGame->matchTime / 60000;
    int seconds = (pGame->matchTime % 60000) / 1000;

    char timeString[10], goalsTeamAString[10], goalsTeamBString[10];

    sprintf(timeString, "%02d:%02d", minutes, seconds);
    snprintf(goalsTeamAString, sizeof(goalsTeamAString), "%d", pGame->teamA);
    snprintf(goalsTeamBString, sizeof(goalsTeamBString), "%d", pGame->teamB);

    pGame->pMatchTimeText = createText(pGame->pRenderer, 255, 255, 255, pGame->pFont, timeString, WINDOW_WIDTH - 200, 20);
    pGame->pGoalsTeamAText = createText(pGame->pRenderer, 255, 255, 255, pGame->pFont, goalsTeamAString, WINDOW_WIDTH - 200, 50);
    pGame->pGoalsTeamBText = createText(pGame->pRenderer, 255, 255, 255, pGame->pFont, goalsTeamBString, WINDOW_WIDTH - 200, 80);

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
}

void updateWithServerData(Game *pGame){
    ServerData serverData;
    memcpy(&serverData, pGame->pPacket->data, sizeof(ServerData));

    pGame->paddleNr = serverData.clientNr;
    pGame->state = serverData.gState;

    for(int i = 0; i < MAX_PADDLES; i++){
        if(i != pGame->paddleNr){
            updatePaddleWithRecievedData(pGame->pPaddle[i], &(serverData.paddles[i]));
        }
        pGame->connected[i] = serverData.connected[i];
    }
    updateBallWithRecievedData(pGame->pBall, &(serverData.ball));
}

void handleInput(Game *pGame, SDL_Event *pEvent){
    ClientData clientData;
    clientData.clientNumber = pGame->paddleNr;

    if(pEvent->type == SDL_KEYDOWN){
        switch(pEvent->key.keysym.scancode){
            case SDL_SCANCODE_W:
            case SDL_SCANCODE_UP:
                updatePaddleVUp(pGame->pPaddle[pGame->paddleNr]);
                clientData.command = UP;
                break;
            case SDL_SCANCODE_S:
            case SDL_SCANCODE_DOWN:
                updatePaddleVDown(pGame->pPaddle[pGame->paddleNr]);
                clientData.command = DOWN;
                break;
            case SDL_SCANCODE_A:
            case SDL_SCANCODE_LEFT:
                updatePaddleVLeft(pGame->pPaddle[pGame->paddleNr]);
                clientData.command = LEFT;
                break;
            case SDL_SCANCODE_D:
            case SDL_SCANCODE_RIGHT:
                updatePaddleVRight(pGame->pPaddle[pGame->paddleNr]);
                clientData.command = RIGHT;
                break;
        }
        memcpy(pGame->pPacket->data, &clientData, sizeof(ClientData));
        pGame->pPacket->len = sizeof(ClientData);
        SDLNet_UDP_Send(pGame->pSocket, -1, pGame->pPacket);
    } else if (pEvent->type == SDL_KEYUP){
        bool sendUpdate = false;
        switch(pEvent->key.keysym.scancode){
            case SDL_SCANCODE_W:
            case SDL_SCANCODE_UP:
            case SDL_SCANCODE_S:
            case SDL_SCANCODE_DOWN:
                resetPaddleSpeed(pGame->pPaddle[pGame->paddleNr], 0, 1);
                clientData.command = RESET_VELOCITY_Y;
                sendUpdate = true;
                break;
            case SDL_SCANCODE_A:
            case SDL_SCANCODE_LEFT:
            case SDL_SCANCODE_D:
            case SDL_SCANCODE_RIGHT:
                resetPaddleSpeed(pGame->pPaddle[pGame->paddleNr], 1, 0);
                clientData.command = RESET_VELOCITY_X;
                sendUpdate = true;
                break;
        }
        if(sendUpdate){
            memcpy(pGame->pPacket->data, &clientData, sizeof(ClientData));
            pGame->pPacket->len = sizeof(ClientData);
            SDLNet_UDP_Send(pGame->pSocket, -1, pGame->pPacket);
        }
    }
}

void handleGameOverText(Game *pGame){
    if (pGame->teamA > pGame->teamB){
        SDL_RenderClear(pGame->pRenderer);
        drawText(pGame->pTeamAText);
        drawText(pGame->pGameOverText);
        SDL_RenderPresent(pGame->pRenderer);
    } else if (pGame->teamB > pGame->teamA){
        SDL_RenderClear(pGame->pRenderer);
        drawText(pGame->pTeamBText);
        drawText(pGame->pGameOverText);
        SDL_RenderPresent(pGame->pRenderer);
    } else {
        SDL_RenderClear(pGame->pRenderer);
        drawText(pGame->pDrawText);
        drawText(pGame->pGameOverText);
        SDL_RenderPresent(pGame->pRenderer);
    }
}

Uint32 decreaseMatchTime(Uint32 interval, void *param){
    Game *pGame = (Game *)param;
    if(pGame->matchTime > 0){
        pGame->matchTime -= 1000; // Decrease by 1 second
    } else {
        pGame->state = GAME_OVER;
    }
    return interval; // Continue the timer
}

void getInputIp(Game *pGame){
    SDL_StartTextInput();
    SDL_RenderClear(pGame->pRenderer);
    int closeRequested = 0;
    SDL_Event event;

    char input[20], inputIp[20], outputIp[20];
    strcpy(input, "");

    while(!closeRequested){
        while(SDL_PollEvent(&event)){
            if(event.type == SDL_QUIT){
                closeRequested = 1;
            } else if(event.type == SDL_TEXTINPUT){
                if(strlen(input) < sizeof(input) - 1){
                    strcat(input, event.text.text);
                }
            }
            else if(event.type == SDL_KEYDOWN){
                if(event.key.keysym.sym == SDLK_BACKSPACE && strlen(input) > 0){
                    input[strlen(input) - 1] = '\0'; // Remove last character
                }
                else if(event.key.keysym.sym == SDLK_RETURN){
                    if(strlen(input) > 0){
                        strcpy(pGame->pIp, input);
                    }
                    closeRequested = 1;
                }
            }
        }
        SDL_RenderClear(pGame->pRenderer);

        snprintf(inputIp, sizeof(inputIp), "Enter IP: ");
        pGame->pEnterIpText = createText(pGame->pRenderer, 255, 255, 255, pGame->pFont, inputIp, WINDOW_WIDTH / 2 - 100, WINDOW_HEIGHT / 2 - 50);
        drawText(pGame->pEnterIpText);
        destroyText(pGame->pEnterIpText);

        snprintf(outputIp, sizeof(outputIp), " %s", input);
        pGame->pOutPutIpText = createText(pGame->pRenderer, 255, 255, 255, pGame->pFont, outputIp, WINDOW_WIDTH / 2 - 100, WINDOW_HEIGHT / 2 - 50 + 50);
        drawText(pGame->pOutPutIpText);
        destroyText(pGame->pOutPutIpText);
        SDL_RenderPresent(pGame->pRenderer);
    }
    SDL_StopTextInput();
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

    if(pGame->pLobbyText) destroyText(pGame->pLobbyText);
    if(pGame->pSpot1Text) destroyText(pGame->pSpot1Text);
    if(pGame->pSpot2Text) destroyText(pGame->pSpot2Text);
    if(pGame->pSpot3Text) destroyText(pGame->pSpot3Text);
    if(pGame->pSpot4Text) destroyText(pGame->pSpot4Text);
    if(pGame->pHostSpotText) destroyText(pGame->pHostSpotText);
    if(pGame->pGoalsTeamAText) destroyText(pGame->pGoalsTeamAText);
    if(pGame->pGoalsTeamBText) destroyText(pGame->pGoalsTeamBText);
    if(pGame->pMatchTimeText) destroyText(pGame->pMatchTimeText);
    if(pGame->pTeamAText) destroyText(pGame->pTeamAText);
    if(pGame->pTeamBText) destroyText(pGame->pTeamBText);
    if(pGame->pIpText) destroyText(pGame->pIpText);
    if(pGame->pEnterIpText) destroyText(pGame->pEnterIpText);
    if(pGame->pFont) TTF_CloseFont(pGame->pFont);

    SDLNet_Quit();
    TTF_Quit();
    SDL_Quit();
}

