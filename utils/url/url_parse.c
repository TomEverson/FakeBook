#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h> // For isspace and tolower

#include "url.h" // Include the updated url.h which now contains Header struct and updated Request struct

char *url_decode(const char *src)
{
    if (!src)
        return NULL;

    size_t len = strlen(src);
    // Allocate max possible space, as decoded string can be shorter
    char *decoded = (char *)malloc(len + 1);
    if (!decoded)
    {
        fprintf(stderr, "Memory allocation failed for URL decode\n");
        return NULL;
    }

    size_t j = 0;
    for (size_t i = 0; i < len; i++)
    {
        if (src[i] == '%' && i + 2 < len &&
            isxdigit((unsigned char)src[i + 1]) &&
            isxdigit((unsigned char)src[i + 2]))
        {
            // Convert hex characters to integer
            char hex_str[3] = {src[i + 1], src[i + 2], '\0'};
            decoded[j++] = (char)strtol(hex_str, NULL, 16);
            i += 2; // Skip the two hex characters
        }
        else if (src[i] == '+')
        {
            decoded[j++] = ' '; // Convert '+' to space
        }
        else
        {
            decoded[j++] = src[i]; // Copy character as is
        }
    }

    decoded[j] = '\0'; // Null-terminate the decoded string

    return decoded;
}

static void to_lower_str(char *str)
{
    for (int i = 0; str[i]; i++)
    {
        str[i] = tolower((unsigned char)str[i]);
    }
}

struct Request parse_request(char buffer[], int *client_fd)
{
    struct Request req;
    char *body_start, *url_start, *token;
    int part_index = 0;

    // Initialize URL parts to empty strings
    for (int i = 0; i < MAX_URL_PARTS; i++)
        req.url[i][0] = '\0';

    // Initialize header count and header names/values to empty strings
    req.header_count = 0;
    for (int i = 0; i < MAX_HEADERS; i++)
    {
        req.headers[i].name[0] = '\0';
        req.headers[i].value[0] = '\0';
    }

    // --- DEBUG: Print raw request buffer ---
    // printf("\n--- Raw Request Buffer ---\n%s\n------------------------\n", buffer);

    // Parse method, URL, and protocol from the first line of the request
    char url_raw[1024], protocol[16];
    // Use sscanf to safely parse the first line
    if (sscanf(buffer, "%7s %1023s %15s", req.method, url_raw, protocol) != 3)
    {
        fprintf(stderr, "parse_request: Malformed request line, returning empty request.\n");
        req.method[0] = '\0'; // Mark request as invalid
        req.client_fd = *client_fd;
        req.body[0] = '\0';
        req.header_count = 0;
        return req;
    }

    req.client_fd = *client_fd;
    strncpy(req.protocol, protocol, sizeof(req.protocol) - 1);
    req.protocol[sizeof(req.protocol) - 1] = '\0';

    // Decode URL before processing its parts
    char *decoded_url = url_decode(url_raw);
    if (decoded_url)
    {
        strncpy(url_raw, decoded_url, sizeof(url_raw) - 1);
        url_raw[sizeof(url_raw) - 1] = '\0';
        free(decoded_url); // Free dynamically allocated memory
    }

    // Process URL into parts
    if (strcmp(url_raw, "/") == 0) // If root URL
    {
        strncpy(req.url[0], "/", MAX_PART_LENGTH - 1);
        req.url[0][MAX_PART_LENGTH - 1] = '\0';
    }
    else if (url_raw[0] == '/') // Ensure it's a valid URL starting with '/'
    {
        url_start = url_raw;
        // Use a temporary copy for strtok as strtok modifies the string
        char temp_url[1024];
        strncpy(temp_url, url_raw, sizeof(temp_url) - 1);
        temp_url[sizeof(temp_url) - 1] = '\0';

        token = strtok(temp_url, "/"); // Split by '/'
        while (token != NULL && part_index < MAX_URL_PARTS)
        {
            snprintf(req.url[part_index], MAX_PART_LENGTH, "/%s", token); // Store with leading '/'
            part_index++;
            token = strtok(NULL, "/");
        }
    }

    // --- Header Parsing Logic ---
    char *header_start_ptr = strstr(buffer, "\r\n"); // Find the end of the first line (request line)
    if (header_start_ptr)
    {
        header_start_ptr += 2; // Move past the first "\r\n" to the actual start of headers
    }
    else
    {
        fprintf(stderr, "parse_request: No CRLF found after request line.\n");
        req.body[0] = '\0';
        return req;
    }

    char *current_header_line_start = header_start_ptr;
    // Find the end of headers, marked by an empty line (\r\n\r\n)
    char *body_delimiter = strstr(buffer, "\r\n\r\n");

    if (!body_delimiter)
    {
        fprintf(stderr, "parse_request: No \\r\\n\\r\\n delimiter found. Assuming no body and invalid header termination.\n");
        // If no double CRLF, assume no body and attempt to parse headers until end of buffer
        // This is a fallback; real HTTP requests should have it.
        body_delimiter = buffer + strlen(buffer); // Set to end of buffer
    }

