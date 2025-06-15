#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>

#include "../../utils/utils.h"
#include "../../db/db.h" // Assuming db.h contains prepare_statement, select_one_from_database

// Struct for incoming request body (email, password for login attempt)
typedef struct
{
    char email[48];
    char password[32];

} RequestBody;

typedef struct
{
    int id; // Assuming 'id' is an integer in your database
    char email[48];
    char password[32]; // Note: In a real app, this would be a hashed password
} UserDbInfo;

// FieldInfo for parsing the incoming request body (email, password)
FieldInfo login_request_body_fields[] = {
    {"email", offsetof(RequestBody, email), 's', sizeof(((RequestBody *)0)->email)},
    {"password", offsetof(RequestBody, password), 's', sizeof(((RequestBody *)0)->password)},
};

const int login_field_count = sizeof(login_request_body_fields) / sizeof(FieldInfo);

// FieldInfo for parsing the data fetched from the database (id, email, password)
FieldInfo user_db_info_fields[] = {
    {"id", offsetof(UserDbInfo, id), 'i', sizeof(((UserDbInfo *)0)->id)},
    {"email", offsetof(UserDbInfo, email), 's', sizeof(((UserDbInfo *)0)->email)},
    {"password", offsetof(UserDbInfo, password), 's', sizeof(((UserDbInfo *)0)->password)},
};

const int user_db_info_field_count = sizeof(user_db_info_fields) / sizeof(FieldInfo);

