#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <unistd.h>
#include <cstring>

#include "utils/utils.h"
#include "routes/routes.h"

#define PORT 4221
#define BUFFER_SIZE 1024

int main()
{
    int server_fd;
    socklen_t client_addr_len;
    struct sockaddr_in client_addr;

    // Create socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1)
    {
        std::cerr << "Socket creation failed!\n";
        return 1;
    }

    // Allow reusing address
    int reuse = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0)
    {
        std::cerr << "SO_REUSEADDR failed!\n";
        return 1;
    }

    // Server address structure
    struct sockaddr_in serv_addr = {
        .sin_family = AF_INET,
        .sin_port = htons(PORT),
        .sin_addr = {htonl(INADDR_ANY)},
    };

    // Bind socket
    if (bind(server_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        std::cerr << "Bind failed!\n";
        return 1;
    }

    // Start listening
    int connection_backlog = 10;
    if (listen(server_fd, connection_backlog) != 0)
    {
        std::cerr << "Listen failed!\n";
        return 1;
    }

    std::cout << "Server is listening on port " << PORT << "...\n";

    // **Main server loop**
    while (true)
    {
        client_addr_len = sizeof(client_addr);
        int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client_fd == -1)
        {
            std::cerr << "Failed to accept client!\n";
            continue; // Keep the server running
        }

        std::cout << "Client connected!\n";

        char buffer[BUFFER_SIZE] = {0};
        ssize_t byte_received = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);
        if (byte_received <= 0)
        {
            std::cerr << "Error reading request or client disconnected.\n";
            close(client_fd);
            continue;
        }

        Request request = parse_request(buffer, &client_fd);

        if (strcmp(request.url[0], "/") == 0)
        {
            index_router(&request);
        }
        else if (strcmp(request.url[0], "/login") == 0)
        {
            login_router(&request);
        }
        else if (strcmp(request.url[0], "/register") == 0)
        {
            register_router(&request);
        }
        else
        {
            const char *not_found_response =
                "HTTP/1.1 404 Not Found\r\n"
                "Content-Type: text/plain\r\n"
                "Content-Length: 13\r\n"
                "\r\n"
                "404 Not Found";
            send(client_fd, not_found_response, strlen(not_found_response), 0);
        }

        close(client_fd);
    }

    close(server_fd);
    return 0;
}
