#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>  // For atoi
#include <stdbool.h> // For bool type

#include "../utils/utils.h"
// Ensure base64_decode, xor_decrypt, and get_cookie_value are available via utils.h

static bool handle_authentication_and_redirect(struct Request *request, int *authenticated_user_id_out)
{
    char *cookie_header_value = NULL;
    char *access_token_base64 = NULL;
    char decrypted_user_id_str[128] = {0};
    int user_id = -1; // Default to unauthenticated

    // Find the "cookie" header (names are converted to lowercase by parse_request)
    for (int i = 0; i < request->header_count; i++)
    {
        if (strcmp(request->headers[i].name, "cookie") == 0)
        {
            cookie_header_value = request->headers[i].value;
            break;
        }
    }

    if (cookie_header_value)
    {
        access_token_base64 = get_cookie_value(cookie_header_value, "access_token");

        if (access_token_base64)
        {
            unsigned char encrypted_user_id_decoded[128] = {0};
            size_t decoded_len = base64_decode(access_token_base64, encrypted_user_id_decoded);

            if (decoded_len > 0)
            {
                xor_decrypt((char *)encrypted_user_id_decoded, decrypted_user_id_str);
                int parsed_user_id = atoi(decrypted_user_id_str);

                if (parsed_user_id > 0) // Assuming user IDs are positive
                {
                    user_id = parsed_user_id;
                    printf("User authenticated with ID: %d\n", user_id);
                }
                else
                {
                    printf("Decrypted user ID is not valid: '%s'\n", decrypted_user_id_str);
                }
            }
            free(access_token_base64);
        }
    }

    if (user_id > 0)
    {

        *authenticated_user_id_out = user_id;
        return true;
    }
    else
    { // User is not authenticated, redirect
        const char *redirect_path = "/login";
        printf("User not authenticated, redirecting to: %s\n", redirect_path);

        // Directly send the redirect response here
        char response_header[256];
        snprintf(response_header, sizeof(response_header),
                 "HTTP/1.1 302 Found\r\n"
                 "Location: %s\r\n"
                 "Content-Length: 0\r\n"
                 "\r\n",
                 redirect_path);
        send(request->client_fd, response_header, strlen(response_header), 0);
        close(request->client_fd);

        return false;
    }
}

void index_router(struct Request *request)
{
    int authenticated_user_id = -1;

    if (!handle_authentication_and_redirect(request, &authenticated_user_id))
    {
        return; // Redirect handled, exit router
    }
    // If we reach here, the user IS authenticated, and authenticated_user_id contains their ID.

    if (strcmp(request->method, "GET") == 0)
    {
        const char *file_template = read_file("/template/index.html");

        send(request->client_fd, HTTP_OK_STATUS, strlen(HTTP_OK_STATUS), 0);
        send(request->client_fd, file_template, strlen(file_template), 0);
        close(request->client_fd);
        return;
    }
    else if (strcmp(request->method, "POST") == 0)
    {
        // Handle POST requests for authenticated users only
        const char *error_message = "<div class='text-red-500'>Method Not Allowed.</div>";
        char response[1024];
        snprintf(response, sizeof(response), HTTP_BAD_REQUEST_STATUS,
                 strlen(error_message), error_message);
        send(request->client_fd, response, strlen(response), 0);
        close(request->client_fd);
        return;
    }
}
