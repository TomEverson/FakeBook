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
        close(request->client_fd);
    }
    else if (strcmp(request->method, "POST") == 0)
    {
        RequestBody request_body = {0};

        parse_body(request->body, &request_body, request_body_fields, field_count);

        // Check The Password Length
        if (strlen(request_body.password) < 8)
        {
            send(request->client_fd, HTTP_BAD_REQUEST_STATUS, strlen(HTTP_BAD_REQUEST_STATUS), 0);
            close(request->client_fd);
            return;
        }

        // Compare Password and Confirm Password
        if (strcmp(request_body.password, request_body.confirm_password) != 0)
        {
            send(request->client_fd, HTTP_BAD_REQUEST_STATUS, strlen(HTTP_BAD_REQUEST_STATUS), 0);
            close(request->client_fd);
            return;
        }

        // Check Database
        sqlite3_stmt *stmt;
        const char *select_sql = "SELECT * FROM users WHERE email = ?;";

        prepare_statement(select_sql, &stmt);
        sqlite3_bind_text16(stmt, 1, request_body.email, -1, SQLITE_STATIC);

        RequestBody query;

        int rc = select_one_from_database(stmt, &query, request_body_fields, field_count);

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
                send(request->client_fd, HTTP_CREATED_STATUS, strlen(HTTP_CREATED_STATUS), 0);
                close(request->client_fd);
                return;
            }
        }
        else
        {
            send(request->client_fd, HTTP_BAD_REQUEST_STATUS, strlen(HTTP_BAD_REQUEST_STATUS), 0);
            close(request->client_fd);
            return;
        }
    }
}
