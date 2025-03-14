#include <string.h>
#include <sys/socket.h>
#include "../utils/utils.h"

void index_router(struct Request *request)
{

    if (strcmp(request->method, "GET") == 0)
    {
        const char *file_template = read_file("/template/index.html");

        send(request->client_fd, HTTP_OK_STATUS, strlen(HTTP_OK_STATUS), 0);
        send(request->client_fd, file_template, strlen(file_template), 0);
    }
}
