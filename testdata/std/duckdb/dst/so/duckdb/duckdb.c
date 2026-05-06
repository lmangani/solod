#include "duckdb.h"

// -- Embeds --

//go:build ignore
#include "duckdb.h"

static int so_duckdb_open_impl(const char* path, so_duckdb_db* out) {
    if (!out) {
        return 1;
    }
    out->db = NULL;
    out->conn = NULL;
    out->open = false;

    if (duckdb_open(path, &out->db) != DuckDBSuccess) {
        return 1;
    }
    if (duckdb_connect(out->db, &out->conn) != DuckDBSuccess) {
        duckdb_close(&out->db);
        return 1;
    }
    out->open = true;
    return 0;
}

int so_duckdb_open(const char* path, so_duckdb_db* out) {
    return so_duckdb_open_impl(path, out);
}

int so_duckdb_open_memory(so_duckdb_db* out) {
    return so_duckdb_open_impl(NULL, out);
}

void so_duckdb_close(so_duckdb_db* db) {
    if (!db || !db->open) {
        return;
    }
    duckdb_disconnect(&db->conn);
    duckdb_close(&db->db);
    db->open = false;
}

void so_duckdb_interrupt(so_duckdb_db* db) {
    if (!db || !db->open) {
        return;
    }
    duckdb_interrupt(db->conn);
}

const char* so_duckdb_library_version(void) {
    return duckdb_library_version();
}

int so_duckdb_query(so_duckdb_db* db, const char* query, so_duckdb_result* out) {
    if (!db || !db->open || !out) {
        return 1;
    }
    duckdb_state st = duckdb_query(db->conn, query, &out->result);
    out->open = true;
    return st == DuckDBSuccess ? 0 : 1;
}

int so_duckdb_query_void(so_duckdb_db* db, const char* query) {
    if (!db || !db->open) {
        return 1;
    }
    return duckdb_query(db->conn, query, NULL) == DuckDBSuccess ? 0 : 1;
}

int so_duckdb_prepare(so_duckdb_db* db, const char* query, so_duckdb_stmt* out) {
    if (!db || !db->open || !out) {
        return 1;
    }
    out->stmt = NULL;
    out->open = false;
    if (duckdb_prepare(db->conn, query, &out->stmt) != DuckDBSuccess) {
        return 1;
    }
    out->open = true;
    return 0;
}

const char* so_duckdb_prepare_error(so_duckdb_stmt* stmt) {
    if (!stmt || !stmt->open) {
        return NULL;
    }
    return duckdb_prepare_error(stmt->stmt);
}

void so_duckdb_stmt_close(so_duckdb_stmt* stmt) {
    if (!stmt || !stmt->open) {
        return;
    }
    duckdb_destroy_prepare(&stmt->stmt);
    stmt->open = false;
}

int so_duckdb_stmt_clear(so_duckdb_stmt* stmt) {
    if (!stmt || !stmt->open) {
        return 1;
    }
    return duckdb_clear_bindings(stmt->stmt) == DuckDBSuccess ? 0 : 1;
}

int so_duckdb_bind_null(so_duckdb_stmt* stmt, int index) {
    if (!stmt || !stmt->open || index <= 0) {
        return 1;
    }
    return duckdb_bind_null(stmt->stmt, (idx_t)index) == DuckDBSuccess ? 0 : 1;
}

int so_duckdb_bind_bool(so_duckdb_stmt* stmt, int index, bool value) {
    if (!stmt || !stmt->open || index <= 0) {
        return 1;
    }
    return duckdb_bind_boolean(stmt->stmt, (idx_t)index, value) == DuckDBSuccess ? 0 : 1;
}

int so_duckdb_bind_int64(so_duckdb_stmt* stmt, int index, int64_t value) {
    if (!stmt || !stmt->open || index <= 0) {
        return 1;
    }
    return duckdb_bind_int64(stmt->stmt, (idx_t)index, value) == DuckDBSuccess ? 0 : 1;
}

int so_duckdb_bind_double(so_duckdb_stmt* stmt, int index, double value) {
    if (!stmt || !stmt->open || index <= 0) {
        return 1;
    }
    return duckdb_bind_double(stmt->stmt, (idx_t)index, value) == DuckDBSuccess ? 0 : 1;
}

int so_duckdb_bind_varchar(so_duckdb_stmt* stmt, int index, const char* value) {
    if (!stmt || !stmt->open || index <= 0) {
        return 1;
    }
    return duckdb_bind_varchar(stmt->stmt, (idx_t)index, value) == DuckDBSuccess ? 0 : 1;
}

