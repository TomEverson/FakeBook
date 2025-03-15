#include <stdio.h>
#include <sqlite3.h>

#include "../utils/utils.h"
#include <string.h>

sqlite3 *open_database()
{
    static sqlite3 *db = NULL; // Static variable to store the single instance

    if (db == NULL) // Open the database only if it's not already opened
    {
        int rc = sqlite3_open("fakebook.db", &db);

        if (rc)
        {
            fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
            db = NULL; // Ensure db remains NULL on failure
        }
        else
        {
            printf("Opened database successfully\n");
        }
    }

    return db; // Return the singleton instance
}

void close_database()
{
    static sqlite3 *db = NULL; // Access the singleton instance

    if (db != NULL)
    {
        sqlite3_close(db);
        db = NULL; // Reset the instance
        printf("Database closed successfully\n");
    }
}

int insert_to_database(sqlite3_stmt *stmt)
{
    sqlite3 *db = open_database(); // Ensure your open_database() is implemented

    if (db == NULL)
    {
        fprintf(stderr, "Database connection is not available.\n");
        return 1;
    }

    // Execute the prepared statement
    int rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE)
    {
        fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return 1;
    }

    printf("Record inserted successfully.\n");

    // Clean up
    sqlite3_finalize(stmt);

    return 0; // Success
}

int select_one_from_database(sqlite3_stmt *stmt, void *result, FieldInfo *fields, int field_count)
{
    sqlite3 *db = open_database(); // Ensure open_database() is implemented

    // Execute the query and fetch the first row
    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        for (int i = 0; i < field_count; i++)
        {
            void *field_ptr = (char *)result + fields[i].offset; // Compute field address

            if (fields[i].type == 'i')
            {
                *((int *)field_ptr) = sqlite3_column_int(stmt, i);
            }
            else if (fields[i].type == 's')
            {
                const char *text = (const char *)sqlite3_column_text(stmt, i);
                strncpy((char *)field_ptr, text ? text : "NULL", fields[i].size - 1);
                ((char *)field_ptr)[fields[i].size - 1] = '\0'; // Null-terminate
            }
        }
    }
    else
    {
        fprintf(stderr, "No record found.\n");
        sqlite3_finalize(stmt);
        return SQLITE_NOTFOUND; // No data found
    }

    // Clean up
    sqlite3_finalize(stmt);

    return SQLITE_OK; // Success
}

int prepare_statement(const char *sql, sqlite3_stmt **stmt) // Use **stmt
{
    sqlite3 *db = open_database(); // Ensure open_database() is implemented
    if (db == NULL)
    {
        fprintf(stderr, "Database connection is not available.\n");
        return 1; // Indicate failure
    }

    int rc = sqlite3_prepare_v2(db, sql, -1, stmt, 0); // Pass **stmt**
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return 1;
    }

    return 0; // Success
}
