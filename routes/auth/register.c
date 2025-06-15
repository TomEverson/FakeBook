#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>

#include "../../utils/utils.h"
#include "../../db/db.h"

typedef struct
{
    char first_name[32];
    char last_name[32];
    char email[48];
    char password[32];
    char confirm_password[32];
} RequestBody;

FieldInfo register_request_body_fields[] = {
    {"first_name", offsetof(RequestBody, first_name), 's', sizeof(((RequestBody *)0)->first_name)},
    {"last_name", offsetof(RequestBody, last_name), 's', sizeof(((RequestBody *)0)->last_name)},
    {"email", offsetof(RequestBody, email), 's', sizeof(((RequestBody *)0)->email)},
    {"password", offsetof(RequestBody, password), 's', sizeof(((RequestBody *)0)->password)},
    {"confirm_password", offsetof(RequestBody, confirm_password), 's', sizeof(((RequestBody *)0)->confirm_password)},
};

const int register_field_count = sizeof(register_request_body_fields) / sizeof(FieldInfo);

void register_router(struct Request *request)
{
    if (strcmp(request->method, "GET") == 0)
    {
        const char *file_template = read_file("/template/auth/register.html");

        send(request->client_fd, HTTP_OK_STATUS, strlen(HTTP_OK_STATUS), 0);
        send(request->client_fd, file_template, strlen(file_template), 0);
        close(request->client_fd);
    }
    else if (strcmp(request->method, "POST") == 0)
    {
        RequestBody request_body = {0};

        // Parse the body
        parse_body(request->body, &request_body, register_request_body_fields, register_field_count);

        // Check First Name
        if (strlen(request_body.first_name) < 3)
        {
            const char *error_message = "<div class='text-red-500'>First Name is Too Short!</div>";
            char response[1024];
            snprintf(response, sizeof(response), HTTP_BAD_REQUEST_STATUS,
                     strlen(error_message), error_message);
            send(request->client_fd, response, strlen(response), 0);
            close(request->client_fd);
            return;
        }

        // Check Last Name
        if (strlen(request_body.last_name) < 3)
        {
            const char *error_message = "<div class='text-red-500'>Last Name is Too Short!</div>";
            char response[1024];
            snprintf(response, sizeof(response), HTTP_BAD_REQUEST_STATUS,
                     strlen(error_message), error_message);
            send(request->client_fd, response, strlen(response), 0);
            close(request->client_fd);
            return;
        }

        // Check If the Email is Valid Or Not
        if (is_invalid_email(request_body.email) == 1)
        {
            const char *error_message = "<div class='text-red-500'>Invalid Email!</div>";
            char response[1024];
            snprintf(response, sizeof(response), HTTP_BAD_REQUEST_STATUS,
                     strlen(error_message), error_message);
            send(request->client_fd, response, strlen(response), 0);
            close(request->client_fd);
            return;
        }

        // Check The Password Length
        if (strlen(request_body.password) < 8)
        {
            const char *error_message = "<div class='text-red-500'>Password Is Too Short!</div>";
            char response[1024];
            snprintf(response, sizeof(response), HTTP_BAD_REQUEST_STATUS,
                     strlen(error_message), error_message);
            send(request->client_fd, response, strlen(response), 0);
            close(request->client_fd);
            return;
        }

        // Compare Password and Confirm Password
        if (strcmp(request_body.password, request_body.confirm_password) != 0)
        {
            const char *error_message = "<div class='text-red-500'>Password Are Not The Same!</div>";
            char response[1024];
            snprintf(response, sizeof(response), HTTP_BAD_REQUEST_STATUS,
                     strlen(error_message), error_message);
            send(request->client_fd, response, strlen(response), 0);
            close(request->client_fd);
            return;
        }

        // Check Database
        sqlite3_stmt *stmt;
        const char *select_sql = "SELECT * FROM users WHERE email = ?;";

        prepare_statement(select_sql, &stmt);
        sqlite3_bind_text16(stmt, 1, request_body.email, -1, SQLITE_STATIC);

        RequestBody query; // This struct seems to be used incorrectly here, as you only need to check for existence.
                           // Consider just checking the return code of select_one_from_database directly.

        int rc = select_one_from_database(stmt, &query, register_request_body_fields, register_field_count);

        if (rc == SQLITE_NOTFOUND)
        {
            // Insert Into Database
            sqlite3_stmt *insert_stmt;
            const char *insert_sql = "INSERT INTO users (first_name, last_name, email, password) VALUES (?, ?, ?, ?);";

            printf("Preparing Statement\n");
            prepare_statement(insert_sql, &insert_stmt);

            // Bind values to placeholders
            sqlite3_bind_text(insert_stmt, 1, request_body.first_name, -1, SQLITE_STATIC);
            sqlite3_bind_text(insert_stmt, 2, request_body.last_name, -1, SQLITE_STATIC);
            sqlite3_bind_text(insert_stmt, 3, request_body.email, -1, SQLITE_STATIC);
            sqlite3_bind_text(insert_stmt, 4, request_body.password, -1, SQLITE_STATIC);

            int insert_rc = insert_to_database(insert_stmt);

            if (insert_rc == 0)
            {
                // *** HTMX Redirect Implementation ***
                const char *redirect_path = "/login"; // The URL of your login page
                char response_header[256];
                snprintf(response_header, sizeof(response_header),
                         "HTTP/1.1 200 OK\r\n"
                         "HX-Redirect: %s\r\n"   // Send the HX-Redirect header
                         "Content-Length: 0\r\n" // No content in the body
                         "\r\n",
                         redirect_path);

                send(request->client_fd, response_header, strlen(response_header), 0);
                close(request->client_fd);
                return;
            }
            else
            {
                // Handle database insertion error
                const char *error_message = "<div class='text-red-500'>Database Error: Failed to register user!</div>";
                char response[1024];
                snprintf(response, sizeof(response), HTTP_INTERNAL_SERVER_ERROR_STATUS,
                         strlen(error_message), error_message);
                send(request->client_fd, response, strlen(response), 0);
                close(request->client_fd);
                return;
            }
        }
        else
        {
            // Email already exists
            const char *error_message = "<div class='text-red-500'>Email already registered!</div>";
            char response[1024];
            snprintf(response, sizeof(response), HTTP_BAD_REQUEST_STATUS,
                     strlen(error_message), error_message);
            send(request->client_fd, response, strlen(response), 0);
            close(request->client_fd);
            return;
        }
    }
}
