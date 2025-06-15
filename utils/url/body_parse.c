#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>

#include "url.h"

// **Function to Parse Key-Value Pairs**
void parse_key_value_pairs(const char *request_body, char **keys, char **values, int *count)
{
    char buffer[512]; // Temporary buffer
    strncpy(buffer, request_body, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';

    int index = 0;
    char *pair = strtok(buffer, "&");
    while (pair && index < *count)
    {
        char *equal_sign = strchr(pair, '=');
        if (equal_sign)
        {
            *equal_sign = '\0'; // Split key and value
            keys[index] = pair;
            values[index] = equal_sign + 1;
            index++;
        }
        pair = strtok(NULL, "&");
    }
    *count = index; // Store actual number of parsed pairs
}

// **Generic Function to Parse Struct**
void parse_body(const char *request_body, void *output, FieldInfo *fields, int field_count)
{
    char *keys[20];
    char *values[20];
    int count = 20;

    parse_key_value_pairs(request_body, keys, values, &count);

    for (int i = 0; i < count; i++)
    {
        for (int j = 0; j < field_count; j++)
        {
            if (strcmp(keys[i], fields[j].name) == 0)
            {
                void *field_ptr = (char *)output + fields[j].offset;

                if (fields[j].type == 's') // String Field
                {
                    strncpy((char *)field_ptr, values[i], fields[j].size - 1);
                    ((char *)field_ptr)[fields[j].size - 1] = '\0'; // Null terminate
                }
                else if (fields[j].type == 'i') // Integer Field
                {
                    sscanf(values[i], "%d", (int *)field_ptr);
                }
                else if (fields[j].type == 'f') // Float Field
                {
                    sscanf(values[i], "%f", (float *)field_ptr);
                }
                break;
            }
        }
    }
}
