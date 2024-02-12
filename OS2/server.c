#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <libgen.h>
#include <openssl/bio.h>
#include <openssl/evp.h>

#define PORT 8080
#define BACKLOG 10
#define BUFFER_SIZE 1024

void handle_client(int client_socket, char *root_dir);
void handle_get_request(int client_socket, char *remote_path, char *root_dir);
void handle_post_request(int client_socket, char *remote_path, char *root_dir);
void send_response(int client_socket, char *response);

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <root_directory>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *root_dir = argv[1];

    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t sin_size = sizeof(struct sockaddr_in);

    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    memset(&(server_addr.sin_zero), '\0', 8);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, BACKLOG) == -1) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);

    while(1) {
        if ((client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &sin_size)) == -1) {
            perror("accept");
            continue;
        }

        printf("Received connection from %s\n", inet_ntoa(client_addr.sin_addr));

        if (!fork()) {
            close(server_socket);
            handle_client(client_socket, root_dir);
            close(client_socket);
            exit(EXIT_SUCCESS);
        }

        close(client_socket);
        while(waitpid(-1, NULL, WNOHANG) > 0);
    }

    return 0;
}

void handle_client(int client_socket, char *root_dir) {
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);

    if (recv(client_socket, buffer, BUFFER_SIZE, 0) <= 0) {
        perror("recv");
        return;
    }

    char *token = strtok(buffer, " ");
    if (strcmp(token, "GET") == 0) {
        char *remote_path = strtok(NULL, " ");
        printf("Received GET request: GET %s\n", remote_path); // Debugging statement
        handle_get_request(client_socket, remote_path, root_dir);
    } else if (strcmp(token, "POST") == 0) {
        char *remote_path = strtok(NULL, " ");
        printf("Received POST request: POST %s\n", remote_path); // Debugging statement
        handle_post_request(client_socket, remote_path, root_dir);
    } else {
        send_response(client_socket, "500 INTERNAL ERROR\r\n\r\n");
    }
}

void handle_get_request(int client_socket, char *remote_path, char *root_dir) {
    // Construct the full file path
    char full_path[PATH_MAX];
    snprintf(full_path, PATH_MAX, "%s%s", root_dir, remote_path);
    printf("Full path: %s\n", full_path); // Debugging statement
    printf("Attempting to open file: %s\n", full_path);

    int file_fd = open(full_path, O_RDONLY);
    if (file_fd == -1) {
        printf("Failed to open file: %s, Error: %s\n", full_path, strerror(errno));
        if (errno == ENOENT) {
            printf("File not found: %s\n", full_path); // Debugging statement
            send_response(client_socket, "404 FILE NOT FOUND\r\n\r\n");
        } else {
            perror("open");
            send_response(client_socket, "500 INTERNAL ERROR\r\n\r\n");
        }
        return;
    }

    struct stat file_stat;
    if (fstat(file_fd, &file_stat) == -1) {
        perror("fstat");
        send_response(client_socket, "500 INTERNAL ERROR\r\n\r\n");
        close(file_fd);
        return;
    }

    char response_header[256];
    snprintf(response_header, sizeof(response_header), "200 OK\r\nContent-Length: %ld\r\n\r\n", file_stat.st_size);
    send_response(client_socket, response_header);

    off_t offset = 0;
    int sent_bytes;
    while ((sent_bytes = sendfile(client_socket, file_fd, &offset, BUFFER_SIZE)) > 0 && offset < file_stat.st_size);

    close(file_fd);
}


void handle_post_request(int client_socket, char *remote_path, char *root_dir) {
    char full_path[PATH_MAX];
    snprintf(full_path, PATH_MAX, "%s%s", root_dir, remote_path);
    printf("Full path: %s\n", full_path); // Debugging statement

    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);
    if (recv(client_socket, buffer, BUFFER_SIZE, 0) <= 0) {
        perror("recv");
        return;
    }

    char *base64_content = strtok(buffer, "\r\n");
    base64_content = strtok(NULL, "\r\n");

    BIO *b64 = BIO_new(BIO_f_base64());
    BIO *mem = BIO_new_mem_buf(base64_content, -1);
    mem = BIO_push(b64, mem);

    FILE *file = fopen(full_path, "wb");
    if (!file) {
        perror("fopen");
        send_response(client_socket, "500 INTERNAL ERROR\r\n\r\n");
        return;
    }

    int length;
    char inbuf[BUFFER_SIZE];
    while ((length = BIO_read(mem, inbuf, BUFFER_SIZE)) > 0) {
        fwrite(inbuf, 1, length, file);
    }

    fclose(file);
    BIO_free_all(mem);

    send_response(client_socket, "200 OK\r\n\r\n");
}

void send_response(int client_socket, char *response) {
    send(client_socket, response, strlen(response), 0);
}
