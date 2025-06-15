#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

void xor_encrypt(char *in, char *out)
{
    char key = 0x2A; // Simple key for demonstration
    int i;
    for (i = 0; in[i] != '\0'; i++)
    {
        out[i] = in[i] ^ key;
    }
    out[i] = '\0'; // Crucial: Null-terminate the output string
}

void xor_decrypt(char *in, char *out)
{
    char key = 0x2A; // Same key used for encryption
    int i;
    for (i = 0; in[i] != '\0'; i++)
    {
        out[i] = in[i] ^ key;
    }
    out[i] = '\0'; // Crucial: Null-terminate the output string
}

// Base64 encoding table
static const char base64_chars[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

/**
 * @brief Performs Base64 encoding.
 * @param in The input binary data.
 * @param in_len The length of the input binary data.
 * @param out The output buffer to store the Base64 encoded string.
 * Must be large enough (approx. 4/3 * in_len + 4 for padding/null).
 */
void base64_encode(const unsigned char *in, size_t in_len, char *out)
{
    int i = 0, j = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];

    while (in_len--)
    {
        char_array_3[i++] = *(in++);
        if (i == 3)
        {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;

            for (i = 0; i < 4; i++)
            {
                out[j++] = base64_chars[char_array_4[i]];
            }
            i = 0;
        }
    }

    if (i)
    {
        for (int k = i; k < 3; k++)
        {
            char_array_3[k] = '\0';
        }

        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        char_array_4[3] = char_array_3[2] & 0x3f;

        for (int k = 0; k < i + 1; k++)
        {
            out[j++] = base64_chars[char_array_4[k]];
        }

        while (i++ < 3)
        {
            out[j++] = '='; // Padding
        }
    }
    out[j] = '\0'; // Null-terminate the output string
}

// Base64 decoding lookup table
static const int base64_val[] = {
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, -1, -1, 63,
    52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -1, -1, -1,
    -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
    15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1,
    -1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
    41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1};

/**
 * @brief Performs Base64 decoding.
 * @param in The Base64 encoded input string.
 * @param out The output buffer to store the decoded binary data.
 * Must be large enough (approx. 3/4 * strlen(in)).
 * @return The length of the decoded data.
 */
size_t base64_decode(const char *in, unsigned char *out)
{
    int i = 0, j = 0, k = 0;
    unsigned char char_array_4[4];
    unsigned char char_array_3[3];
    size_t in_len = strlen(in);

    while (in_len-- && (in[k] != '=') && (base64_val[(int)in[k]] != -1))
    {
        char_array_4[i++] = in[k++];
        if (i == 4)
        {
            for (i = 0; i < 4; i++)
            {
                char_array_4[i] = base64_val[(int)char_array_4[i]];
            }

            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

            for (i = 0; i < 3; i++)
            {
                out[j++] = char_array_3[i];
            }
            i = 0;
        }
    }

    if (i)
    {
        for (int l = i; l < 4; l++)
        {
            char_array_4[l] = '\0';
        }

        for (int l = 0; l < 4; l++)
        {
            char_array_4[l] = base64_val[(int)char_array_4[l]];
        }

        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
        char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

        for (int l = 0; l < i - 1; l++)
        {
            out[j++] = char_array_3[l];
        }
    }
    out[j] = '\0'; // Null-terminate the output string
    return j;
}
