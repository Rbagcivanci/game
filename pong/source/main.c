#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include <stdio.h>
#include <stdbool.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define PADDLE_WIDTH 10
#define PADDLE_HEIGHT 60
#define BALL_SIZE 10
#define FPS 60
#define FRAME_TIME (1000.0f / FPS)

typedef enum
{
    STATE_LOBBY,
    STATE_PLAYING
} GameState;

int main(int argc, char **argv)
{
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
    {
        printf("SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    if (TTF_Init() < 0)
    {
        printf("TTF_Init failed: %s\n", TTF_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow("Pong", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    if (!window)
    {
        printf("SDL_CreateWindow failed: %s\n", SDL_GetError());
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 0);
    if (!renderer)
    {
        printf("SDL_CreateRenderer failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    TTF_Font *font = TTF_OpenFont("C:/Windows/Fonts/arial.ttf", 24);
    if (!font)
    {
        printf("TTF_OpenFont failed: %s\n", TTF_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    // musiken
    Mix_Music *music;
    Mix_Chunk *soundEffect;
    int frequency = MIX_DEFAULT_FREQUENCY;
    Uint16 format = MIX_DEFAULT_FORMAT;
    int nchannels = 2;
    int chunksize = 4096;

    if (Mix_OpenAudio(frequency, format, nchannels, chunksize) != 0)
    {
        printf("Mix_OpenAudio faild: %s\n", Mix_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    music = Mix_LoadMUS("../lib/resources/GameMusic.mp3");
    Mix_VolumeMusic(8);
    if (!music)
    {
        printf("Mix_LoadMUS failed: %s\n", Mix_GetError());
    }
    else
    {
        Mix_PlayMusic(music, -1); // -1 = loopa musiken
        printf("musiken Spelas!");
    }

    soundEffect = Mix_LoadWAV("../lib/resources/Boom.mp3");
    if (!soundEffect)
    {
        printf("Mix_LoadWAV failed: %s\n", Mix_GetError());
    }

    // Game objects
    SDL_Rect leftPaddle = {20, WINDOW_HEIGHT / 2 - PADDLE_HEIGHT / 2, PADDLE_WIDTH, PADDLE_HEIGHT};
    SDL_Rect rightPaddle = {WINDOW_WIDTH - 20 - PADDLE_WIDTH, WINDOW_HEIGHT / 2 - PADDLE_HEIGHT / 2, PADDLE_WIDTH, PADDLE_HEIGHT};
    float ballX = WINDOW_WIDTH / 2 - BALL_SIZE / 2, ballY = WINDOW_HEIGHT / 2 - BALL_SIZE / 2;
    SDL_Rect ball = {(int)ballX, (int)ballY, BALL_SIZE, BALL_SIZE};
    float ballVelX = 5.1f, ballVelY = 5.1f;
    int leftScore = 0, rightScore = 0;

    bool isRunning = true;
    GameState gameState = STATE_LOBBY;
    SDL_Event event;
    Uint32 frameStart;

    while (isRunning)
    {
        frameStart = SDL_GetTicks();

        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                isRunning = false;
            }
            if (event.type == SDL_KEYDOWN)
            {
                if (gameState == STATE_LOBBY && event.key.keysym.sym == SDLK_SPACE)
                {
                    gameState = STATE_PLAYING;
                    Mix_HaltMusic(); // Stänger av musiken
                }
            }
        }

        if (gameState == STATE_PLAYING)
        {
            // Paddle movement
            const Uint8 *state = SDL_GetKeyboardState(NULL);
            if (state[SDL_SCANCODE_W] && leftPaddle.y > 0)
                leftPaddle.y -= 5;
            if (state[SDL_SCANCODE_S] && leftPaddle.y < WINDOW_HEIGHT - PADDLE_HEIGHT)
                leftPaddle.y += 5;
            rightPaddle.y = ball.y - PADDLE_HEIGHT / 2;

            // Ball movement
            ballX += ballVelX;
            ballY += ballVelY;
            ball.x = (int)ballX;
            ball.y = (int)ballY;

            // Ball collision with top/bottom
            if (ball.y <= 0 || ball.y >= WINDOW_HEIGHT - BALL_SIZE)
            {
                ballVelY = -ballVelY;
                Mix_PlayChannel(-1, soundEffect, 0);
            }

            // Ball collision with paddles
            if (SDL_HasIntersection(&ball, &leftPaddle) || SDL_HasIntersection(&ball, &rightPaddle))
            {
                ballVelX = -ballVelX;
                Mix_PlayChannel(-1, soundEffect, 0);
            }

            // Scoring and game end condition
            if (ball.x < 0)
            {
                rightScore++;
                if (rightScore >= 3)
                {
                    printf("Right Player Wins! Final Score: %d - %d\n", leftScore, rightScore);
                }
                ballX = WINDOW_WIDTH / 2 - BALL_SIZE / 2;
                ballY = WINDOW_HEIGHT / 2 - BALL_SIZE / 2;
                ball.x = (int)ballX;
                ball.y = (int)ballY;
            }

            else if (ball.x > WINDOW_WIDTH - BALL_SIZE)
            {
                leftScore++;
                if (leftScore >= 3)
                {
                    printf("Left Player Wins! Final Score: %d - %d\n", leftScore, rightScore);
                }
                ballX = WINDOW_WIDTH / 2 - BALL_SIZE / 2;
                ballY = WINDOW_HEIGHT / 2 - BALL_SIZE / 2;
                ball.x = (int)ballX;
                ball.y = (int)ballY;
            }
        }

        // Rendering
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        if (gameState == STATE_LOBBY)
        {
            SDL_Color white = {255, 255, 255, 255};
            SDL_Surface *surface = TTF_RenderText_Solid(font, "Tryck SPACE for att starta", white);
            SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
            SDL_Rect textRect = {
                (WINDOW_WIDTH - surface->w) / 2,
                (WINDOW_HEIGHT - surface->h) / 2,
                surface->w,
                surface->h};
            SDL_RenderCopy(renderer, texture, NULL, &textRect);
            SDL_FreeSurface(surface);
            SDL_DestroyTexture(texture);
        }

        else
        {
            // Spelet är igång, rita paddles och boll
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderFillRect(renderer, &leftPaddle);
            SDL_RenderFillRect(renderer, &rightPaddle);
            SDL_RenderFillRect(renderer, &ball);

            char scoreText[20];
            sprintf(scoreText, "%d - %d", leftScore, rightScore);
            SDL_Color white = {255, 255, 255, 255};
            SDL_Surface *scoreSurface = TTF_RenderText_Solid(font, scoreText, white);
            SDL_Texture *scoreTexture = SDL_CreateTextureFromSurface(renderer, scoreSurface);
            SDL_Rect scoreRect = {WINDOW_WIDTH / 2 - 50, 20, scoreSurface->w, scoreSurface->h};
            SDL_RenderCopy(renderer, scoreTexture, NULL, &scoreRect);
            SDL_FreeSurface(scoreSurface);
            SDL_DestroyTexture(scoreTexture);
        }

        SDL_RenderPresent(renderer);

        // Cap frame rate
        Uint32 frameTime = SDL_GetTicks() - frameStart;
        if (frameTime < FRAME_TIME)
        {
            SDL_Delay((Uint32)(FRAME_TIME - frameTime));
        }
    }

    // Stänger ned allt
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    Mix_FreeMusic(music);
    Mix_CloseAudio();
    TTF_Quit();
    SDL_Quit();

    return 0;
}