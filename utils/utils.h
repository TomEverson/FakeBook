#ifndef UTILS_H
#define UTILS_H

#define MAX_URL_PARTS 10    // Maximum number of parts in URL
#define MAX_PART_LENGTH 256 // Maximum length of each part

#include <stddef.h>
#include "http_status/http_status.h" // Include status.h

struct Request

{
    char method[8];
    char protocol[16];
    char url[MAX_URL_PARTS][MAX_PART_LENGTH]; // Array of URL parts
    int client_fd;
    char body[1024];
};

// **Field Metadata**
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

char *read_file(const char *input_file);

#endif