int so_duckdb_stmt_exec(so_duckdb_stmt* stmt, so_duckdb_result* out) {
    if (!stmt || !stmt->open || !out) {
        return 1;
    }
    duckdb_state st = duckdb_execute_prepared(stmt->stmt, &out->result);
    out->open = true;
    return st == DuckDBSuccess ? 0 : 1;
}

void so_duckdb_result_close(so_duckdb_result* res) {
    if (!res || !res->open) {
        return;
    }
    duckdb_destroy_result(&res->result);
    res->open = false;
}

const char* so_duckdb_result_error(so_duckdb_result* res) {
    if (!res || !res->open) {
        return NULL;
    }
    return duckdb_result_error(&res->result);
}

int32_t so_duckdb_result_error_type(so_duckdb_result* res) {
    if (!res || !res->open) {
        return 0;
    }
    return (int32_t)duckdb_result_error_type(&res->result);
}

int32_t so_duckdb_result_statement_type(so_duckdb_result* res) {
    if (!res || !res->open) {
        return 0;
    }
    return (int32_t)duckdb_result_statement_type(res->result);
}

int so_duckdb_result_row_count(so_duckdb_result* res) {
    if (!res || !res->open) {
        return 0;
    }
    return (int)duckdb_row_count(&res->result);
}

int so_duckdb_result_rows_changed(so_duckdb_result* res) {
    if (!res || !res->open) {
        return 0;
    }
    return (int)duckdb_rows_changed(&res->result);
}

int so_duckdb_result_column_count(so_duckdb_result* res) {
    if (!res || !res->open) {
        return 0;
    }
    return (int)duckdb_column_count(&res->result);
}

const char* so_duckdb_result_column_name(so_duckdb_result* res, int col) {
    if (!res || !res->open || col < 0) {
        return NULL;
    }
    return duckdb_column_name(&res->result, (idx_t)col);
}

int32_t so_duckdb_column_type(so_duckdb_result* res, int col) {
    if (!res || !res->open || col < 0) {
        return (int32_t)DUCKDB_TYPE_INVALID;
    }
    return (int32_t)duckdb_column_type(&res->result, (idx_t)col);
}

void* so_duckdb_column_data(so_duckdb_result* res, int col) {
    if (!res || !res->open || col < 0) {
        return NULL;
    }
    return duckdb_column_data(&res->result, (idx_t)col);
}

bool* so_duckdb_nullmask_data(so_duckdb_result* res, int col) {
    if (!res || !res->open || col < 0) {
        return NULL;
    }
    return duckdb_nullmask_data(&res->result, (idx_t)col);
}

duckdb_logical_type so_duckdb_column_logical_type(so_duckdb_result* res, int col) {
    if (!res || !res->open || col < 0) {
        return NULL;
    }
    return duckdb_column_logical_type(&res->result, (idx_t)col);
}

void so_duckdb_logical_type_destroy(duckdb_logical_type lt) {
    duckdb_logical_type local = lt;
    duckdb_destroy_logical_type(&local);
}

bool so_duckdb_value_is_null(so_duckdb_result* res, int col, int row) {
    if (!res || !res->open || col < 0 || row < 0) {
        return true;
    }
    return duckdb_value_is_null(&res->result, (idx_t)col, (idx_t)row);
}

bool so_duckdb_value_bool(so_duckdb_result* res, int col, int row) {
    return duckdb_value_boolean(&res->result, (idx_t)col, (idx_t)row);
}

int64_t so_duckdb_value_int64(so_duckdb_result* res, int col, int row) {
    return duckdb_value_int64(&res->result, (idx_t)col, (idx_t)row);
}

double so_duckdb_value_double(so_duckdb_result* res, int col, int row) {
    return duckdb_value_double(&res->result, (idx_t)col, (idx_t)row);
}

char* so_duckdb_value_string(so_duckdb_result* res, int col, int row) {
    return duckdb_value_varchar(&res->result, (idx_t)col, (idx_t)row);
}

void so_duckdb_string_free(char* ptr) {
    if (ptr) {
        duckdb_free(ptr);
    }
}

