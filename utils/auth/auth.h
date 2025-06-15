#ifndef EMAIL_H
#define EMAIL_H

#include <stddef.h>

int is_invalid_email(const char *email);

void xor_encrypt(char *in, char *out);

void xor_decrypt(char *in, char *out);

void base64_encode(const unsigned char *in, size_t in_len, char *out);

size_t base64_decode(const char *in, unsigned char *out);

char *get_cookie_value(const char *cookie_header, const char *cookie_name);

#endif
