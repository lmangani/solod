#pragma once
#include "so/builtin/builtin.h"
#include "so/c/c.h"
#include "so/errors/errors.h"
#include "so/mem/mem.h"
#include "so/strings/strings.h"

// -- Embeds --

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
int so_duckdb_open_memory(so_duckdb_db* out);
void so_duckdb_close(so_duckdb_db* db);
void so_duckdb_interrupt(so_duckdb_db* db);
const char* so_duckdb_library_version(void);

int so_duckdb_query(so_duckdb_db* db, const char* query, so_duckdb_result* out);
int so_duckdb_query_void(so_duckdb_db* db, const char* query);

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
int32_t so_duckdb_result_error_type(so_duckdb_result* res);
int32_t so_duckdb_result_statement_type(so_duckdb_result* res);
int so_duckdb_result_row_count(so_duckdb_result* res);
int so_duckdb_result_rows_changed(so_duckdb_result* res);
int so_duckdb_result_column_count(so_duckdb_result* res);
const char* so_duckdb_result_column_name(so_duckdb_result* res, int col);
int32_t so_duckdb_column_type(so_duckdb_result* res, int col);
void* so_duckdb_column_data(so_duckdb_result* res, int col);
bool* so_duckdb_nullmask_data(so_duckdb_result* res, int col);

duckdb_logical_type so_duckdb_column_logical_type(so_duckdb_result* res, int col);
void so_duckdb_logical_type_destroy(duckdb_logical_type lt);

bool so_duckdb_value_is_null(so_duckdb_result* res, int col, int row);
bool so_duckdb_value_bool(so_duckdb_result* res, int col, int row);
int64_t so_duckdb_value_int64(so_duckdb_result* res, int col, int row);
double so_duckdb_value_double(so_duckdb_result* res, int col, int row);
char* so_duckdb_value_string(so_duckdb_result* res, int col, int row);

void so_duckdb_string_free(char* ptr);

// -- Types --

typedef struct duckdb_Conn duckdb_Conn;
typedef struct duckdb_ConnResult duckdb_ConnResult;
typedef struct duckdb_Stmt duckdb_Stmt;
typedef struct duckdb_StmtResult duckdb_StmtResult;
typedef struct duckdb_Result duckdb_Result;
typedef struct duckdb_ResultResult duckdb_ResultResult;
typedef struct duckdb_Rows duckdb_Rows;
typedef struct duckdb_ColTypeResult duckdb_ColTypeResult;

// Conn is a connection to a DuckDB database instance.
typedef struct duckdb_Conn {
    so_duckdb_db db;
    bool closed;
} duckdb_Conn;

// ConnResult is a helper struct for returning
// a Conn and an error from a function.
typedef struct duckdb_ConnResult {
    duckdb_Conn val;
    so_Error err;
} duckdb_ConnResult;

// Stmt is a prepared statement bound to a connection.
typedef struct duckdb_Stmt {
    so_duckdb_stmt stmt;
    bool closed;
} duckdb_Stmt;

// StmtResult is a helper struct for returning
// a Stmt and an error from a function.
typedef struct duckdb_StmtResult {
    duckdb_Stmt val;
    so_Error err;
} duckdb_StmtResult;

// Result owns a DuckDB query result.
typedef struct duckdb_Result {
    so_duckdb_result res;
    bool closed;
} duckdb_Result;

// ResultResult is a helper struct for returning
// a Result and an error from a function.
typedef struct duckdb_ResultResult {
    duckdb_Result val;
    so_Error err;
} duckdb_ResultResult;

// Rows is a row iterator over a Result.
typedef struct duckdb_Rows {
    duckdb_Result* result;
    so_int row;
} duckdb_Rows;

// ColType is the physical SQL column type (duckdb_type / DUCKDB_TYPE_*).
// Named ColType (not Type) so translated C does not emit duckdb_Type, which clashes with DuckDB's duckdb_type.
//
// See https://duckdb.org/docs/current/clients/c/types
typedef int32_t duckdb_ColType;

// ColTypeResult is a helper struct for returning a ColType and an error from a function.
typedef struct duckdb_ColTypeResult {
    duckdb_ColType val;
    so_Error err;
} duckdb_ColTypeResult;

// StatementType identifies the kind of SQL statement that produced a result.
typedef int32_t duckdb_StatementType;

// ErrorType classifies errors attached to a failed duckdb_query result.
typedef int32_t duckdb_ErrorType;

