#include <stdio.h>
#include <stdlib.h>
#include <uv.h>
#include <string.h> 

#define PORT 8080
#define BUFFER_SIZE 1024

void on_connect(uv_connect_t *connection, int status);
void on_read(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf);
void on_write(uv_write_t *request, int status);
void on_close(uv_handle_t *handle);
void send_request(uv_stream_t *stream, const char *request);
void handle_response(const char *response, const char *server_ip);
void handle_file_list(const char *file_list, const char *server_ip, uv_loop_t *loop, uv_tcp_t *socket);

uv_loop_t *loop;
uv_tcp_t client_socket;
uv_connect_t connection;

typedef struct {
    uv_write_t request;
    uv_buf_t buffer;
} write_req_t;

void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
    *buf = uv_buf_init((char *)malloc(suggested_size), suggested_size);
}

void on_connect(uv_connect_t *connection, int status) {
    if (status < 0) {
        fprintf(stderr, "Connection error: %s\n", uv_strerror(status));
        return;
    }

    printf("Connected to server\n");

    uv_read_start(connection->handle, alloc_buffer, on_read);
}

void on_read(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf) {
    if (nread > 0) {
        printf("Received %ld bytes: \n%.*s\n", nread, (int)nread, buf->base);

        // Process received data here
        handle_response(buf->base, NULL);

        // Free the buffer memory
        free(buf->base);
    } else if (nread < 0) {
        if (nread != UV_EOF) {
            fprintf(stderr, "Read error: %s\n", uv_strerror(nread));
        }
        uv_close((uv_handle_t *)stream, on_close);
    }
}

void on_write(uv_write_t *request, int status) {
    if (status) {
        fprintf(stderr, "Write error: %s\n", uv_strerror(status));
    }
    free(request);
}

void on_close(uv_handle_t *handle) {
    printf("Connection closed\n");
}

void send_request(uv_stream_t *stream, const char *request) {
    write_req_t *write_req = (write_req_t *)malloc(sizeof(write_req_t));
    write_req->buffer = uv_buf_init((char *)request, strlen(request));
    uv_write(&write_req->request, stream, &write_req->buffer, 1, on_write);
}

void handle_response(const char *response, const char *server_ip) {
    // Check if the response is a file list
    if (strstr(response, ".list")) {
        handle_file_list(response, server_ip, loop, &client_socket);
    } else {
        printf("Received file content:\n%s\n", response);
        // Save the file content to a file
        // (Implement saving the file content to a file)
    }
}

void handle_file_list(const char *file_list, const char *server_ip, uv_loop_t *loop, uv_tcp_t *socket) {
    char *file_list_copy = strdup(file_list);  // Create a copy of the file_list
    char *file_name = strtok(file_list_copy, "\n");
    while (file_name != NULL) {
        // Remove any leading or trailing whitespace characters
        char *trimmed_file_name = strtok(file_name, " \t\r\n");
        if (trimmed_file_name != NULL) {
            // Construct and send the request for each file in the list
            char request[BUFFER_SIZE];
            snprintf(request, BUFFER_SIZE, "GET /%s HTTP/1.1\r\nHost: %s\r\n\r\n", trimmed_file_name, server_ip);
            send_request((uv_stream_t *)socket, request);
        }
        // Move to the next file name
        file_name = strtok(NULL, "\n");
    }
    free(file_list_copy);  // Free the allocated memory for the copied file_list
}



int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <server_ip> <file_path>\n", argv[0]);
        return 1;
    }

    const char *server_ip = argv[1];
    const char *file_path = argv[2];

    loop = uv_default_loop();

    struct sockaddr_in dest;
    uv_ip4_addr(server_ip, PORT, &dest);

    uv_tcp_init(loop, &client_socket);

    uv_tcp_connect(&connection, &client_socket, (const struct sockaddr *)&dest, on_connect);

    // Construct and send the request
    char request[BUFFER_SIZE];
    snprintf(request, BUFFER_SIZE, "GET /%s HTTP/1.1\r\nHost: %s\r\n\r\n", file_path, server_ip);
    send_request((uv_stream_t *)&client_socket, request);

    uv_run(loop, UV_RUN_DEFAULT);

    return 0;
}
