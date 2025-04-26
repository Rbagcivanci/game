#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
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

    // Skapa UDP socket
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Failed to create socket");
        return EXIT_FAILURE;
    }

    // Konfiguera server address
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        perror("Invalid server IP address");
        close(sockfd);
        return EXIT_FAILURE;
    }

    // Non-blocking mode på terminalen
    setNonBlockingInput();

    printf("Connected to Pong server. Use 'w' to move up and 's' to move down.\n");

    char buffer[BUFFER_SIZE];
    socklen_t server_addr_len = sizeof(server_addr);

    while (1) {
        //Checka user input 
        char input = getchar();
        if (input == 'w' || input == 's') {
            // Skicka input till servern
            if (sendto(sockfd, &input, sizeof(input), 0, (struct sockaddr*)&server_addr, server_addr_len) < 0) {
                perror("Failed to send input to server");
                break;
            }
        }

        // Motta meddelande från servern
        int bytes_received = recvfrom(sockfd, buffer, BUFFER_SIZE - 1, 0, (struct sockaddr*)&server_addr, &server_addr_len);
        if (bytes_received > 0) {
            buffer[bytes_received] = '\0';
            // Visa uppdaterad information från servern (som exempelvis poäng eller position)
            printf("\r%s", buffer);
            fflush(stdout);
        }
    }

    // Stäng socket
    close(sockfd);
    return EXIT_SUCCESS;
}  