// -- Variables and constants --
so_Error duckdb_ErrOpen = errors_New("duckdb: open failed");
so_Error duckdb_ErrClosed = errors_New("duckdb: connection is closed");
so_Error duckdb_ErrQuery = errors_New("duckdb: query failed");
so_Error duckdb_ErrPrepare = errors_New("duckdb: prepare failed");
so_Error duckdb_ErrBind = errors_New("duckdb: bind failed");
so_Error duckdb_ErrExec = errors_New("duckdb: execute failed");
so_Error duckdb_ErrNullValue = errors_New("duckdb: null value");
so_Error duckdb_ErrInvalidCol = errors_New("duckdb: invalid column index");
so_Error duckdb_ErrInvalidRow = errors_New("duckdb: invalid row index");
const duckdb_ColType duckdb_ColInvalid = 0;
const duckdb_ColType duckdb_ColBoolean = 1;
const duckdb_ColType duckdb_ColTinyInt = 2;
const duckdb_ColType duckdb_ColSmallInt = 3;
const duckdb_ColType duckdb_ColInteger = 4;
const duckdb_ColType duckdb_ColBigInt = 5;
const duckdb_ColType duckdb_ColUTinyInt = 6;
const duckdb_ColType duckdb_ColUSmallInt = 7;
const duckdb_ColType duckdb_ColUInteger = 8;
const duckdb_ColType duckdb_ColUBigInt = 9;
const duckdb_ColType duckdb_ColFloat = 10;
const duckdb_ColType duckdb_ColDouble = 11;
const duckdb_ColType duckdb_ColTimestamp = 12;
const duckdb_ColType duckdb_ColDate = 13;
const duckdb_ColType duckdb_ColTime = 14;
const duckdb_ColType duckdb_ColInterval = 15;
const duckdb_ColType duckdb_ColHugeInt = 16;
const duckdb_ColType duckdb_ColUHugeInt = 32;
const duckdb_ColType duckdb_ColVarchar = 17;
const duckdb_ColType duckdb_ColBlob = 18;
const duckdb_ColType duckdb_ColDecimal = 19;
const duckdb_ColType duckdb_ColTimestampS = 20;
const duckdb_ColType duckdb_ColTimestampMS = 21;
const duckdb_ColType duckdb_ColTimestampNS = 22;
const duckdb_ColType duckdb_ColEnum = 23;
const duckdb_ColType duckdb_ColList = 24;
const duckdb_ColType duckdb_ColStruct = 25;
const duckdb_ColType duckdb_ColMap = 26;
const duckdb_ColType duckdb_ColUUID = 27;
const duckdb_ColType duckdb_ColUnion = 28;
const duckdb_ColType duckdb_ColBit = 29;
const duckdb_ColType duckdb_ColTimeTZ = 30;
const duckdb_ColType duckdb_ColTimestampTZ = 31;
const duckdb_ColType duckdb_ColArray = 33;
const duckdb_ColType duckdb_ColAny = 34;
const duckdb_ColType duckdb_ColBigNum = 35;
const duckdb_ColType duckdb_ColSQLNull = 36;
const duckdb_ColType duckdb_ColStringLiteral = 37;
const duckdb_ColType duckdb_ColIntegerLiteral = 38;
const duckdb_ColType duckdb_ColTimeNS = 39;
const duckdb_ColType duckdb_ColGeometry = 40;
const duckdb_StatementType duckdb_StatementInvalid = 0;
const duckdb_StatementType duckdb_StatementSelect = 1;
const duckdb_StatementType duckdb_StatementInsert = 2;
const duckdb_StatementType duckdb_StatementUpdate = 3;
const duckdb_StatementType duckdb_StatementExplain = 4;
const duckdb_StatementType duckdb_StatementDelete = 5;
const duckdb_StatementType duckdb_StatementPrepare = 6;
const duckdb_StatementType duckdb_StatementCreate = 7;
const duckdb_StatementType duckdb_StatementExecute = 8;
const duckdb_StatementType duckdb_StatementAlter = 9;
const duckdb_StatementType duckdb_StatementTransaction = 10;
const duckdb_StatementType duckdb_StatementCopy = 11;
const duckdb_StatementType duckdb_StatementAnalyze = 12;
const duckdb_StatementType duckdb_StatementVariableSet = 13;
const duckdb_StatementType duckdb_StatementCreateFunc = 14;
const duckdb_StatementType duckdb_StatementDrop = 15;
const duckdb_StatementType duckdb_StatementExport = 16;
const duckdb_StatementType duckdb_StatementPragma = 17;
const duckdb_StatementType duckdb_StatementVacuum = 18;
const duckdb_StatementType duckdb_StatementCall = 19;
const duckdb_StatementType duckdb_StatementSet = 20;
const duckdb_StatementType duckdb_StatementLoad = 21;
const duckdb_StatementType duckdb_StatementRelation = 22;
const duckdb_StatementType duckdb_StatementExtension = 23;
const duckdb_StatementType duckdb_StatementLogicalPlan = 24;
const duckdb_StatementType duckdb_StatementAttach = 25;
const duckdb_StatementType duckdb_StatementDetach = 26;
const duckdb_StatementType duckdb_StatementMulti = 27;
const duckdb_ErrorType duckdb_ErrorInvalid = 0;
const duckdb_ErrorType duckdb_ErrorOutOfRange = 1;
const duckdb_ErrorType duckdb_ErrorConversion = 2;
const duckdb_ErrorType duckdb_ErrorUnknownType = 3;
const duckdb_ErrorType duckdb_ErrorDecimal = 4;
const duckdb_ErrorType duckdb_ErrorMismatchType = 5;
const duckdb_ErrorType duckdb_ErrorDivideByZero = 6;
const duckdb_ErrorType duckdb_ErrorObjectSize = 7;
const duckdb_ErrorType duckdb_ErrorInvalidType = 8;
const duckdb_ErrorType duckdb_ErrorSerialization = 9;
const duckdb_ErrorType duckdb_ErrorTransaction = 10;
const duckdb_ErrorType duckdb_ErrorNotImplemented = 11;
const duckdb_ErrorType duckdb_ErrorExpression = 12;
const duckdb_ErrorType duckdb_ErrorCatalog = 13;
const duckdb_ErrorType duckdb_ErrorParser = 14;
const duckdb_ErrorType duckdb_ErrorPlanner = 15;
const duckdb_ErrorType duckdb_ErrorScheduler = 16;
const duckdb_ErrorType duckdb_ErrorExecutor = 17;
const duckdb_ErrorType duckdb_ErrorConstraint = 18;
const duckdb_ErrorType duckdb_ErrorIndex = 19;
const duckdb_ErrorType duckdb_ErrorStat = 20;
const duckdb_ErrorType duckdb_ErrorConnection = 21;
const duckdb_ErrorType duckdb_ErrorSyntax = 22;
const duckdb_ErrorType duckdb_ErrorSettings = 23;
const duckdb_ErrorType duckdb_ErrorBinder = 24;
const duckdb_ErrorType duckdb_ErrorNetwork = 25;
const duckdb_ErrorType duckdb_ErrorOptimizer = 26;
const duckdb_ErrorType duckdb_ErrorNullPointer = 27;
const duckdb_ErrorType duckdb_ErrorIO = 28;
const duckdb_ErrorType duckdb_ErrorInterrupt = 29;
const duckdb_ErrorType duckdb_ErrorFatal = 30;
const duckdb_ErrorType duckdb_ErrorInternal = 31;
const duckdb_ErrorType duckdb_ErrorInvalidInput = 32;
const duckdb_ErrorType duckdb_ErrorOutOfMemory = 33;
const duckdb_ErrorType duckdb_ErrorPermission = 34;
const duckdb_ErrorType duckdb_ErrorParameterNotResolved = 35;
const duckdb_ErrorType duckdb_ErrorParameterNotAllowed = 36;
const duckdb_ErrorType duckdb_ErrorDependency = 37;
const duckdb_ErrorType duckdb_ErrorHTTP = 38;
const duckdb_ErrorType duckdb_ErrorMissingExtension = 39;
const duckdb_ErrorType duckdb_ErrorAutoload = 40;
const duckdb_ErrorType duckdb_ErrorSequence = 41;
const duckdb_ErrorType duckdb_ErrorInvalidConfiguration = 42;