    // printf("\n--- Parsing Headers in parse_request ---\n");
    // printf("  Buffer start: %p\n", (void *)buffer);
    // printf("  Header start ptr: %p ('%.20s')\n", (void *)header_start_ptr, header_start_ptr);
    // printf("  Body delimiter ptr: %p ('%.10s')\n", (void *)body_delimiter, body_delimiter);
    // printf("  Length of buffer processed for headers (approx): %ld\n", (long)(body_delimiter - header_start_ptr));

    while (current_header_line_start < body_delimiter && req.header_count < MAX_HEADERS)
    {
        char *line_end = strstr(current_header_line_start, "\r\n");

        // printf("  -- Iteration %d --\n", req.header_count + 1);
        // printf("    Current line start: %p ('%.20s')\n", (void *)current_header_line_start, current_header_line_start);
        // printf("    Line end found at: %p ('%.5s')\n", (void *)line_end, line_end);
        // printf("    Is line_end NULL? %d\n", line_end == NULL);
        // printf("    Is line_end >= body_delimiter? %d\n", line_end >= body_delimiter); // This will be true for Cookie
        // printf("    (current_header_line_start < body_delimiter) = %d\n", current_header_line_start < body_delimiter);

        // MODIFIED: Changed >= to > to allow the last header line ending at body_delimiter to be processed
        if (!line_end || line_end > body_delimiter)
        {
            // This is the last header line before the body delimiter, or a malformed line
            // or we've hit the boundary.
            if (line_end == body_delimiter && line_end - current_header_line_start == 0)
            {
                // This specific case means an empty line, marking end of headers (body_delimiter found)
                printf("parse_request: Found end of headers (empty line, at body_delimiter).\n");
                break;
            }
            // If line_end is NULL or points past body_delimiter but not an empty line,
            // it means the last "header" line is incomplete or malformed.
            fprintf(stderr, "parse_request: Breaking: Incomplete or malformed header line or end of buffer reached.\n");
            break; // Stop parsing headers
        }

        size_t line_len = line_end - current_header_line_start;

        // Use a temporary buffer for the header line
        char header_line[line_len + 1]; // +1 for null terminator
        strncpy(header_line, current_header_line_start, line_len);
        header_line[line_len] = '\0';

        // printf("  Processing header line: '%s'\n", header_line); // DEBUG print

        char *colon_pos = strchr(header_line, ':');
        if (colon_pos)
        {
            *colon_pos = '\0'; // Null-terminate header name string
            char *name = header_line;
            char *value = colon_pos + 1;

            // Trim leading whitespace from the header value
            while (*value && isspace((unsigned char)*value))
            {
                value++;
            }

            // Convert header name to lowercase before copying to ensure consistent lookup
            to_lower_str(name);

            // Copy header name to the Request struct
            strncpy(req.headers[req.header_count].name, name, MAX_HEADER_NAME_LENGTH - 1);
            req.headers[req.header_count].name[MAX_HEADER_NAME_LENGTH - 1] = '\0';

            // Copy header value to the Request struct
            strncpy(req.headers[req.header_count].value, value, MAX_HEADER_VALUE_LENGTH - 1);
            req.headers[req.header_count].value[MAX_HEADER_VALUE_LENGTH - 1] = '\0';

            // printf("  Parsed Header: Name='%s', Value='%s'\n", req.headers[req.header_count].name, req.headers[req.header_count].value); // DEBUG print
            req.header_count++;
        }
        else
        {
            fprintf(stderr, "parse_request: Skipping malformed header line (no colon): '%s'\n", header_line);
        }
        current_header_line_start = line_end + 2; // Move to the start of the next header line (past \r\n)
    }
    printf("--- End Parsing Headers in parse_request (Count: %d) ---\n", req.header_count);
    // --- End Header Parsing Logic ---

    // Extract body (if present)
    body_start = body_delimiter;                                             // body_delimiter already points to the "\r\n\r\n"
    if (body_start != NULL && (body_start + 4) <= (buffer + strlen(buffer))) // Ensure there's content after delimiter
    {
        body_start += 4; // Move past the "\r\n\r\n" to the actual body content
        // Only decode if body is not empty to avoid issues with url_decode on empty string
        if (strlen(body_start) > 0)
        {
            char *decoded_body = url_decode(body_start); // Decode URL-encoded body (e.g., from form submissions)
            if (decoded_body)
            {
                strncpy(req.body, decoded_body, sizeof(req.body) - 1);
                req.body[sizeof(req.body) - 1] = '\0';
                free(decoded_body); // Free dynamically allocated memory
            }
            else
            {
                // If decoding failed or not URL-encoded, just copy the raw body
                strncpy(req.body, body_start, sizeof(req.body) - 1);
                req.body[sizeof(req.body) - 1] = '\0';
            }
        }
        else
        {
            req.body[0] = '\0'; // Body is just the delimiter, so effectively empty
        }
    }
    else
    {
        req.body[0] = '\0'; // No body found or delimiter at end of buffer
    }

    return req;
}
