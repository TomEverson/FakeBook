#include <sqlite3.h>
#include <stdio.h>

#include "db.h"

int setup_database()
{

    sqlite3 *db = open_database();

    if (db == NULL)
    {
        fprintf(stderr, "Database connection is not available.\n");
        return 1;
    }

    const char *user_table_sql =
        "CREATE TABLE IF NOT EXISTS users ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "first_name TEXT NOT NULL, "
        "last_name TEXT NOT NULL, "
        "email TEXT UNIQUE NOT NULL, "
        "password TEXT NOT NULL, "
        "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP"
        ");";

    sql_table_setup(user_table_sql, db);
    return 0;
}

void sql_table_setup(const char *sql, sqlite3 *db)
{
    char *err_msg = 0;

    int rc = sqlite3_exec(db, sql, 0, 0, &err_msg);

    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
    }
    printf("Table Created SuccessFully\n");
}
