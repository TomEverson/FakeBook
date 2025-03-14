#include <string.h>
#include <sys/socket.h>
#include <stddef.h>
#include <stdio.h>
#include "../../utils/utils.h"

typedef struct
{
    char first_name[32];
    char last_name[32];
    char email[48];
    char password[32];
    char confirm_password[32];
} RequestBody;

FieldInfo request_body_fields[] = {
    {"first_name", offsetof(RequestBody, first_name), 's', sizeof(((RequestBody *)0)->first_name)},
    {"last_name", offsetof(RequestBody, last_name), 's', sizeof(((RequestBody *)0)->last_name)},
    {"email", offsetof(RequestBody, email), 's', sizeof(((RequestBody *)0)->email)},
    {"password", offsetof(RequestBody, password), 's', sizeof(((RequestBody *)0)->password)},
    {"confirm_password", offsetof(RequestBody, confirm_password), 's', sizeof(((RequestBody *)0)->confirm_password)},
};

const int field_count = sizeof(request_body_fields) / sizeof(FieldInfo);

void register_router(struct Request *request)
{

    if (strcmp(request->method, "GET") == 0)
    {
        const char *file_template = read_file("/template/auth/register.html");

        send(request->client_fd, HTTP_OK_STATUS, strlen(HTTP_OK_STATUS), 0);
        send(request->client_fd, file_template, strlen(file_template), 0);
    }
    else if (strcmp(request->method, "POST") == 0)
    {
        RequestBody parsed_data = {0};

        parse_body(request->body, &parsed_data, request_body_fields, field_count);

        printf("First Name: %s\n", parsed_data.first_name);
        printf("Last Name: %s\n", parsed_data.last_name);
        printf("Email: %s\n", parsed_data.email);
        printf("Password: %s\n", parsed_data.password);
        printf("Confirm Password: %s\n", parsed_data.confirm_password);

        send(request->client_fd, HTTP_CREATED_STATUS, strlen(HTTP_CREATED_STATUS), 0);
    }
}
