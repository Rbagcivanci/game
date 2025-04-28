SRCDIR=./src
COMMONSRCDIR=../lib/src
INCDIR=../lib/include
CC=gcc
INCLUDE = -IC:/msys64/mingw64/include/SDL2 -I$(INCDIR)
CFLAGS = -g $(INCLUDE) -c
LDFLAGS = -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_net -lm

pongClient: main.o ball.o paddle.o text.o
	$(CC) -o pongClient main.o ball.o text.o paddle.o $(LDFLAGS)

main.o: $(SRCDIR)/main.c $(INCDIR)/paddle_data.h $(INCDIR)/paddle.h $(INCDIR)/ball.h $(INCDIR)/text.h
	$(CC) $(CFLAGS) $(SRCDIR)/main.c

paddle.o: $(COMMONSRCDIR)/paddle.c $(INCDIR)/paddle_data.h $(INCDIR)/paddle.h $(INCDIR)/ball.h
	$(CC) $(CFLAGS) $(COMMONSRCDIR)/paddle.c

text.o: $(COMMONSRCDIR)/text.c $(INCDIR)/text.h
	$(CC) $(CFLAGS) $(COMMONSRCDIR)/text.c

ball.o: $(COMMONSRCDIR)/ball.c $(INCDIR)/ball.h $(INCDIR)/paddle_data.h
	$(CC) $(CFLAGS) $(COMMONSRCDIR)/ball.c

clean:
	rm *.exe
	rm *.o