// -- Variables and constants --
extern so_Error duckdb_ErrOpen;
extern so_Error duckdb_ErrClosed;
extern so_Error duckdb_ErrQuery;
extern so_Error duckdb_ErrPrepare;
extern so_Error duckdb_ErrBind;
extern so_Error duckdb_ErrExec;
extern so_Error duckdb_ErrNullValue;
extern so_Error duckdb_ErrInvalidCol;
extern so_Error duckdb_ErrInvalidRow;
extern const duckdb_ColType duckdb_ColInvalid;
extern const duckdb_ColType duckdb_ColBoolean;
extern const duckdb_ColType duckdb_ColTinyInt;
extern const duckdb_ColType duckdb_ColSmallInt;
extern const duckdb_ColType duckdb_ColInteger;
extern const duckdb_ColType duckdb_ColBigInt;
extern const duckdb_ColType duckdb_ColUTinyInt;
extern const duckdb_ColType duckdb_ColUSmallInt;
extern const duckdb_ColType duckdb_ColUInteger;
extern const duckdb_ColType duckdb_ColUBigInt;
extern const duckdb_ColType duckdb_ColFloat;
extern const duckdb_ColType duckdb_ColDouble;
extern const duckdb_ColType duckdb_ColTimestamp;
extern const duckdb_ColType duckdb_ColDate;
extern const duckdb_ColType duckdb_ColTime;
extern const duckdb_ColType duckdb_ColInterval;
extern const duckdb_ColType duckdb_ColHugeInt;
extern const duckdb_ColType duckdb_ColUHugeInt;
extern const duckdb_ColType duckdb_ColVarchar;
extern const duckdb_ColType duckdb_ColBlob;
extern const duckdb_ColType duckdb_ColDecimal;
extern const duckdb_ColType duckdb_ColTimestampS;
extern const duckdb_ColType duckdb_ColTimestampMS;
extern const duckdb_ColType duckdb_ColTimestampNS;
extern const duckdb_ColType duckdb_ColEnum;
extern const duckdb_ColType duckdb_ColList;
extern const duckdb_ColType duckdb_ColStruct;
extern const duckdb_ColType duckdb_ColMap;
extern const duckdb_ColType duckdb_ColUUID;
extern const duckdb_ColType duckdb_ColUnion;
extern const duckdb_ColType duckdb_ColBit;
extern const duckdb_ColType duckdb_ColTimeTZ;
extern const duckdb_ColType duckdb_ColTimestampTZ;
extern const duckdb_ColType duckdb_ColArray;
extern const duckdb_ColType duckdb_ColAny;
extern const duckdb_ColType duckdb_ColBigNum;
extern const duckdb_ColType duckdb_ColSQLNull;
extern const duckdb_ColType duckdb_ColStringLiteral;
extern const duckdb_ColType duckdb_ColIntegerLiteral;
extern const duckdb_ColType duckdb_ColTimeNS;
extern const duckdb_ColType duckdb_ColGeometry;
extern const duckdb_StatementType duckdb_StatementInvalid;
extern const duckdb_StatementType duckdb_StatementSelect;
extern const duckdb_StatementType duckdb_StatementInsert;
extern const duckdb_StatementType duckdb_StatementUpdate;
extern const duckdb_StatementType duckdb_StatementExplain;
extern const duckdb_StatementType duckdb_StatementDelete;
extern const duckdb_StatementType duckdb_StatementPrepare;
extern const duckdb_StatementType duckdb_StatementCreate;
extern const duckdb_StatementType duckdb_StatementExecute;
extern const duckdb_StatementType duckdb_StatementAlter;
extern const duckdb_StatementType duckdb_StatementTransaction;
extern const duckdb_StatementType duckdb_StatementCopy;
extern const duckdb_StatementType duckdb_StatementAnalyze;
extern const duckdb_StatementType duckdb_StatementVariableSet;
extern const duckdb_StatementType duckdb_StatementCreateFunc;
extern const duckdb_StatementType duckdb_StatementDrop;
extern const duckdb_StatementType duckdb_StatementExport;
extern const duckdb_StatementType duckdb_StatementPragma;
extern const duckdb_StatementType duckdb_StatementVacuum;
extern const duckdb_StatementType duckdb_StatementCall;
extern const duckdb_StatementType duckdb_StatementSet;
extern const duckdb_StatementType duckdb_StatementLoad;
extern const duckdb_StatementType duckdb_StatementRelation;
extern const duckdb_StatementType duckdb_StatementExtension;
extern const duckdb_StatementType duckdb_StatementLogicalPlan;
extern const duckdb_StatementType duckdb_StatementAttach;
extern const duckdb_StatementType duckdb_StatementDetach;
extern const duckdb_StatementType duckdb_StatementMulti;
extern const duckdb_ErrorType duckdb_ErrorInvalid;
extern const duckdb_ErrorType duckdb_ErrorOutOfRange;
extern const duckdb_ErrorType duckdb_ErrorConversion;
extern const duckdb_ErrorType duckdb_ErrorUnknownType;
extern const duckdb_ErrorType duckdb_ErrorDecimal;
extern const duckdb_ErrorType duckdb_ErrorMismatchType;
extern const duckdb_ErrorType duckdb_ErrorDivideByZero;
extern const duckdb_ErrorType duckdb_ErrorObjectSize;
extern const duckdb_ErrorType duckdb_ErrorInvalidType;
extern const duckdb_ErrorType duckdb_ErrorSerialization;
extern const duckdb_ErrorType duckdb_ErrorTransaction;
extern const duckdb_ErrorType duckdb_ErrorNotImplemented;
extern const duckdb_ErrorType duckdb_ErrorExpression;
extern const duckdb_ErrorType duckdb_ErrorCatalog;
extern const duckdb_ErrorType duckdb_ErrorParser;
extern const duckdb_ErrorType duckdb_ErrorPlanner;
extern const duckdb_ErrorType duckdb_ErrorScheduler;
extern const duckdb_ErrorType duckdb_ErrorExecutor;
extern const duckdb_ErrorType duckdb_ErrorConstraint;
extern const duckdb_ErrorType duckdb_ErrorIndex;
extern const duckdb_ErrorType duckdb_ErrorStat;
extern const duckdb_ErrorType duckdb_ErrorConnection;
extern const duckdb_ErrorType duckdb_ErrorSyntax;
extern const duckdb_ErrorType duckdb_ErrorSettings;
extern const duckdb_ErrorType duckdb_ErrorBinder;
extern const duckdb_ErrorType duckdb_ErrorNetwork;
extern const duckdb_ErrorType duckdb_ErrorOptimizer;
extern const duckdb_ErrorType duckdb_ErrorNullPointer;
extern const duckdb_ErrorType duckdb_ErrorIO;
extern const duckdb_ErrorType duckdb_ErrorInterrupt;
extern const duckdb_ErrorType duckdb_ErrorFatal;
extern const duckdb_ErrorType duckdb_ErrorInternal;
extern const duckdb_ErrorType duckdb_ErrorInvalidInput;
extern const duckdb_ErrorType duckdb_ErrorOutOfMemory;
extern const duckdb_ErrorType duckdb_ErrorPermission;
extern const duckdb_ErrorType duckdb_ErrorParameterNotResolved;
extern const duckdb_ErrorType duckdb_ErrorParameterNotAllowed;
extern const duckdb_ErrorType duckdb_ErrorDependency;
extern const duckdb_ErrorType duckdb_ErrorHTTP;
extern const duckdb_ErrorType duckdb_ErrorMissingExtension;
extern const duckdb_ErrorType duckdb_ErrorAutoload;
extern const duckdb_ErrorType duckdb_ErrorSequence;
extern const duckdb_ErrorType duckdb_ErrorInvalidConfiguration;

