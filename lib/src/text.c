#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "text.h"

struct text {
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    SDL_Rect rect;
};

Text *createText(SDL_Renderer *renderer, int r, int g, int b, TTF_Font *font, const char *text, int x, int y){
    Text *pText = (Text *)malloc(sizeof(Text));
    pText->pRenderer=pRenderer;
    SDL_Color color = { r, g, b };
    SDL_Surface *surface = TTF_RenderText_Solid(pFont, pString, color);
    if (!pSurface){
        printf("Error: %s\n", SDL_GetError());
        return NULL;
    }
    pText->pTexture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    if (!pText->pTexture) {
        printf("Error: %s\n", SDL_GetError());
        return NULL;
    }

    SDL_QueryTexture(pText->texture, NULL, NULL, &pText->rect.w, &pText->rect.h);
    pText->rect.x = x - pText->rect.w / 2;
    pText->rect.y = y - pText->rect.h / 2;

    return pText;
}

void drawText(Text *pText) {
    SDL_RenderCopy(pText->renderer, pText->texture, NULL, &pText->rect);
}

void destroyText(Text *pText) {
    SDL_DestroyTexture(pText->texture);
    free(pText);
}


