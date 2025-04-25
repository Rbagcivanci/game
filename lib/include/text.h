#ifndef text_h
#define text_h

typedef struct text Text;

Text *createText(SDL_Renderer *renderer, int r, int g, int b,TTF_Font *font, char *pString, int x, int y);
void drawText(Text *pText);
void destroyText(Text *pText);

#endif // TEXT_H