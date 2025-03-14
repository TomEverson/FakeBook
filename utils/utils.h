#ifndef UTILS_H
#define UTILS_H

#define MAX_URL_PARTS 10    // Maximum number of parts in URL
#define MAX_PART_LENGTH 256 // Maximum length of each part

#include "http_status/http_status.h" // Include status.h

struct Request

{
    char method[8];
    char protocol[16];
    char url[MAX_URL_PARTS][MAX_PART_LENGTH]; // Array of URL parts
    int client_fd;
    char body[1024];
};

struct Request parse_request(char buffer[], int *client_fd);

char *read_file(const char *input_file);

#endif
