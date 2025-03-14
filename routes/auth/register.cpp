#include <iostream>
#include <string>
#include <sys/socket.h>
#include "../../utils/utils.h"

struct RequestBody

{
    char first_name[32];
    char last_name[32];
    char email[48];
    char password[32];
    char confirm_password[32];
};

void register_router(struct Request *request)
{

    if (strcmp(request->method, "GET") == 0)
    {
        const char *file_template = read_file("/template/auth/register.html");

        send(request->client_fd, HTTP_OK_STATUS, strlen(HTTP_OK_STATUS), 0);
        send(request->client_fd, file_template, strlen(file_template), 0);
    }
    else if (strcmp(request->method, "POST") == 0)
    {

        std::cout << request->body << std::endl;

        send(request->client_fd, HTTP_OK_STATUS, strlen(HTTP_OK_STATUS), 0);
    }
}
