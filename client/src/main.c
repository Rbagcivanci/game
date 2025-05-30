#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <SDL.h>
#include <SDL_main.h>
#include <SDL_timer.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_net.h>
#include <SDL_mixer.h>
#include "paddle_data.h"
#include "paddle.h"
#include "ball.h"
#include "text.h"
#include "obstacles.h"
#define MOVEMENT_SPEED 5

typedef struct game {
    SDL_Window *pWindow;
    SDL_Renderer *pRenderer;
    SDL_Surface *pBackgroundSurface, *pIpSurface, *pLobbySurface, *pGameOverSurface;
    SDL_Texture *backgroundTexture, *pIpTexture, *pLobbyTexture, *pGameOverTexture;
    TTF_Font *pFont, *pScoreFont, *pGameOverFont;
    Text *pLobbyText, *pEnterIpText, *pOutPutIpText, *pIpText, *pTeamAText, *pTeamBText, *pDrawText, *pGameOverText, *pGoalsTeamAText, *pGoalsTeamBText, *pHostSpotText, *pSpot1Text, *pSpot2Text, *pSpot3Text, *pSpot4Text;
    Paddle *pPaddle[MAX_PADDLES];
    int nrOfPaddles, paddleNr;
    Ball *pBall;
    GameState state;
    ClientData clients[MAX_PADDLES];
    bool connected[MAX_PADDLES];
    bool hostConnected;
    char pIp[50];
    int teamScores[2];
    Mix_Music *lobbySoundtrack;
    UDPsocket pSocket;
    IPaddress serverAddress;
    UDPpacket *pPacket;
}Game;

int initiate(Game *pGame);
void run(Game *pGame);
void closeGame(Game *pGame);
void handleInput(Game *pGame, SDL_Event *pEvent);
void updateWithServerData(Game *pGame);
void renderGame(Game *pGame);
void renderLobby(Game *pGame);
void getInputIp(Game *pGame);
void handleGameOverText(Game *pGame);

int main(int argc, char *argv[]) {
    Game g = {0};
    if(!initiate(&g)) return 1;
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

    pGame->pWindow = SDL_CreateWindow("Pong Client", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
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

    pGame->pFont = TTF_OpenFont("../lib/resources/Symtext.ttf", 50);
    pGame->pScoreFont = TTF_OpenFont("../lib/resources/Symtext.ttf", 30);
    pGame->pGameOverFont = TTF_OpenFont("../lib/resources/Symtext.ttf", 100);
    if(!pGame->pFont || !pGame->pScoreFont || !pGame->pGameOverFont){
        printf("Error: %s\n",TTF_GetError());
        closeGame(pGame);
        return 0;
    }

    if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0){
        printf("SDL_mixer Error: %s\n", Mix_GetError());
        return 0;
    }

    pGame->lobbySoundtrack = Mix_LoadMUS("../lib/resources/lobbyMusik.mp3");
    if (!pGame->lobbySoundtrack) {
        printf("Kunde inte ladda upp lobby musik: %s\n", Mix_GetError());
        return 0;
    }

    if (!(pGame->pSocket = SDLNet_UDP_Open(0))) {
        printf("SDLNet_UDP_Open misslyckades: %s\n", SDLNet_GetError());
        return 0;
    }

    if(SDLNet_ResolveHost(&(pGame->serverAddress), "127.0.0.1", 2000)) {
        printf("SDLNet_ResolveHost(%s 2000): %s\n",pGame->pIp, SDLNet_GetError());
        return 0;
    }

    if(!(pGame->pPacket = SDLNet_AllocPacket(512))){
        printf("SDLNet_AllocPacket: %s\n",SDLNet_GetError());
        return 0;
    }

    pGame->pPacket->address.host = pGame->serverAddress.host;
    pGame->pPacket->address.port = pGame->serverAddress.port;

    pGame->pBackgroundSurface = IMG_Load("../lib/resources/backgroundMain.png");
    pGame->pIpSurface = IMG_Load("../lib/resources/LobbyIpBackground.png");
    pGame->pLobbySurface = IMG_Load("../lib/resources/LobbyIpBackground.png");
    if(!pGame->pBackgroundSurface || !pGame->pIpSurface || !pGame->pLobbySurface){
        printf("Error: %s\n",SDL_GetError());
        closeGame(pGame);
        return 0;
    }

    pGame->backgroundTexture = SDL_CreateTextureFromSurface(pGame->pRenderer, pGame->pBackgroundSurface);
    SDL_FreeSurface(pGame->pBackgroundSurface);
    pGame->pIpTexture = SDL_CreateTextureFromSurface(pGame->pRenderer, pGame->pIpSurface);
    SDL_FreeSurface(pGame->pIpSurface);
    pGame->pLobbyTexture = SDL_CreateTextureFromSurface(pGame->pRenderer, pGame->pLobbySurface);
    SDL_FreeSurface(pGame->pLobbySurface);
    if(!pGame->backgroundTexture || !pGame->pIpTexture || !pGame->pLobbyTexture){
        printf("Error: %s\n",SDL_GetError());
        closeGame(pGame);
        return 0;
    }

    for (int i = 0; i < MAX_PADDLES; i++) pGame->pPaddle[i] = createPaddle( pGame->pRenderer, WINDOW_WIDTH, WINDOW_HEIGHT, i);

    for(int i = 0; i < MAX_PADDLES; i++) pGame->connected[i] = false;

    pGame->nrOfPaddles = MAX_PADDLES;

    pGame->pBall = createBall(pGame->pRenderer);
    if(!pGame->pBall){
        printf("Failed to create ball\n");
        closeGame(pGame);
        return 0;
    }

    pGame->pTeamAText = createText(pGame->pRenderer, 255, 255, 255, pGame->pFont, "Team A Won", WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2 + 150);
    pGame->pTeamBText = createText(pGame->pRenderer, 255, 255, 255, pGame->pFont, "Team B Won", WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2 + 150);
    pGame->pDrawText = createText(pGame->pRenderer, 255, 255, 255, pGame->pFont, "Draw", WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2 + 150);
    pGame->pGameOverText = createText(pGame->pRenderer, 255, 255, 255, pGame->pGameOverFont, "Game Over", WINDOW_WIDTH / 2, 
    WINDOW_HEIGHT / 2);

    for(int i=0;i<MAX_PADDLES;i++){
        if(!pGame->pPaddle[i]){
            printf("Error: %s\n", SDL_GetError());
            return 0;
        }
    }

    if(!pGame->pTeamAText || !pGame->pTeamBText || !pGame->pDrawText || !pGame->pGameOverText){
        printf("Error: %s\n",SDL_GetError());
        closeGame(pGame);
        return 0;
    }

    pGame->teamScores[0] = 0;
    pGame->teamScores[1] = 0;
    pGame->state = START;

    return 1;
}

