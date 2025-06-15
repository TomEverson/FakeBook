#include <stdio.h>
#include <string.h>
#include <ctype.h>

int is_invalid_email(const char *email)
{
    if (!email || strlen(email) < 5)
    {             // Minimum valid email: a@b.c
        return 1; // Invalid
    }

    int at_count = 0;
    const char *at_pos = NULL;
    const char *dot_pos = NULL;

    for (size_t i = 0; email[i] != '\0'; i++)
    {
        char c = email[i];

        if (c == '@')
        {
            at_count++;
            at_pos = &email[i]; // Store the position of '@'
        }
        else if (c == '.')
        {
            dot_pos = &email[i]; // Store last dot position
        }
        else if (!isalnum(c) && c != '.' && c != '-' && c != '_')
        {
            return 1; // Invalid character found
        }
    }

    // Email must have exactly one '@'
    if (at_count != 1)
        return 1;

    // '@' must not be at the start or end
    if (at_pos == email || at_pos == email + strlen(email) - 1)
        return 1;

    // There must be a '.' after '@' in the domain part
    if (!dot_pos || dot_pos < at_pos + 2)
        return 1; // Ensures "a@b.c"

    // '.' must not be at the end
    if (*(dot_pos + 1) == '\0')
        return 1;

    return 0; // Valid email
}
