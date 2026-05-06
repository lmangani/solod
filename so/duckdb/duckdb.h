#pragma once

#include <stdbool.h>
#include <stdint.h>

#include <duckdb.h>

typedef struct so_duckdb_db {
    duckdb_database db;
    duckdb_connection conn;
    bool open;
} so_duckdb_db;

typedef struct so_duckdb_stmt {
    duckdb_prepared_statement stmt;
    bool open;
} so_duckdb_stmt;

typedef struct so_duckdb_result {
    duckdb_result result;
    bool open;
} so_duckdb_result;

int so_duckdb_open(const char* path, so_duckdb_db* out);
void so_duckdb_close(so_duckdb_db* db);

int so_duckdb_query(so_duckdb_db* db, const char* query, so_duckdb_result* out);

int so_duckdb_prepare(so_duckdb_db* db, const char* query, so_duckdb_stmt* out);
const char* so_duckdb_prepare_error(so_duckdb_stmt* stmt);
void so_duckdb_stmt_close(so_duckdb_stmt* stmt);
int so_duckdb_stmt_clear(so_duckdb_stmt* stmt);

int so_duckdb_bind_null(so_duckdb_stmt* stmt, int index);
int so_duckdb_bind_bool(so_duckdb_stmt* stmt, int index, bool value);
int so_duckdb_bind_int64(so_duckdb_stmt* stmt, int index, int64_t value);
int so_duckdb_bind_double(so_duckdb_stmt* stmt, int index, double value);
int so_duckdb_bind_varchar(so_duckdb_stmt* stmt, int index, const char* value);

int so_duckdb_stmt_exec(so_duckdb_stmt* stmt, so_duckdb_result* out);

void so_duckdb_result_close(so_duckdb_result* res);
const char* so_duckdb_result_error(so_duckdb_result* res);
int so_duckdb_result_row_count(so_duckdb_result* res);
int so_duckdb_result_rows_changed(so_duckdb_result* res);
int so_duckdb_result_column_count(so_duckdb_result* res);
const char* so_duckdb_result_column_name(so_duckdb_result* res, int col);

bool so_duckdb_value_is_null(so_duckdb_result* res, int col, int row);
bool so_duckdb_value_bool(so_duckdb_result* res, int col, int row);
int64_t so_duckdb_value_int64(so_duckdb_result* res, int col, int row);
double so_duckdb_value_double(so_duckdb_result* res, int col, int row);
char* so_duckdb_value_string(so_duckdb_result* res, int col, int row);

void so_duckdb_string_free(char* ptr);