// -- duckdb.go --

// LibraryVersion returns duckdb_library_version() (e.g. "v1.5.2").
// The string is owned by DuckDB; do not free it.
so_String duckdb_LibraryVersion(void) {
    so_const_char* p = so_duckdb_library_version();
    if (p == NULL) {
        return so_str("");
    }
    return c_String(so_const_char, (p));
}

// Open creates a new DuckDB database handle and connection from a path string.
// Use a file path for an on-disk database or ":memory:" for an in-memory database string path.
//
// For the same in-memory setup as the C examples using duckdb_open(NULL), use [OpenInMemory].
duckdb_ConnResult duckdb_Open(so_String path) {
    so_duckdb_db db = {0};
    if (so_duckdb_open(so_cstr(path), &db) != 0) {
        return (duckdb_ConnResult){.val = (duckdb_Conn){}, .err = duckdb_ErrOpen};
    }
    return (duckdb_ConnResult){.val = (duckdb_Conn){.db = db}, .err = NULL};
}

// OpenInMemory opens an ephemeral database using duckdb_open(NULL) and duckdb_connect,
// matching the Startup & Shutdown C examples.
duckdb_ConnResult duckdb_OpenInMemory(void) {
    so_duckdb_db db = {0};
    if (so_duckdb_open_memory(&db) != 0) {
        return (duckdb_ConnResult){.val = (duckdb_Conn){}, .err = duckdb_ErrOpen};
    }
    return (duckdb_ConnResult){.val = (duckdb_Conn){.db = db}, .err = NULL};
}

// Close closes the connection and frees underlying DuckDB resources.
so_Error duckdb_Conn_Close(void* self) {
    duckdb_Conn* c = self;
    if (c->closed) {
        return NULL;
    }
    so_duckdb_close(&c->db);
    c->closed = true;
    return NULL;
}

