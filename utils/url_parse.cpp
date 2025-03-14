#include <iostream>
#include <string>
#include "utils.h"

using namespace std;

struct Request parse_request(char buffer[], int *client_fd)
{
    struct Request req;
    char *body_start, *url_start, *token;
    int part_index = 0;

    // Initialize URL parts to empty strings
    for (int i = 0; i < MAX_URL_PARTS; i++)
        req.url[i][0] = '\0';

    // Parse method, URL, and protocol
    char url_raw[1024], protocol[16];
    sscanf(buffer, "%7s %1023s %15s", req.method, url_raw, protocol);
    req.client_fd = *client_fd;
    strlcpy(req.protocol, protocol, sizeof(req.protocol));

    // Process URL into parts
    if (strcmp(url_raw, "/") == 0) // If root URL
    {
        strlcpy(req.url[0], "/", MAX_PART_LENGTH);
    }
    else if (url_raw[0] == '/') // Ensure it's a valid URL
    {
        url_start = url_raw;            // Keep leading '/'
        token = strtok(url_start, "/"); // Split by '/'

        while (token != NULL && part_index < MAX_URL_PARTS)
        {
            snprintf(req.url[part_index], MAX_PART_LENGTH, "/%s", token); // Add '/' before segment
            part_index++;
            token = strtok(NULL, "/");
        }
    }

    // Extract body (if present)
    body_start = strstr(buffer, "\r\n\r\n");
    if (body_start != NULL)
    {
        body_start += 4;
        strlcpy(req.body, body_start, sizeof(req.body));
    }
    else
    {
        req.body[0] = '\0'; // No body found
    }

    return req;
}