void run(Game *pGame){
    int closeRequested = 0;
    SDL_Event event;
    ClientData clientData;
    int joining = 0;
    pGame->hostConnected = false;
    strcpy(pGame->pIp, "Nothing yet...");

    while(!closeRequested){
        switch(pGame->state){
            case ONGOING:
                Mix_HaltMusic();

                while(SDLNet_UDP_Recv(pGame->pSocket, pGame->pPacket)) updateWithServerData(pGame);

                if(SDL_PollEvent(&event)){
                    if(event.type == SDL_QUIT){
                        closeRequested = 1;
                    } else handleInput(pGame, &event);
                }

                /*for(int i = 0; i < MAX_PADDLES; i++) updatePaddlePosition(pGame->pPaddle[i]);
                updateBallPosition(pGame->pBall);*/
                renderGame(pGame);
                SDL_RenderPresent(pGame->pRenderer);
                break;
            
            case GAME_OVER:
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
                if (!Mix_PlayingMusic()) {
                    Mix_PlayMusic(pGame->lobbySoundtrack, -1) == -1; {
                        printf("Kunde inte spela upp musiken: %s\n", Mix_GetError);
                    }
                }
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
                        joining = 1;
                        clientData.command = READY;
                        clientData.clientNumber = -1;
                        memcpy(pGame->pPacket->data, &clientData, sizeof(ClientData));
                        pGame->pPacket->len = sizeof(ClientData);
                        for(int i = 0; i < 20; i++) SDLNet_UDP_Send(pGame->pSocket, -1, pGame->pPacket);
                        
                        pGame->pPacket->address.host = pGame->serverAddress.host;
                        pGame->pPacket->address.port = pGame->serverAddress.port;

                        if(SDLNet_UDP_Recv(pGame->pSocket, pGame->pPacket)){
                            pGame->hostConnected = true;
                            updateWithServerData(pGame);
                        }
                    }
                }
                while(SDLNet_UDP_Recv(pGame->pSocket, pGame->pPacket)){
                    updateWithServerData(pGame);
                    if(pGame->state == ONGOING){
                        joining = 0;
                    }
                }
                break;
        }
        SDL_Delay(1000/60); // 60 FPS
    }
}

