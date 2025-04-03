#include <SDL.h>
#include <stdio.h>
#include <stdbool.h>

int main(int argv, char** args){
    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_Window *window = SDL_CreateWindow("Hello SDL", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1000 /*Bredd*/, 100 /*Höjd*/, 0);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 0);

    bool isRunning = true;
    SDL_Event event;
    
    while(isRunning){
        while(SDL_PollEvent(&event)){
            switch (event.type)
            {
                case SDL_QUIT: isRunning = false;
            }
        }
        SDL_RenderClear(renderer);
        SDL_SetRenderDrawColor(renderer, 110, 0, 0, 110);
        SDL_RenderPresent(renderer);
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}