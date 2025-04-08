#ifndef text_h
#define text_h

typedef struct text Text

Text *createText(SDL_Renderer *renderer, TTF_Font *font, const char *text, SDL_Color color);
void drawText(Text *pText);
void destroyText(Text *pText);

#endif // TEXT_H