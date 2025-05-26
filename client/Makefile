SRCDIR=./src
COMMONSRCDIR=../lib/src
INCDIR=../lib/include
CC=gcc
<<<<<<< HEAD
INCLUDE=-IC:/msys64/mingw64/include/SDL2 -I$(INCDIR)
CFLAGS=-g $(INCLUDE) -c
LDFLAGS=-lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_net -lm
=======
INCLUDE = -IC:/msys64/mingw64/include/SDL2 -I$(INCDIR)
CFLAGS = -g $(INCLUDE) -c
LDFLAGS = -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_net -lSDL2_mixer -lm
>>>>>>> 05b5cd4f38921a10c86dc01d9e90af2c9f13314f

pongClient: main.o ball.o paddle.o text.o obstacles.o
	$(CC) -o pongClient main.o ball.o paddle.o text.o obstacles.o $(LDFLAGS)

main.o: $(SRCDIR)/main.c $(INCDIR)/paddle_data.h $(INCDIR)/paddle.h $(INCDIR)/ball.h $(INCDIR)/text.h $(INCDIR)/obstacles.h
	$(CC) $(CFLAGS) $(SRCDIR)/main.c

paddle.o: $(COMMONSRCDIR)/paddle.c $(INCDIR)/paddle_data.h $(INCDIR)/paddle.h $(INCDIR)/ball.h
	$(CC) $(CFLAGS) $(COMMONSRCDIR)/paddle.c

text.o: $(COMMONSRCDIR)/text.c $(INCDIR)/text.h
	$(CC) $(CFLAGS) $(COMMONSRCDIR)/text.c

ball.o: $(COMMONSRCDIR)/ball.c $(INCDIR)/ball.h $(INCDIR)/paddle_data.h
	$(CC) $(CFLAGS) $(COMMONSRCDIR)/ball.c

obstacles.o: $(COMMONSRCDIR)/obstacles.c $(INCDIR)/obstacles.h $(INCDIR)/paddle_data.h $(INCDIR)/ball.h
	$(CC) $(CFLAGS) $(COMMONSRCDIR)/obstacles.c

clean:
	rm -f *.exe *.o