// -- Functions and methods --

// LibraryVersion returns duckdb_library_version() (e.g. "v1.5.2").
// The string is owned by DuckDB; do not free it.
so_String duckdb_LibraryVersion(void);

// Open creates a new DuckDB database handle and connection from a path string.
// Use a file path for an on-disk database or ":memory:" for an in-memory database string path.
//
// For the same in-memory setup as the C examples using duckdb_open(NULL), use [OpenInMemory].
duckdb_ConnResult duckdb_Open(so_String path);

// OpenInMemory opens an ephemeral database using duckdb_open(NULL) and duckdb_connect,
// matching the Startup & Shutdown C examples.
duckdb_ConnResult duckdb_OpenInMemory(void);

// Close closes the connection and frees underlying DuckDB resources.
so_Error duckdb_Conn_Close(void* self);

// Interrupt requests interruption of the current query on this connection (duckdb_interrupt).
void duckdb_Conn_Interrupt(void* self);

// Query executes SQL and returns a materialized result set.
//
// If the returned error is non-nil, the [Result] still wraps the DuckDB result
// object (including error details); the caller must call [Result.Close].
duckdb_ResultResult duckdb_Conn_Query(void* self, so_String query);

// Exec executes SQL and returns the number of changed rows.
so_R_int_err duckdb_Conn_Exec(void* self, so_String query);

// ExecSQL runs SQL without retaining a result set (duckdb_query with a NULL result pointer).
// Use this for DDL or statements where row metadata is not needed.
//
// Errors return [ErrQuery] without an error message; use [Conn.Query] when diagnostics are required.
so_Error duckdb_Conn_ExecSQL(void* self, so_String sql);

// Prepare creates a prepared statement.
duckdb_StmtResult duckdb_Conn_Prepare(void* self, so_String query);

// Close releases prepared statement resources.
so_Error duckdb_Stmt_Close(void* self);

// ClearBindings clears all existing parameter bindings.
so_Error duckdb_Stmt_ClearBindings(void* self);