// Interrupt requests interruption of the current query on this connection (duckdb_interrupt).
void duckdb_Conn_Interrupt(void* self) {
    duckdb_Conn* c = self;
    if (c->closed) {
        return;
    }
    so_duckdb_interrupt(&c->db);
}

// Query executes SQL and returns a materialized result set.
//
// If the returned error is non-nil, the [Result] still wraps the DuckDB result
// object (including error details); the caller must call [Result.Close].
duckdb_ResultResult duckdb_Conn_Query(void* self, so_String query) {
    duckdb_Conn* c = self;
    if (c->closed) {
        return (duckdb_ResultResult){.val = (duckdb_Result){}, .err = duckdb_ErrClosed};
    }
    so_duckdb_result res = {0};
    so_int rc = so_duckdb_query(&c->db, so_cstr(query), &res);
    duckdb_Result out = (duckdb_Result){.res = res};
    if (rc != 0) {
        return (duckdb_ResultResult){.val = out, .err = duckdb_ErrQuery};
    }
    return (duckdb_ResultResult){.val = out, .err = NULL};
}

// Exec executes SQL and returns the number of changed rows.
so_R_int_err duckdb_Conn_Exec(void* self, so_String query) {
    duckdb_Conn* c = self;
    duckdb_ResultResult _res1 = duckdb_Conn_Query(c, query);
    duckdb_Result res = _res1.val;
    so_Error err = _res1.err;
    if (err != NULL) {
        (void)duckdb_Result_Close(&res);
        return (so_R_int_err){.val = 0, .err = err};
    }
    so_int n = duckdb_Result_RowsChanged(&res);
    so_Error closeErr = duckdb_Result_Close(&res);
    return (so_R_int_err){.val = n, .err = closeErr};
}

// ExecSQL runs SQL without retaining a result set (duckdb_query with a NULL result pointer).
// Use this for DDL or statements where row metadata is not needed.
//
// Errors return [ErrQuery] without an error message; use [Conn.Query] when diagnostics are required.
so_Error duckdb_Conn_ExecSQL(void* self, so_String sql) {
    duckdb_Conn* c = self;
    if (c->closed) {
        return duckdb_ErrClosed;
    }
    if (so_duckdb_query_void(&c->db, so_cstr(sql)) != 0) {
        return duckdb_ErrQuery;
    }
    return NULL;
}

// Prepare creates a prepared statement.
duckdb_StmtResult duckdb_Conn_Prepare(void* self, so_String query) {
    duckdb_Conn* c = self;
    if (c->closed) {
        return (duckdb_StmtResult){.val = (duckdb_Stmt){}, .err = duckdb_ErrClosed};
    }
    so_duckdb_stmt stmt = {0};
    if (so_duckdb_prepare(&c->db, so_cstr(query), &stmt) != 0) {
        return (duckdb_StmtResult){.val = (duckdb_Stmt){}, .err = duckdb_ErrPrepare};
    }
    return (duckdb_StmtResult){.val = (duckdb_Stmt){.stmt = stmt}, .err = NULL};
}

// Close releases prepared statement resources.
so_Error duckdb_Stmt_Close(void* self) {
    duckdb_Stmt* s = self;
    if (s->closed) {
        return NULL;
    }
    so_duckdb_stmt_close(&s->stmt);
    s->closed = true;
    return NULL;
}

// ClearBindings clears all existing parameter bindings.
so_Error duckdb_Stmt_ClearBindings(void* self) {
    duckdb_Stmt* s = self;
    if (s->closed) {
        return duckdb_ErrClosed;
    }
    if (so_duckdb_stmt_clear(&s->stmt) != 0) {
        return duckdb_ErrBind;
    }
    return NULL;
}

// BindNull binds NULL to parameter index (1-based).
so_Error duckdb_Stmt_BindNull(void* self, so_int index) {
    duckdb_Stmt* s = self;
    if (s->closed) {
        return duckdb_ErrClosed;
    }
    if (so_duckdb_bind_null(&s->stmt, index) != 0) {
        return duckdb_ErrBind;
    }
    return NULL;
}

// BindBool binds a bool value to parameter index (1-based).
so_Error duckdb_Stmt_BindBool(void* self, so_int index, bool value) {
    duckdb_Stmt* s = self;
    if (s->closed) {
        return duckdb_ErrClosed;
    }
    if (so_duckdb_bind_bool(&s->stmt, index, value) != 0) {
        return duckdb_ErrBind;
    }
    return NULL;
}

// BindInt64 binds an int64 value to parameter index (1-based).
so_Error duckdb_Stmt_BindInt64(void* self, so_int index, int64_t value) {
    duckdb_Stmt* s = self;
    if (s->closed) {
        return duckdb_ErrClosed;
    }
    if (so_duckdb_bind_int64(&s->stmt, index, value) != 0) {
        return duckdb_ErrBind;
    }
    return NULL;
}

