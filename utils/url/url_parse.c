#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "../utils.h"

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
    strncpy(req.protocol, protocol, sizeof(req.protocol) - 1);
    req.protocol[sizeof(req.protocol) - 1] = '\0';

    // Decode URL before processing
    char *decoded_url = url_decode(url_raw); // Decode URL
    if (decoded_url)
    {
        strncpy(url_raw, decoded_url, sizeof(url_raw) - 1);
        url_raw[sizeof(url_raw) - 1] = '\0';
        free(decoded_url); // Free memory after decoding
    }

    // Process URL into parts
    if (strcmp(url_raw, "/") == 0) // If root URL
    {
        strncpy(req.url[0], "/", MAX_PART_LENGTH - 1);
        req.url[0][MAX_PART_LENGTH - 1] = '\0';
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
        char *decoded_body = url_decode(body_start); // Decode body
        if (decoded_body)
        {
            strncpy(req.body, decoded_body, sizeof(req.body) - 1);
            req.body[sizeof(req.body) - 1] = '\0';
            free(decoded_body); // Free memory after decoding
        }
        else
        {
            strncpy(req.body, body_start, sizeof(req.body) - 1);
            req.body[sizeof(req.body) - 1] = '\0';
        }
    }
    else
    {
        req.body[0] = '\0'; // No body found
    }

    return req;
}

char *url_decode(const char *src)
{
    if (!src)
        return NULL;

    size_t len = strlen(src);
    char *decoded = (char *)malloc(len + 1); // Allocate enough space
    if (!decoded)
    {
        fprintf(stderr, "Memory allocation failed\n");
        return NULL;
    }

    size_t j = 0;
    for (size_t i = 0; i < len; i++)
    {
        if (src[i] == '%' && i + 2 < len && isxdigit((unsigned char)src[i + 1]) && isxdigit((unsigned char)src[i + 2]))
        {
            char hex_str[3] = {src[i + 1], src[i + 2], '\0'};
            decoded[j++] = (char)strtol(hex_str, NULL, 16);
            i += 2; // Skip the next two hex characters
        }
        else if (src[i] == '+')
        {
            decoded[j++] = ' '; // Convert '+' to space
        }
        else
        {
            decoded[j++] = src[i];
        }

        // Ensure we don't overflow the buffer
        if (j >= len)
        {
            fprintf(stderr, "Decoding error: buffer overflow detected\n");
            free(decoded);
            return NULL;
        }
    }

    decoded[j] = '\0'; // Null-terminate
    return decoded;
}
