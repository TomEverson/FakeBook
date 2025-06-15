#include "http_status.h"

const char *HTTP_OK_STATUS = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";

const char *HTTP_CREATED_STATUS = "HTTP/1.1 201 CREATED\r\nContent-Type: text/html\r\n\r\n";

const char *HTTP_BAD_REQUEST_STATUS = "HTTP/1.1 400 Bad Request\r\nContent-Type: text/html\r\nContent-Length: %zu\r\n\r\n%s";

const char *HTTP_INTERNAL_SERVER_ERROR_STATUS = "HTTP/1.1 500 Internal Server Error\r\nContent-Type: text/html\r\nContent-Length: %zu\r\n\r\n%s";
