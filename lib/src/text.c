#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "text.h"

struct text {
    SDL_Renderer *renderer;
    TTF_Font *font;
    SDL_Color color;
    SDL_Texture *texture;
    SDL_Rect rect;
};

Text *createText(SDL_Renderer *renderer, TTF_Font *font, const char *text, SDL_Color color) {
    Text *pText = (Text *)malloc(sizeof(Text));
    if (!pText) {
        return NULL;
    }

    pText->renderer = renderer;
    pText->font = font;
    pText->color = color;

    SDL_Surface *surface = TTF_RenderText_Solid(font, text, color);
    if (!surface) {
        free(pText);
        return NULL;
    }

    pText->texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!pText->texture) {
        SDL_FreeSurface(surface);
        free(pText);
        return NULL;
    }

    pText->rect.w = surface->w;
    pText->rect.h = surface->h;
    SDL_FreeSurface(surface);

    return pText;
}

void drawText(Text *pText) {
    if (pText && pText->texture) {
        SDL_RenderCopy(pText->renderer, pText->texture, NULL, &pText->rect);
    }
}

void destroyText(Text *pText) {
    if (pText) {
        if (pText->texture) {
            SDL_DestroyTexture(pText->texture);
        }
        free(pText);
    }
}