void renderLobby(Game *pGame){
    SDL_RenderClear(pGame->pRenderer);
    SDL_RenderCopy(pGame->pRenderer, pGame->pLobbyTexture, NULL, NULL);

    char lobbyText[50], hostSpotText[50], spot1Text[50], spot2Text[50], spot3Text[50], spot4Text[50], inputIpText[50];

    if(pGame->hostConnected && pGame->paddleNr >= 0){
        snprintf(lobbyText, sizeof(lobbyText), "Lobby! You are paddle: %d", pGame->paddleNr + 1);
    } 
    else {
        snprintf(lobbyText, sizeof(lobbyText), "Lobby! Waiting for paddle assignment");
    }

    snprintf(hostSpotText, sizeof(hostSpotText), pGame->hostConnected ? "Host is connected!" : "Host is not connected...");

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
    snprintf(inputIpText, sizeof(inputIpText), "Press Enter to enter IP");

    pGame->pLobbyText = createText(pGame->pRenderer, 255, 255, 255, pGame->pFont, lobbyText, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2 - 75);
    pGame->pHostSpotText = createText(pGame->pRenderer, 80, 200, 190, pGame->pFont, hostSpotText, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2 + 25);
    pGame->pSpot1Text = createText(pGame->pRenderer, 80, 200, 190, pGame->pFont, spot1Text, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2 + 75);
    pGame->pSpot2Text = createText(pGame->pRenderer, 80, 200, 190, pGame->pFont, spot2Text, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2 + 125);
    pGame->pSpot3Text = createText(pGame->pRenderer, 80, 200, 190, pGame->pFont, spot3Text, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2 + 175);
    pGame->pSpot4Text = createText(pGame->pRenderer, 80, 200, 190, pGame->pFont, spot4Text, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2 + 225);
    pGame->pEnterIpText = createText(pGame->pRenderer, 255, 255, 255, pGame->pFont, inputIpText, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2 + 275);
    pGame->pOutPutIpText = createText(pGame->pRenderer, 80, 200, 190, pGame->pFont, pGame->pIp, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2 + 325);

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

    char goalsTeamAString[10], goalsTeamBString[10];

    snprintf(goalsTeamAString, sizeof(goalsTeamAString), "%d", pGame->teamScores[0]);
    snprintf(goalsTeamBString, sizeof(goalsTeamBString), "%d", pGame->teamScores[1]);

    pGame->pGoalsTeamAText = createText(pGame->pRenderer, 255, 255, 255, pGame->pScoreFont, goalsTeamAString, (WINDOW_WIDTH / 2) - 50 , 50);
    pGame->pGoalsTeamBText = createText(pGame->pRenderer, 255, 255, 255, pGame->pScoreFont, goalsTeamBString, (WINDOW_WIDTH / 2) + 50, 50);

    drawText(pGame->pGoalsTeamAText);
    drawText(pGame->pGoalsTeamBText);
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
}

