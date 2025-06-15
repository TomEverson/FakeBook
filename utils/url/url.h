#ifndef URL_H
#define URL_H

#include <stddef.h>

#define MAX_URL_PARTS 10
#define MAX_PART_LENGTH 256
#define MAX_HEADERS 50
#define MAX_HEADER_NAME_LENGTH 64
#define MAX_HEADER_VALUE_LENGTH 256

typedef struct
{
    char name[MAX_HEADER_NAME_LENGTH];
    char value[MAX_HEADER_VALUE_LENGTH];
} Header;

// Structure to hold a parsed HTTP request
struct Request
{
    char method[8];                           // HTTP method (e.g., "GET", "POST")
    char protocol[16];                        // HTTP protocol (e.g., "HTTP/1.1")
    char url[MAX_URL_PARTS][MAX_PART_LENGTH]; // Array of URL parts (e.g., {"/", "users", "/123"})
    int client_fd;                            // Client file descriptor for sending response
    char body[1024];                          // Request body content
    Header headers[MAX_HEADERS];              // Array to store parsed HTTP headers
    int header_count;                         // Number of headers successfully parsed
};

typedef struct
{
    const char *name; // Field Name
    size_t offset;    // Field Offset in Struct
    char type;        // 's' for char[], 'i' for int, 'f' for float
    size_t size;      // Size of field
} FieldInfo;

struct Request parse_request(char buffer[], int *client_fd);
void parse_body(const char *request_body, void *output, FieldInfo *fields, int field_count);
char *url_decode(const char *src);

#endif
