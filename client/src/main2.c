#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_net.h>
#include <termios.h>
#include <fcntl.h>

#define BUFFER_SIZE 1024

// Funktion för att ställa in terminalen i non-blocking mode
void setNonBlockingInput() {
    struct termios t;
    tcgetattr(STDIN_FILENO, &t);
    t.c_lflag &= ~ICANON; // Avaktivera canonical mode
    t.c_lflag &= ~ECHO;   // Avaktivera echo
    tcsetattr(STDIN_FILENO, TCSANOW, &t);
    fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK); // Ställer non-blocking input
}

int main(int argc, char** argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <server_ip> <server_port>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char* server_ip = argv[1];
    int server_port = atoi(argv[2]);

    // Initiera SDL och SDL_net
    if (SDL_Init(0) < 0) {
        fprintf(stderr, "Failed to initialize SDL: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    if (SDLNet_Init() < 0) {
        fprintf(stderr, "Failed to initialize SDL_net: %s\n", SDLNet_GetError());
        SDL_Quit();
        return EXIT_FAILURE;
    }

    // Skapa UDP socket
    UDPsocket sock = SDLNet_UDP_Open(0);
    if (!sock) {
        fprintf(stderr, "Failed to create UDP socket: %s\n", SDLNet_GetError());
        SDLNet_Quit();
        SDL_Quit();
        return EXIT_FAILURE;
    }

    // Konfiguera server address
    IPaddress server_addr;
    if (SDLNet_ResolveHost(&server_addr, server_ip, server_port) < 0) {
        fprintf(stderr, "Failed to resolve server address: %s\n", SDLNet_GetError());
        SDLNet_UDP_Close(sock);
        SDLNet_Quit();
        SDL_Quit();
        return EXIT_FAILURE;
    }

    // Non-blocking mode på terminalen
    setNonBlockingInput();

    printf("Connected to Pong server. Use 'w' to move up and 's' to move down.\n");

    char buffer[BUFFER_SIZE];
    UDPpacket* packet = SDLNet_AllocPacket(BUFFER_SIZE);
    if (!packet) {
        fprintf(stderr, "Failed to allocate packet: %s\n", SDLNet_GetError());
        SDLNet_UDP_Close(sock);
        SDLNet_Quit();
        SDL_Quit();
        return EXIT_FAILURE;
    }

    while (1) {
        // Checka user input
        char input = getchar();
        if (input == 'w' || input == 's') {
            // Skicka input till servern
            packet->data[0] = input;
            packet->len = 1;
            packet->address = server_addr;

            if (SDLNet_UDP_Send(sock, -1, packet) == 0) {
                fprintf(stderr, "Failed to send input to server: %s\n", SDLNet_GetError());
                break;
            }
        }

        // Motta meddelande från servern
        if (SDLNet_UDP_Recv(sock, packet)) {
            packet->data[packet->len] = '\0'; // Null-terminate the received data
            // Visa uppdaterad information från servern (som exempelvis poäng eller position)
            printf("\r%s", (char*)packet->data);
            fflush(stdout);
        }
    }

    // Stäng socket och frigör resurser
    SDLNet_FreePacket(packet);
    SDLNet_UDP_Close(sock);
    SDLNet_Quit();
    SDL_Quit();

    return EXIT_SUCCESS;
}