// BindFloat64 binds a float64 value to parameter index (1-based).
so_Error duckdb_Stmt_BindFloat64(void* self, so_int index, double value) {
    duckdb_Stmt* s = self;
    if (s->closed) {
        return duckdb_ErrClosed;
    }
    if (so_duckdb_bind_double(&s->stmt, index, value) != 0) {
        return duckdb_ErrBind;
    }
    return NULL;
}

// BindString binds a string value to parameter index (1-based).
so_Error duckdb_Stmt_BindString(void* self, so_int index, so_String value) {
    duckdb_Stmt* s = self;
    if (s->closed) {
        return duckdb_ErrClosed;
    }
    if (so_duckdb_bind_varchar(&s->stmt, index, so_cstr(value)) != 0) {
        return duckdb_ErrBind;
    }
    return NULL;
}

// Query executes the prepared statement and returns a result set.
//
// If the returned error is non-nil, the [Result] still must be closed.
duckdb_ResultResult duckdb_Stmt_Query(void* self) {
    duckdb_Stmt* s = self;
    if (s->closed) {
        return (duckdb_ResultResult){.val = (duckdb_Result){}, .err = duckdb_ErrClosed};
    }
    so_duckdb_result res = {0};
    so_int rc = so_duckdb_stmt_exec(&s->stmt, &res);
    duckdb_Result out = (duckdb_Result){.res = res};
    if (rc != 0) {
        return (duckdb_ResultResult){.val = out, .err = duckdb_ErrExec};
    }
    return (duckdb_ResultResult){.val = out, .err = NULL};
}

// Exec executes the prepared statement and returns changed row count.
so_R_int_err duckdb_Stmt_Exec(void* self) {
    duckdb_Stmt* s = self;
    duckdb_ResultResult _res1 = duckdb_Stmt_Query(s);
    duckdb_Result res = _res1.val;
    so_Error err = _res1.err;
    if (err != NULL) {
        (void)duckdb_Result_Close(&res);
        return (so_R_int_err){.val = 0, .err = err};
    }
    so_int n = duckdb_Result_RowsChanged(&res);
    so_Error closeErr = duckdb_Result_Close(&res);
    return (so_R_int_err){.val = n, .err = closeErr};
}

// PrepareError returns the latest prepare error for this statement.
so_String duckdb_Stmt_PrepareError(void* self) {
    duckdb_Stmt* s = self;
    so_const_char* msg = so_duckdb_prepare_error(&s->stmt);
    if (msg == NULL) {
        return so_str("");
    }
    return c_String(so_const_char, (msg));
}

// Close releases the underlying result data.
so_Error duckdb_Result_Close(void* self) {
    duckdb_Result* r = self;
    if (r->closed) {
        return NULL;
    }
    so_duckdb_result_close(&r->res);
    r->closed = true;
    return NULL;
}

// Error returns the result-level error message, if any.
//
// The pointer is owned by DuckDB; do not free it. It is valid until [Result.Close].
so_String duckdb_Result_Error(void* self) {
    duckdb_Result* r = self;
    so_const_char* msg = so_duckdb_result_error(&r->res);
    if (msg == NULL) {
        return so_str("");
    }
    return c_String(so_const_char, (msg));
}

// ErrorType returns the error classification when [Conn.Query] failed.
duckdb_ErrorType duckdb_Result_ErrorType(void* self) {
    duckdb_Result* r = self;
    if (r->closed) {
        return duckdb_ErrorInvalid;
    }
    return (duckdb_ErrorType)(so_duckdb_result_error_type(&r->res));
}

// StatementType returns the statement type that produced this result.
duckdb_StatementType duckdb_Result_StatementType(void* self) {
    duckdb_Result* r = self;
    if (r->closed) {
        return duckdb_StatementInvalid;
    }
    return (duckdb_StatementType)(so_duckdb_result_statement_type(&r->res));
}

// RowCount returns the number of rows in this result.
so_int duckdb_Result_RowCount(void* self) {
    duckdb_Result* r = self;
    return so_duckdb_result_row_count(&r->res);
}

// RowsChanged returns number of rows changed by the statement.
so_int duckdb_Result_RowsChanged(void* self) {
    duckdb_Result* r = self;
    return so_duckdb_result_rows_changed(&r->res);
}

// ColumnCount returns number of columns in this result.
so_int duckdb_Result_ColumnCount(void* self) {
    duckdb_Result* r = self;
    return so_duckdb_result_column_count(&r->res);
}

