#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define DEFAULT_PORT 2345   // Default port number if -p is not specified
#define BUFFER_SIZE 1024    // Size of the buffer for incoming messages

int verbose = 0; // If -v flag is provided, print incoming messages

// Function to handle a client connection
void *handleConnection(void *sock_fd_ptr) {
    int sock_fd = *((int *)sock_fd_ptr); // Get socket fd from the pointer
    free(sock_fd_ptr);

    char buffer[BUFFER_SIZE];
    char send_buf[BUFFER_SIZE];
    int bytes_read;

    // Loop to receive messages until client disconnects
    while ((bytes_read = recv(sock_fd, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[bytes_read] = '\0';  // Null-terminate the received data

        // Split input by newline and send back each line
        char *line = strtok(buffer, "\n");
        while (line != NULL) {
            snprintf(send_buf, sizeof(send_buf), "%s\n", line);
            send(sock_fd, send_buf, strlen(send_buf), 0);

            if (verbose) {
                printf("Received: %s\n", line); // Print if -v flag is used
            }

            line = strtok(NULL, "\n");
        }
    }

    // Close connection when client disconnects
    close(sock_fd);
    return NULL;
}

int main(int argc, char *argv[]) {
    int port = DEFAULT_PORT;

    // Parse command-line arguments
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-p") && i + 1 < argc) {
            port = atoi(argv[++i]); // Get port number
        } else if (!strcmp(argv[i], "-v")) {
            verbose = 1; // Enable verbose printing
        }
    }

    // Create a TCP socket
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        perror("socket");
        exit(1);
    }

    // Allow address reuse (prevents "address already in use" errors)
    int opt = 1;
    setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // Set up server address structure
    struct sockaddr_in server_address = {
        .sin_family = AF_INET,
        .sin_port = htons(port),
        .sin_addr.s_addr = INADDR_ANY
    };

    // Bind the socket to the address and port
    if (bind(socket_fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("bind");
        exit(1);
    }

    // Start listening for connections (max 10 in queue)
    if (listen(socket_fd, 10) < 0) {
        perror("listen");
        exit(1);
    }

    printf("[+] Listening on port %d...\n", port);

    // Main loop: accept and handle client connections
    while (1) {
        struct sockaddr_in client_address;
        socklen_t client_len = sizeof(client_address);

        // Allocate memory for client socket descriptor
        int *client_fd_buf = malloc(sizeof(int));
        if (!client_fd_buf) {
            perror("malloc");
            continue;
        }

        // Accept a new client connection
        *client_fd_buf = accept(socket_fd, (struct sockaddr *)&client_address, &client_len);
        if (*client_fd_buf < 0) {
            perror("accept");
            free(client_fd_buf);
            continue;
        }

        printf("[+] Accepted connection on %d\n", *client_fd_buf);

        // Create a new thread to handle the connection
        pthread_t thread;
        pthread_create(&thread, NULL, handleConnection, client_fd_buf);
        pthread_detach(thread);
    }

    return 0;
}
