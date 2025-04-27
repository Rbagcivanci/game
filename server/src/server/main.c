#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>
#include <SDL.h>
#include <SDL_timer.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_net.h>
#include <text.h>
#include <pong.h>

struct game
{
    SDL_Window *pWindow;
    SDL_Renderer *pRenderer;
    Paddle *pPaddles[MAX_PLAYERS];
    int nr_of_paddles;
    Ball *pBall;
    TTF_Font *pFont, *pScoreFont;
    Text *pOverText, *pStartText;
    GameState state;
	UDPsocket pSocket;
	UDPpacket *pPacket;
    IPaddress clients[MAX_PLAYERS];
    int nrOfClients;
    ServerData sData;
    int teamScores[2];
};

typedef struct game Game;

//prototyp

int initiate(Game *pGame);
void run(Game *pGame);
void close(Game *pGame);
void add(IPaddress address, IPaddress clients[], int *pNrOfClients);
void sendGameData(Game *pGame);
void executeCommand(game *pGame, ClientData, cData);
void setUpGame(Game *pGame);

int main(in argc, char *argv[])
{
    Game g = {0};
    if (!initiate(&g)) 
    {
        return 1;
    }
    run(&g);
    close(&g);
    return 0;
}

int initiate(Game *pGame)
{
    srand(time(NULL));
    if (SDL_Init(SDL_INIT_VIDEO || SDL_INIT_TIMER) != 0)
    {
        printf("Error %s\n", SDL_GetError());
        SDL_Quit();
        return 0;
    }
    
    if(TTF_Init() != 0)
    {
        printf("Error %s\n", SDL_GetError());
        SDL_Quit();
        return 0;
    }

    if(SDLNet_Init())
    {
        printf("Error %s\n", SDL_GetError());
        TTF_Quit();
        SDL_Quit();
        return 0;
    }

    pGame -> pWindow = SDL_CreateWindow("Crazy pong server", SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    if(!pGame->pWindow)
    {
        printf("Error: %s\n", TTF_GetError());
        close(pGame);
        return 0;
    }

    pGame -> pRenderer = SDL_CreateRenderer(pGame -> pWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if(!pGame -> pRenderer)
    {
        printf("Error: %s\n", SDL_GetError());
        close(pGame);
        return 0;
    }

    pGame -> pFont = TTF_OpenFont("C:/Windows/Fonts/arial.ttf", 100);
    pGame -> pScoreFont = TTF_OpenFont("C:/Windows/Fonts/arial.ttf", 24);
    if(!pGame -> pFont || !pGame -> pScoreFont)
    {
        printf("Error: %s\n", TTF_GetError());
        close(pGame);
        return 0;
    }

    pGame -> pSocket = SDLNet_UDP_Open(2000);
    if(!(pGame -> pSocket = SDLNet_UDP_Open(2000)))
    {
        printf("SDLNet_UDP_Open: %s\n", SDLNet_GetError());
        close(pGame);
        return 0;
    }

    pGame -> pPacket = SDLNet_AllocPacket(512);
    if(!(pGame -> pPacket = SDLNet_AllocPacket(512)))
    {
        printf("SDLNet_AllocPacket: %s\n", SDLNet_GetError());
        close(pGame);
        return 0;

    }

    // Create game object
    for(int i = 0; i < MAX_PLAYERS; i++)
    {
        pGame -> pPaddle[i] = createPaddle(i, pGame -> pRenderer, WINDOW_WIDTH, WINDOW_HEIGHT);
        if (!pGame -> pPaddle[i])
        {
            printf("CreatePaddle Error for player %s\n", i);
            close(pGame);
            return 0;
        }
    }

    pGame ->nr_of_paddles = 0;
    pGame -> pBall = createBall(WINDOW_WIDTH, WINDOW_HEIGHT);
    if(!pGame -> pBall)
    {
        printf("createBall Error\n");
        close(pGame);
        return 0;
    }

    //initialize game
    pGame -> state = START;
    pGame -> nrOfClients = 0;
    pGame -> teamScores[0] = 0;
    pGame -> teamScores[1] = 0;

    return 1;

}

void run(Game *pGame)
{
    int close_requested = 0;
    SDL_Event event;
    ClientData cData;
    const int score_limit = 5;

    while(!close_requested)
    {
        switch(pGame -> state)
        {
            case ONGOING:
                sendGameData(pGame);
                while(SDLNet_UDP_Recv(pGame -> pSocket, pGame -> pPacket) == 1)
                {
                    memcpy(&cData, pGame -> pPacket -> data, sizeof(ClientData));
                    executeCommand(pGame, cData);
                }

                if(SDL_PollEvent(&event))
                {
                    if(event.type == SDL_QUIT)
                    {
                        close_requested = 1;
                    }
                }

                for(int i = 0; i < pGame -> nr_of_paddles; i++)
                {
                    updatePaddle(pGame -> pPaddle[i], WINDOW_HEIGHT);
                }

                updateBall(pGame -> pBall, pGame -> pPaddle, pGame -> nr_of_paddles, pGame -> teamScores, WINDOW_WIDTH, WINDOW_HEIGHT);

                if(pGame -> teamScores[0] >= score_limit || pGame -> teamScores[1] >= score_limit)
                {
                    pGame -> state = GAME_OVER;
                }

                SDL_SetRenderDrawColor(pGame -> pRenderer, 0, 0, 0, 225);

                for(int i = 0; i < pGame -> nr_of_paddles; i++)
                {
                    drawPaddle(pGame -> pPaddle[i], pGame -> pRenderer);
                }
                
                drawBall(pGame -> pBall, pGame -> pRenderer);

                char scoreText[32];
                snprintf(scoreText, sizeof(scoreText), "%d - %d", pGame -> teamScores[0], pGame -> teamScores[1]);
                Text *scoreDisplay = createText(pGame -> pRenderer, 238, 168, 65, pGame -> pScoreFont, scoreText, WINDOW_WIDTH / 2, 50);
                drawText(scoreDisplay);
                destroyText(scoreDisplay);
                SDL_RenderPresent(pGame -> pRenderer);
                break;

            case GAME_OVER:
                drawText(pGame -> pOverText);
                sendGameData(pGame);
                if(pGame -> nrOfClients == MAX_PLAYERS)
                {
                    pGame -> nrOfClients = 0;
                }

                if(SDL_PollEvent(&event) && event.type == SDL_QUIT)
                {
                    close_requested = 1;
                }
                break;

            case START:
                drawText(pGame -> pStartText);
                SDL_RenderPresent(pGame -> pRenderer);
                if(SDL_PollEvent(&event) && event.type == SDL_QUIT)
                {
                    close_requested = 1;
                }
                if(SDLNet_UDP_Recv(pGame -> pSocket, pGame -> pPacket) == 1)
                {
                    add(pGame -> pPacket -> address, pGame -> clients, &(pGame -> nrOfClients));
                    pGame -> nr_of_paddles = pGame -> nrOfClients;

                    if(pGame -> nrOfClients == MAX_PLAYERS)
                    {
                        setUpGame(pGame);
                    }
                }
                break;

            default:
                break;        
        }
        SDL_Delay(1000/60); //60 fps spel
    }
}

void setUpGame(Game *pGame)
{
    for(int i = 0; i < MAX_PLAYERS; i++)
    {
        resetPaddle(pGame -> pPaddle[i], WINDOW_WIDTH, WINDOW_HEIGHT);
        resetBall(pGame -> pBall, WINDOW_WIDTH, WINDOW_HEIGHT);
        pGame -> nr_of_paddles = MAX_PLAYERS;
        pGame -> teamScores[0] = 0;
        pGame -> teamScores[1] = 0;
        pGame -> state = ONGOING;
    }

    pGame -> sData.ball_x = pGame -> pBall -> x;
    pGame -> sData.ball_y = pGame -> pBall -> y;
    pGame -> sData.ball_vx = pGame -> pBall -> vx;
    pGame -> sData.ball.vy = pGame -> pBall -> vy;
    pGame -> sData.teamScores[0] = pGame -> teamScores[0];
    pGame -> sData.teamScores[1] = pGame -> teamScores[1];

    for(int i = 0; i < pGame -> nrOfClients; i++)
    {
        pGame -> sData.playerNr = i;
        memcpy(pGame -> pPacket -> data, &(pGame -> sData), sizeof(ServerData));
        pGame -> pPacket -> len = sizeof(ServerData);
        pGame -> pPacket -> address = pGame -> clients[i];
        SDLNet_UDP_Send(pGame -> pSocket, -1, pGame -> pPacket);
    }
}

void add(IPaddress address, IPaddress clients[], int *pNrOfClients)
{
    for(int i = 0; i < *pNrOfClients; i++)
    {
        if(address.host == clients[i].host && address.port == clients[i].port)
        {
            return;
        }
    }
    clients[*pNrOfClients] = address;
    (*pNrOfClients)++;
}

void executeCommand(Game *pGame, ClientData cData)
{
    if(cData.playerNumber < 0 || cData.playerNumber >= MAX_PLAYERS)
    {
        return ;
    }

    switch(cData.comman)
    {
        case MOVE_UP:
            moveUp(pGame -> pPaddle[cData.playerNumber]);
            break;
        case MOVE_DOWN:
            moveDown(pGame -> pPaddle[cData.playerNumber]);
            break;
    }
}

void close(Game *pGame)
{
    for(int i = 0; i < MAX_PLAYERS; i++)
    {
        if(pGame -> pPaddle[i])
        {
            destroyPaddle(pGame -> pPaddle[i]);
        }
    }
    if(pGame -> pBall) destroyBall(pGame -> pBall);
    if(pGame -> pRenderer) SDL_DestroyRenderer(pGame -> pRenderer);
    if(pGame -> pWindow) SDL_DestroyWindow(pGame -> pWindow);
    if(pGame -> pOverText) destroyText(pGame -> pOverText);
    if(pGame -> pStartText) destroyText(pGame -> pStartText);
    if(pGame -> pFont) TTF_CloseFont(pGame -> pFont);
    if(pGame -> pScoreFont) TTF_CloseFont(pGame ->pScoreFont);
    if(pGame -> pPacket) SDLNet_FreePacket(pGame -> pPacket);
    if(pGame -> pSocket) SDLNet_UDP_Close(pGame -> pSocket);
    SDLNet_Quit;
    TTF_Quit;
    SDL_Quit;

}