// ColumnName returns a column name by index.
so_R_str_err duckdb_Result_ColumnName(void* self, so_int col) {
    duckdb_Result* r = self;
    if (col < 0 || col >= duckdb_Result_ColumnCount(r)) {
        return (so_R_str_err){.val = so_str(""), .err = duckdb_ErrInvalidCol};
    }
    so_const_char* name = so_duckdb_result_column_name(&r->res, col);
    if (name == NULL) {
        return (so_R_str_err){.val = so_str(""), .err = duckdb_ErrInvalidCol};
    }
    return (so_R_str_err){.val = c_String(so_const_char, (name)), .err = NULL};
}

// ColumnType returns the physical SQL type of column col (see [ColType] constants).
duckdb_ColTypeResult duckdb_Result_ColumnType(void* self, so_int col) {
    duckdb_Result* r = self;
    if (r->closed) {
        return (duckdb_ColTypeResult){.val = duckdb_ColInvalid, .err = duckdb_ErrQuery};
    }
    if (col < 0 || col >= duckdb_Result_ColumnCount(r)) {
        return (duckdb_ColTypeResult){.val = duckdb_ColInvalid, .err = duckdb_ErrInvalidCol};
    }
    return (duckdb_ColTypeResult){.val = (duckdb_ColType)(so_duckdb_column_type(&r->res, col)), .err = NULL};
}

// ColumnData returns duckdb_column_data: a pointer to columnar data for col.
// Layout depends on [Result.ColumnType] / [ColType]; see DuckDB C documentation.
void* duckdb_Result_ColumnData(void* self, so_int col) {
    duckdb_Result* r = self;
    if (r->closed || col < 0 || col >= duckdb_Result_ColumnCount(r)) {
        return NULL;
    }
    return so_duckdb_column_data(&r->res, col);
}

// NullmaskData returns duckdb_nullmask_data for col, or nil if unavailable.
void* duckdb_Result_NullmaskData(void* self, so_int col) {
    duckdb_Result* r = self;
    if (r->closed || col < 0 || col >= duckdb_Result_ColumnCount(r)) {
        return NULL;
    }
    return so_duckdb_nullmask_data(&r->res, col);
}

// ColumnLogicalType returns an opaque duckdb_logical_type handle for col (see DuckDB C docs).
// Release it with [DestroyLogicalType].
so_R_ptr_err duckdb_Result_ColumnLogicalType(void* self, so_int col) {
    duckdb_Result* r = self;
    if (r->closed) {
        return (so_R_ptr_err){.val = NULL, .err = duckdb_ErrQuery};
    }
    if (col < 0 || col >= duckdb_Result_ColumnCount(r)) {
        return (so_R_ptr_err){.val = NULL, .err = duckdb_ErrInvalidCol};
    }
    void* h = so_duckdb_column_logical_type(&r->res, col);
    if (h == NULL) {
        return (so_R_ptr_err){.val = NULL, .err = duckdb_ErrInvalidCol};
    }
    return (so_R_ptr_err){.val = h, .err = NULL};
}

// DestroyLogicalType destroys a handle returned by [Result.ColumnLogicalType].
void duckdb_DestroyLogicalType(void* lt) {
    if (lt == NULL) {
        return;
    }
    so_duckdb_logical_type_destroy(lt);
}

// IsNull reports whether a value at (row, col) is null.
so_R_bool_err duckdb_Result_IsNull(void* self, so_int row, so_int col) {
    duckdb_Result* r = self;
    if (row < 0 || row >= duckdb_Result_RowCount(r)) {
        return (so_R_bool_err){.val = false, .err = duckdb_ErrInvalidRow};
    }
    if (col < 0 || col >= duckdb_Result_ColumnCount(r)) {
        return (so_R_bool_err){.val = false, .err = duckdb_ErrInvalidCol};
    }
    return (so_R_bool_err){.val = so_duckdb_value_is_null(&r->res, col, row), .err = NULL};
}

// Bool returns a bool value from (row, col).
so_R_bool_err duckdb_Result_Bool(void* self, so_int row, so_int col) {
    duckdb_Result* r = self;
    so_R_bool_err _res1 = duckdb_Result_IsNull(r, row, col);
    bool isNull = _res1.val;
    so_Error err = _res1.err;
    if (err != NULL) {
        return (so_R_bool_err){.val = false, .err = err};
    }
    if (isNull) {
        return (so_R_bool_err){.val = false, .err = duckdb_ErrNullValue};
    }
    return (so_R_bool_err){.val = so_duckdb_value_bool(&r->res, col, row), .err = NULL};
}

