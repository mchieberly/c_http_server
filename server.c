/**
 * HTTP Server
 * 
 * Malachi Eberly
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 8080

int main()
{
    int server_fd, new_socket;
    struct sockaddr_in address;

	int opt = 1;
    char buffer[1024] = { 0 };
    char* hello = "Hello from server";

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port 8080
    if (setsockopt(server_fd, SOL_SOCKET,
                   SO_REUSEADDR | SO_REUSEPORT, &opt,
                   sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Forcefully attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

	if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Server is listening on port %d...\n", PORT);

    socklen_t addrlen = sizeof(address);

    // Accept an incoming connection
    new_socket = accept(server_fd, (struct sockaddr*)&address, &addrlen);
    if (new_socket < 0) {
        perror("accept");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Read the client's message
    if (read(new_socket, buffer, sizeof(buffer) - 1) > 0) {
        printf("Received from client: %s\n", buffer);
    }

    // Send a specific response to the client
    send(new_socket, hello, strlen(hello), 0);
    printf("Response sent to client: %s\n", hello);

    // closing the connected socket
    close(new_socket);
    // closing the listening socket
    close(server_fd);
    return 0;
}
