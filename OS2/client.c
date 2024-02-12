#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <server_ip> <resource_path>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *server_ip = argv[1];
    char *resource_path = argv[2];

    int client_socket;
    struct sockaddr_in server_addr;

    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr(server_ip);
    memset(&(server_addr.sin_zero), '\0', 8);

    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1) {
        perror("connect");
        exit(EXIT_FAILURE);
    }

    char request[BUFFER_SIZE];
    snprintf(request, BUFFER_SIZE, "GET /%s HTTP/1.1\r\nHost: %s\r\n\r\n", resource_path, server_ip);

    if (send(client_socket, request, strlen(request), 0) == -1) {
        perror("send");
        exit(EXIT_FAILURE);
    }

    printf("Request sent for resource: %s\n", resource_path);

    char response[BUFFER_SIZE];
    int bytes_received;
    while ((bytes_received = recv(client_socket, response, BUFFER_SIZE, 0)) > 0) {
        printf("Received %d bytes:\n", bytes_received);
        fwrite(response, 1, bytes_received, stdout);
    }

    if (bytes_received == -1) {
        perror("recv");
        exit(EXIT_FAILURE);
    }

    close(client_socket);

    return 0;
}
