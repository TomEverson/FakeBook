#ifndef DB_H
#define DB_H

#include <sqlite3.h>

#include "../utils/utils.h"

sqlite3 *open_database();

int setup_database();

void close_database();

void sql_table_setup(const char *sql, sqlite3 *db);

int insert_to_database(sqlite3_stmt *stmt);

int select_one_from_database(sqlite3_stmt *stmt, void *result, FieldInfo *fields, int field_count);

int prepare_statement(const char *sql, sqlite3_stmt **stmt);
#endif
