#include "main.h"

// -- Implementation --

int main(void) {
    so_String libVer = duckdb_LibraryVersion();
    if (so_len(libVer) == 0) {
        so_panic("library version empty");
    }
    so_println("%s %.*s", "duckdb library:", libVer.len, libVer.ptr);
    duckdb_ConnResult _res1 = duckdb_OpenInMemory();
    duckdb_Conn db = _res1.val;
    so_Error err = _res1.err;
    if (err != NULL) {
        so_panic("duckdb open failed");
    }
    duckdb_Conn_Interrupt(&db);
    {
        // Smoke-test query path (duckdb_query + result metadata).
        duckdb_ResultResult _res2 = duckdb_Conn_Query(&db, so_str("SELECT version()"));
        duckdb_Result res = _res2.val;
        so_Error err = _res2.err;
        if (err != NULL) {
            so_panic("duckdb query failed");
        }
        if (duckdb_Result_RowCount(&res) != 1) {
            duckdb_Result_Close(&res);
            so_panic("expected one row");
        }
        if (duckdb_Result_ColumnCount(&res) != 1) {
            duckdb_Result_Close(&res);
            so_panic("expected one column");
        }
        if (duckdb_Result_StatementType(&res) != duckdb_StatementSelect) {
            duckdb_Result_Close(&res);
            so_panic("expected SELECT statement type");
        }
        duckdb_ColTypeResult _res3 = duckdb_Result_ColumnType(&res, 0);
        duckdb_ColType typ = _res3.val;
        err = _res3.err;
        if (err != NULL) {
            duckdb_Result_Close(&res);
            so_panic("column type failed");
        }
        if (typ != duckdb_ColVarchar) {
            duckdb_Result_Close(&res);
            so_panic("expected VARCHAR column");
        }
        so_R_ptr_err _res4 = duckdb_Result_ColumnLogicalType(&res, 0);
        void* lt = _res4.val;
        err = _res4.err;
        if (err != NULL) {
            duckdb_Result_Close(&res);
            so_panic("logical type failed");
        }
        duckdb_DestroyLogicalType(lt);
        so_R_str_err _res5 = duckdb_Result_ColumnName(&res, 0);
        so_String name = _res5.val;
        err = _res5.err;
        if (err != NULL) {
            duckdb_Result_Close(&res);
            so_panic("column name failed");
        }
        if (so_len(name) == 0) {
            duckdb_Result_Close(&res);
            so_panic("empty column name");
        }
        so_R_str_err _res6 = duckdb_Result_StringCopy(&res, mem_System, 0, 0);
        so_String version = _res6.val;
        err = _res6.err;
        if (err != NULL) {
            duckdb_Result_Close(&res);
            so_panic("version read failed");
        }
        if (so_len(version) == 0) {
            duckdb_Result_Close(&res);
            so_panic("empty version");
        }
        if (!strings_Contains(version, so_str("v"))) {
            duckdb_Result_Close(&res);
            so_panic("unexpected version format");
        }
        so_println("%s %.*s", "duckdb version:", version.len, version.ptr);
        mem_FreeString(mem_System, version);
        duckdb_Result_Close(&res);
    }
    {
        // Failed query: duckdb_query fills error state; Result.Close must still run.
        duckdb_ResultResult _res7 = duckdb_Conn_Query(&db, so_str("SELECT * FROM duckdb_missing_table_so_test_xx"));
        duckdb_Result badRes = _res7.val;
        so_Error err = _res7.err;
        if (err == NULL) {
            duckdb_Result_Close(&badRes);
            so_panic("expected query error");
        }
        if (so_string_eq(duckdb_Result_Error(&badRes), so_str(""))) {
            duckdb_Result_Close(&badRes);
            so_panic("expected error message on failed result");
        }
        duckdb_Result_Close(&badRes);
    }
    {
        // Prepared statements + typed binds/reads.
        so_Error err = duckdb_Conn_ExecSQL(&db, so_str("CREATE TABLE people(id BIGINT, name VARCHAR, score DOUBLE)"));
        if (err != NULL) {
            so_panic("create table failed");
        }
        duckdb_StmtResult _res8 = duckdb_Conn_Prepare(&db, so_str("INSERT INTO people VALUES (?, ?, ?)"));
        duckdb_Stmt stmt = _res8.val;
        err = _res8.err;
        if (err != NULL) {
            so_panic("prepare insert failed");
        }
        err = duckdb_Stmt_BindInt64(&stmt, 1, 1);
        if (err != NULL) {
            duckdb_Stmt_Close(&stmt);
            so_panic("bind int failed");
        }
        err = duckdb_Stmt_BindString(&stmt, 2, so_str("Alice"));
        if (err != NULL) {
            duckdb_Stmt_Close(&stmt);
            so_panic("bind string failed");
        }
        err = duckdb_Stmt_BindFloat64(&stmt, 3, 9.5);
        if (err != NULL) {
            duckdb_Stmt_Close(&stmt);
            so_panic("bind float failed");
        }
        so_R_int_err _res9 = duckdb_Stmt_Exec(&stmt);
        err = _res9.err;
        if (err != NULL) {
            duckdb_Stmt_Close(&stmt);
            so_panic("insert exec failed");
        }
        err = duckdb_Stmt_ClearBindings(&stmt);
        if (err != NULL) {
            duckdb_Stmt_Close(&stmt);
            so_panic("clear bindings failed");
        }
        err = duckdb_Stmt_BindInt64(&stmt, 1, 2);
        if (err != NULL) {
            duckdb_Stmt_Close(&stmt);
            so_panic("bind int #2 failed");
        }
        err = duckdb_Stmt_BindString(&stmt, 2, so_str("Bob"));
        if (err != NULL) {
            duckdb_Stmt_Close(&stmt);
            so_panic("bind string #2 failed");
        }
        err = duckdb_Stmt_BindFloat64(&stmt, 3, 7.25);
        if (err != NULL) {
            duckdb_Stmt_Close(&stmt);
            so_panic("bind float #2 failed");
        }
        so_R_int_err _res10 = duckdb_Stmt_Exec(&stmt);
        err = _res10.err;
        if (err != NULL) {
            duckdb_Stmt_Close(&stmt);
            so_panic("insert exec #2 failed");
        }
        duckdb_ResultResult _res11 = duckdb_Conn_Query(&db, so_str("SELECT id, name, score FROM people ORDER BY id"));
        duckdb_Result rowsRes = _res11.val;
        err = _res11.err;
        if (err != NULL) {
            duckdb_Stmt_Close(&stmt);
            so_panic("select rows failed");
        }
        if (duckdb_Result_RowCount(&rowsRes) != 2) {
            duckdb_Result_Close(&rowsRes);
            duckdb_Stmt_Close(&stmt);
            so_panic("expected two rows");
        }
        void* idCol = duckdb_Result_ColumnData(&rowsRes, 0);
        if (idCol == NULL) {
            duckdb_Result_Close(&rowsRes);
            duckdb_Stmt_Close(&stmt);
            so_panic("expected column data pointer");
        }
        void* nm = duckdb_Result_NullmaskData(&rowsRes, 0);
        if (nm == NULL) {
            duckdb_Result_Close(&rowsRes);
            duckdb_Stmt_Close(&stmt);
            so_panic("expected nullmask pointer");
        }
        duckdb_Rows rows = duckdb_Result_Rows(&rowsRes);
        so_int count = 0;
        for (; duckdb_Rows_Next(&rows);) {
            so_R_i64_err _res12 = duckdb_Rows_Int64(&rows, 0);
            int64_t id = _res12.val;
            so_Error err = _res12.err;
            if (err != NULL) {
                duckdb_Result_Close(&rowsRes);
                duckdb_Stmt_Close(&stmt);
                so_panic("read id failed");
            }
            so_R_str_err _res13 = duckdb_Rows_StringCopy(&rows, mem_System, 1);
            so_String name = _res13.val;
            err = _res13.err;
            if (err != NULL) {
                duckdb_Result_Close(&rowsRes);
                duckdb_Stmt_Close(&stmt);
                so_panic("read name failed");
            }
            so_R_f64_err _res14 = duckdb_Rows_Float64(&rows, 2);
            double score = _res14.val;
            err = _res14.err;
            if (err != NULL) {
                duckdb_Result_Close(&rowsRes);
                duckdb_Stmt_Close(&stmt);
                so_panic("read score failed");
            }
            so_println("%s %" PRId64 " %.*s %f", "row:", id, name.len, name.ptr, score);
            mem_FreeString(mem_System, name);
            count++;
        }
        if (count != 2) {
            duckdb_Result_Close(&rowsRes);
            duckdb_Stmt_Close(&stmt);
            so_panic("unexpected row count");
        }
        duckdb_Result_Close(&rowsRes);
        duckdb_Stmt_Close(&stmt);
    }
    duckdb_Conn_Close(&db);
}