// BindNull binds NULL to parameter index (1-based).
so_Error duckdb_Stmt_BindNull(void* self, so_int index);

// BindBool binds a bool value to parameter index (1-based).
so_Error duckdb_Stmt_BindBool(void* self, so_int index, bool value);

// BindInt64 binds an int64 value to parameter index (1-based).
so_Error duckdb_Stmt_BindInt64(void* self, so_int index, int64_t value);

// BindFloat64 binds a float64 value to parameter index (1-based).
so_Error duckdb_Stmt_BindFloat64(void* self, so_int index, double value);

// BindString binds a string value to parameter index (1-based).
so_Error duckdb_Stmt_BindString(void* self, so_int index, so_String value);

// Query executes the prepared statement and returns a result set.
//
// If the returned error is non-nil, the [Result] still must be closed.
duckdb_ResultResult duckdb_Stmt_Query(void* self);

// Exec executes the prepared statement and returns changed row count.
so_R_int_err duckdb_Stmt_Exec(void* self);

// PrepareError returns the latest prepare error for this statement.
so_String duckdb_Stmt_PrepareError(void* self);

// Close releases the underlying result data.
so_Error duckdb_Result_Close(void* self);

// Error returns the result-level error message, if any.
//
// The pointer is owned by DuckDB; do not free it. It is valid until [Result.Close].
so_String duckdb_Result_Error(void* self);

// ErrorType returns the error classification when [Conn.Query] failed.
duckdb_ErrorType duckdb_Result_ErrorType(void* self);

// StatementType returns the statement type that produced this result.
duckdb_StatementType duckdb_Result_StatementType(void* self);

// RowCount returns the number of rows in this result.
so_int duckdb_Result_RowCount(void* self);

// RowsChanged returns number of rows changed by the statement.
so_int duckdb_Result_RowsChanged(void* self);

// ColumnCount returns number of columns in this result.
so_int duckdb_Result_ColumnCount(void* self);

// ColumnName returns a column name by index.
so_R_str_err duckdb_Result_ColumnName(void* self, so_int col);

// ColumnType returns the physical SQL type of column col (see [ColType] constants).
duckdb_ColTypeResult duckdb_Result_ColumnType(void* self, so_int col);

// ColumnData returns duckdb_column_data: a pointer to columnar data for col.
// Layout depends on [Result.ColumnType] / [ColType]; see DuckDB C documentation.
void* duckdb_Result_ColumnData(void* self, so_int col);

// NullmaskData returns duckdb_nullmask_data for col, or nil if unavailable.
void* duckdb_Result_NullmaskData(void* self, so_int col);

// ColumnLogicalType returns an opaque duckdb_logical_type handle for col (see DuckDB C docs).
// Release it with [DestroyLogicalType].
so_R_ptr_err duckdb_Result_ColumnLogicalType(void* self, so_int col);

// DestroyLogicalType destroys a handle returned by [Result.ColumnLogicalType].
void duckdb_DestroyLogicalType(void* lt);

// IsNull reports whether a value at (row, col) is null.
so_R_bool_err duckdb_Result_IsNull(void* self, so_int row, so_int col);

// Bool returns a bool value from (row, col).
so_R_bool_err duckdb_Result_Bool(void* self, so_int row, so_int col);

// Int64 returns an int64 value from (row, col).
so_R_i64_err duckdb_Result_Int64(void* self, so_int row, so_int col);

// Float64 returns a float64 value from (row, col).
so_R_f64_err duckdb_Result_Float64(void* self, so_int row, so_int col);

// StringCopy returns a heap-allocated copy of a string value from (row, col).
// The caller owns the returned string and must free it using mem.FreeString.
so_R_str_err duckdb_Result_StringCopy(void* self, mem_Allocator a, so_int row, so_int col);

// Rows creates a row iterator over this result.
duckdb_Rows duckdb_Result_Rows(void* self);

// Next advances to the next row and reports whether one exists.
bool duckdb_Rows_Next(void* self);

// Row returns the current row index.
so_int duckdb_Rows_Row(void* self);

// IsNull reports whether current row value at col is null.
so_R_bool_err duckdb_Rows_IsNull(void* self, so_int col);

// Bool reads a bool from current row at col.
so_R_bool_err duckdb_Rows_Bool(void* self, so_int col);

// Int64 reads an int64 from current row at col.
so_R_i64_err duckdb_Rows_Int64(void* self, so_int col);

// Float64 reads a float64 from current row at col.
so_R_f64_err duckdb_Rows_Float64(void* self, so_int col);

// StringCopy reads a string from current row at col and clones it.
// The caller owns the returned string and must free it using mem.FreeString.
so_R_str_err duckdb_Rows_StringCopy(void* self, mem_Allocator a, so_int col);
