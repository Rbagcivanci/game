main: main.o
	gcc -o pong main.o -lmingw32 -lSDL2main -lSDL2 -lSDL2_ttf

main.o: .\source\main.c
	gcc -c -g -IC:\msys64\mingw64\include\SDL2 .\source\main.c

clean:
	rm *.exe
	rm *.o