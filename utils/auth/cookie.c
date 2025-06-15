#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *get_cookie_value(const char *cookie_header, const char *cookie_name)
{
    if (!cookie_header || !cookie_name)
    {
        return NULL;
    }

    char *cookie_start = strstr(cookie_header, cookie_name);
    if (!cookie_start)
    {
        return NULL; // Cookie not found
    }

    // Move past the cookie name and the '=' sign
    cookie_start += strlen(cookie_name) + 1;

    char *cookie_end = strchr(cookie_start, ';');
    size_t len;

    if (cookie_end)
    {
        len = cookie_end - cookie_start;
    }
    else
    {
        len = strlen(cookie_start);
    }

    char *value = (char *)malloc(len + 1);
    if (value)
    {
        strncpy(value, cookie_start, len);
        value[len] = '\0';
    }
    return value;
}