void login_router(struct Request *request)
{
    if (strcmp(request->method, "GET") == 0)
    {
        const char *file_template = read_file("/template/auth/login.html");
        if (!file_template)
        {
            const char *error_message = "<div class='text-red-500'>Login page not found!</div>";
            char response[1024];
            snprintf(response, sizeof(response), HTTP_INTERNAL_SERVER_ERROR_STATUS,
                     strlen(error_message), error_message);
            send(request->client_fd, response, strlen(response), 0);
            close(request->client_fd);
            return;
        }

        send(request->client_fd, HTTP_OK_STATUS, strlen(HTTP_OK_STATUS), 0);
        send(request->client_fd, file_template, strlen(file_template), 0);
        close(request->client_fd);
    }
    else if (strcmp(request->method, "POST") == 0)
    {
        RequestBody request_body = {0};

        parse_body(request->body, &request_body, login_request_body_fields, login_field_count);

        if (strlen(request_body.email) == 0 || strlen(request_body.password) == 0)
        {
            const char *error_message = "<div class='text-red-500'>Email and password are required.</div>";
            char response[1024];
            snprintf(response, sizeof(response), HTTP_BAD_REQUEST_STATUS,
                     strlen(error_message), error_message);
            send(request->client_fd, response, strlen(response), 0);
            close(request->client_fd);
            return;
        }

        if (is_invalid_email(request_body.email) == 1)
        {
            const char *error_message = "<div class='text-red-500'>Invalid email format.</div>";
            char response[1024];
            snprintf(response, sizeof(response), HTTP_BAD_REQUEST_STATUS,
                     strlen(error_message), error_message);
            send(request->client_fd, response, strlen(response), 0);
            close(request->client_fd);
            return;
        }

        sqlite3_stmt *stmt;
        const char *select_sql = "SELECT id, email, password FROM users WHERE email = ?;";

        if (prepare_statement(select_sql, &stmt) != SQLITE_OK)
        {
            const char *error_message = "<div class='text-red-500'>Internal server error (DB prepare).</div>";
            char response[1024];
            snprintf(response, sizeof(response), HTTP_INTERNAL_SERVER_ERROR_STATUS,
                     strlen(error_message), error_message);
            send(request->client_fd, response, strlen(response), 0);
            close(request->client_fd);
            return;
        }

        sqlite3_bind_text(stmt, 1, request_body.email, -1, SQLITE_STATIC);

        UserDbInfo user_from_db = {0};
        int rc = select_one_from_database(stmt, &user_from_db, user_db_info_fields, user_db_info_field_count);

        if (rc == SQLITE_NOTFOUND)
        {
            const char *error_message = "<div class='text-red-500'>Invalid email or password.</div>";
            char response[1024];
            snprintf(response, sizeof(response), HTTP_BAD_REQUEST_STATUS,
                     strlen(error_message), error_message);
            send(request->client_fd, response, strlen(response), 0);
            close(request->client_fd);
            return;
        }
        else if (rc != SQLITE_OK)
        {
            const char *error_message = "<div class='text-red-500'>Database error during login.</div>";
            char response[1024];
            snprintf(response, sizeof(response), HTTP_INTERNAL_SERVER_ERROR_STATUS,
                     strlen(error_message), error_message);
            send(request->client_fd, response, strlen(response), 0);
            close(request->client_fd);
            return;
        }

        // --- Password Verification (Insecure for demonstration, use hashing in production!) ---
        if (strcmp(request_body.password, user_from_db.password) == 0)
        {
            // Login successful!
            char encrypted_user_id[128] = {0};
            char base64_encoded_id[256] = {0}; // Buffer for Base64 encoded string

            // Convert int user_id to string before encryption
            char user_id_str[12]; // Max 11 digits for int + null terminator
            snprintf(user_id_str, sizeof(user_id_str), "%d", user_from_db.id);

            // Encrypt the user ID string
            xor_encrypt(user_id_str, encrypted_user_id);

            // Base64 encode the encrypted user ID for safe transmission in cookies
            base64_encode((const unsigned char *)encrypted_user_id, strlen(encrypted_user_id), base64_encoded_id);

            // Debug print of the Base64 encoded string
            printf("Base64 Encoded User ID: %s\n", base64_encoded_id);

            const char *redirect_path = "/"; // Redirect to your application's dashboard or home page

            char response_header[512]; // Increased buffer size for Set-Cookie header
            snprintf(response_header, sizeof(response_header),
                     "HTTP/1.1 200 OK\r\n"
                     "HX-Redirect: %s\r\n"                                                          // Re-added HX-Redirect for HTMX
                     "Set-Cookie: access_token=%s; Path=/; HttpOnly; Max-Age=%d; SameSite=Lax;\r\n" // Set the access token cookie
                     "Content-Length: 0\r\n"
                     "\r\n",
                     redirect_path,
                     base64_encoded_id, // Use the Base64 encoded string for the cookie
                     3600               // Max-Age in seconds (e.g., 1 hour)
            );

            send(request->client_fd, response_header, strlen(response_header), 0);
            close(request->client_fd);
            return;
        }
        else
        {
            // Password mismatch
            const char *error_message = "<div class='text-red-500'>Invalid email or password.</div>";
            char response[1024];
            snprintf(response, sizeof(response), HTTP_BAD_REQUEST_STATUS,
                     strlen(error_message), error_message);
            send(request->client_fd, response, strlen(response), 0);
            close(request->client_fd);
            return;
        }
    }
    else if (strcmp(request->method, "DELETE") == 0)
    {
        // Handle DELETE method for logout (deleting the cookie)
        const char *redirect_path = "/login";
        char response_header[512]; // Increased buffer size for Set-Cookie header

        printf("DELETE request received on /login: Attempting to log out and redirect to %s\n", redirect_path);

        // To delete a cookie, set its value to empty and Max-Age to 0 (or Expires to a past date).
        // Crucially, ensure Path, HttpOnly, and SameSite match the original cookie's attributes.
        snprintf(response_header, sizeof(response_header),
                 "HTTP/1.1 200 OK\r\n"                                                       // Using 200 OK is common with HX-Redirect
                 "HX-Redirect: %s\r\n"                                                       // HTMX-specific header to redirect client-side
                 "Set-Cookie: access_token=; Path=/; HttpOnly; Max-Age=0; SameSite=Lax;\r\n" // Delete the cookie
                 "Content-Length: 0\r\n"                                                     // No response body needed for a redirect
                 "\r\n",
                 redirect_path);

        send(request->client_fd, response_header, strlen(response_header), 0);
        close(request->client_fd);
        return; // Important: Terminate function after sending response
    }
    else
    {
        const char *error_message = "<div class='text-red-500'>Method Not Allowed.</div>";
        char response[1024];
        snprintf(response, sizeof(response), HTTP_BAD_REQUEST_STATUS,
                 strlen(error_message), error_message);
        send(request->client_fd, response, strlen(response), 0);
        close(request->client_fd);
        return;
    }
}