void updateWithServerData(Game *pGame){
    ServerData serverData;
    memcpy(&serverData, pGame->pPacket->data, sizeof(ServerData));
    pGame->paddleNr = serverData.clientNr;
    pGame->state = serverData.gState;
    pGame->teamScores[0]=serverData.teamScores[0];
    pGame->teamScores[1]=serverData.teamScores[1];
    pGame->hostConnected = serverData.hostConnected;

    for(int i = 0; i < MAX_PADDLES; i++)
    {
        updatePaddleWithRecievedData(pGame->pPaddle[i], &(serverData.paddles[i]));
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
                updatePaddleVUp(pGame->pPaddle[clientData.clientNumber]);
                clientData.command = UP;
                break;
            case SDL_SCANCODE_A:
            case SDL_SCANCODE_LEFT:
                updatePaddleVLeft(pGame->pPaddle[clientData.clientNumber]);
                clientData.command = LEFT;
                break;
            case SDL_SCANCODE_D:
            case SDL_SCANCODE_RIGHT:
                updatePaddleVRight(pGame->pPaddle[clientData.clientNumber]);
                clientData.command = RIGHT;
                break;
            case SDL_SCANCODE_S:
            case SDL_SCANCODE_DOWN:
                updatePaddleVDown(pGame->pPaddle[clientData.clientNumber]);
                clientData.command = DOWN;
                break;
        }
        memcpy(pGame->pPacket->data, &clientData, sizeof(ClientData));
		pGame->pPacket->len = sizeof(ClientData);
        SDLNet_UDP_Send(pGame->pSocket, -1,pGame->pPacket);
    }

    else if (pEvent->type == SDL_KEYUP) {
        bool sendReset = false;
        switch (pEvent->key.keysym.scancode) {
            case SDL_SCANCODE_W:
            case SDL_SCANCODE_UP:
            case SDL_SCANCODE_S:
            case SDL_SCANCODE_DOWN:
                if (!SDL_GetKeyboardState(NULL)[SDL_SCANCODE_W] && !SDL_GetKeyboardState(NULL)[SDL_SCANCODE_UP] && !SDL_GetKeyboardState(NULL)[SDL_SCANCODE_S] && !SDL_GetKeyboardState(NULL)[SDL_SCANCODE_DOWN]) {
                    resetPaddleSpeed(pGame->pPaddle[clientData.clientNumber], 0, 1);
                    clientData.command = RESET_VELOCITY_Y;
                    sendReset = true;
                }
                break;
            case SDL_SCANCODE_A:
            case SDL_SCANCODE_LEFT:
            case SDL_SCANCODE_D:
            case SDL_SCANCODE_RIGHT:
                if (!SDL_GetKeyboardState(NULL)[SDL_SCANCODE_A] && !SDL_GetKeyboardState(NULL)[SDL_SCANCODE_LEFT] && !SDL_GetKeyboardState(NULL)[SDL_SCANCODE_D] && !SDL_GetKeyboardState(NULL)[SDL_SCANCODE_RIGHT]) {
                    resetPaddleSpeed(pGame->pPaddle[clientData.clientNumber], 1, 0);
                    clientData.command = RESET_VELOCITY_X;
                    sendReset = true;
                }
                break;
        }
        if (sendReset) {
            memcpy(pGame->pPacket->data, &clientData, sizeof(ClientData));
            pGame->pPacket->len = sizeof(ClientData);
            SDLNet_UDP_Send(pGame->pSocket, -1, pGame->pPacket);
        }
    }
}

void handleGameOverText(Game *pGame){
    if (pGame->teamScores[0] > pGame->teamScores[1]){
        SDL_RenderClear(pGame->pRenderer);
        drawText(pGame->pTeamAText);
        drawText(pGame->pGameOverText);
        SDL_RenderPresent(pGame->pRenderer);
    } else if (pGame->teamScores[1] > pGame->teamScores[0]){
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
                    input[strlen(input) - 1] = '\0';
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
        SDL_RenderCopy(pGame->pRenderer, pGame->pLobbyTexture, NULL, NULL);

        snprintf(inputIp, sizeof(inputIp), "Enter IP: ");
        pGame->pEnterIpText = createText(pGame->pRenderer, 255, 255, 255, pGame->pFont, inputIp, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2 - 50);
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
    if (pGame->pBall) destroyBall(pGame->pBall);
    if (pGame->pRenderer) SDL_DestroyRenderer(pGame->pRenderer);
    if (pGame->pWindow) SDL_DestroyWindow(pGame->pWindow);
    if (pGame->backgroundTexture) SDL_DestroyTexture(pGame->backgroundTexture);
    if (pGame->pIpTexture) SDL_DestroyTexture(pGame->pIpTexture);
    if(pGame->pLobbyText) destroyText(pGame->pLobbyText);
    if(pGame->pSpot1Text) destroyText(pGame->pSpot1Text);
    if(pGame->pSpot2Text) destroyText(pGame->pSpot2Text);
    if(pGame->pSpot3Text) destroyText(pGame->pSpot3Text);
    if(pGame->pSpot4Text) destroyText(pGame->pSpot4Text);
    if(pGame->pHostSpotText) destroyText(pGame->pHostSpotText);
    if(pGame->pGoalsTeamAText) destroyText(pGame->pGoalsTeamAText);
    if(pGame->pGoalsTeamBText) destroyText(pGame->pGoalsTeamBText);
    if(pGame->pTeamAText) destroyText(pGame->pTeamAText);
    if(pGame->pTeamBText) destroyText(pGame->pTeamBText);
    if(pGame->pIpText) destroyText(pGame->pIpText);
    if(pGame->pEnterIpText) destroyText(pGame->pEnterIpText);
    if(pGame->pFont) TTF_CloseFont(pGame->pFont);
    if(pGame->lobbySoundtrack) Mix_FreeMusic(pGame->lobbySoundtrack);

    Mix_CloseAudio();
    SDLNet_Quit();
    TTF_Quit();
    SDL_Quit();
}