// Int64 returns an int64 value from (row, col).
so_R_i64_err duckdb_Result_Int64(void* self, so_int row, so_int col) {
    duckdb_Result* r = self;
    so_R_bool_err _res1 = duckdb_Result_IsNull(r, row, col);
    bool isNull = _res1.val;
    so_Error err = _res1.err;
    if (err != NULL) {
        return (so_R_i64_err){.val = 0, .err = err};
    }
    if (isNull) {
        return (so_R_i64_err){.val = 0, .err = duckdb_ErrNullValue};
    }
    return (so_R_i64_err){.val = so_duckdb_value_int64(&r->res, col, row), .err = NULL};
}

// Float64 returns a float64 value from (row, col).
so_R_f64_err duckdb_Result_Float64(void* self, so_int row, so_int col) {
    duckdb_Result* r = self;
    so_R_bool_err _res1 = duckdb_Result_IsNull(r, row, col);
    bool isNull = _res1.val;
    so_Error err = _res1.err;
    if (err != NULL) {
        return (so_R_f64_err){.val = 0, .err = err};
    }
    if (isNull) {
        return (so_R_f64_err){.val = 0, .err = duckdb_ErrNullValue};
    }
    return (so_R_f64_err){.val = so_duckdb_value_double(&r->res, col, row), .err = NULL};
}

// StringCopy returns a heap-allocated copy of a string value from (row, col).
// The caller owns the returned string and must free it using mem.FreeString.
so_R_str_err duckdb_Result_StringCopy(void* self, mem_Allocator a, so_int row, so_int col) {
    duckdb_Result* r = self;
    so_R_bool_err _res1 = duckdb_Result_IsNull(r, row, col);
    bool isNull = _res1.val;
    so_Error err = _res1.err;
    if (err != NULL) {
        return (so_R_str_err){.val = so_str(""), .err = err};
    }
    if (isNull) {
        return (so_R_str_err){.val = so_str(""), .err = duckdb_ErrNullValue};
    }
    char* ptr = so_duckdb_value_string(&r->res, col, row);
    if (ptr == NULL) {
        return (so_R_str_err){.val = so_str(""), .err = duckdb_ErrQuery};
    }
    so_String tmp = c_String(char, (ptr));
    so_String out = strings_Clone(a, tmp);
    so_duckdb_string_free(ptr);
    return (so_R_str_err){.val = out, .err = NULL};
}

// Rows creates a row iterator over this result.
duckdb_Rows duckdb_Result_Rows(void* self) {
    duckdb_Result* r = self;
    return (duckdb_Rows){.result = r, .row = -1};
}

// Next advances to the next row and reports whether one exists.
bool duckdb_Rows_Next(void* self) {
    duckdb_Rows* rs = self;
    if (rs->result == NULL) {
        return false;
    }
    rs->row++;
    return rs->row < duckdb_Result_RowCount(rs->result);
}

// Row returns the current row index.
so_int duckdb_Rows_Row(void* self) {
    duckdb_Rows* rs = self;
    return rs->row;
}

// IsNull reports whether current row value at col is null.
so_R_bool_err duckdb_Rows_IsNull(void* self, so_int col) {
    duckdb_Rows* rs = self;
    if (rs->result == NULL) {
        return (so_R_bool_err){.val = false, .err = duckdb_ErrQuery};
    }
    return duckdb_Result_IsNull(rs->result, rs->row, col);
}

// Bool reads a bool from current row at col.
so_R_bool_err duckdb_Rows_Bool(void* self, so_int col) {
    duckdb_Rows* rs = self;
    if (rs->result == NULL) {
        return (so_R_bool_err){.val = false, .err = duckdb_ErrQuery};
    }
    return duckdb_Result_Bool(rs->result, rs->row, col);
}

// Int64 reads an int64 from current row at col.
so_R_i64_err duckdb_Rows_Int64(void* self, so_int col) {
    duckdb_Rows* rs = self;
    if (rs->result == NULL) {
        return (so_R_i64_err){.val = 0, .err = duckdb_ErrQuery};
    }
    return duckdb_Result_Int64(rs->result, rs->row, col);
}

// Float64 reads a float64 from current row at col.
so_R_f64_err duckdb_Rows_Float64(void* self, so_int col) {
    duckdb_Rows* rs = self;
    if (rs->result == NULL) {
        return (so_R_f64_err){.val = 0, .err = duckdb_ErrQuery};
    }
    return duckdb_Result_Float64(rs->result, rs->row, col);
}

// StringCopy reads a string from current row at col and clones it.
// The caller owns the returned string and must free it using mem.FreeString.
so_R_str_err duckdb_Rows_StringCopy(void* self, mem_Allocator a, so_int col) {
    duckdb_Rows* rs = self;
    if (rs->result == NULL) {
        return (so_R_str_err){.val = so_str(""), .err = duckdb_ErrQuery};
    }
    return duckdb_Result_StringCopy(rs->result, a, rs->row, col);
}

// -- extern.go --

// -- types.go --
