// Package duckdb provides native DuckDB integration for Solod.
//
// This package is designed around DuckDB's C API with explicit resource
// ownership and predictable lifecycles.
//
// Building translated programs that import this package requires DuckDB headers
// and library to be available to the C compiler and linker (e.g. `-lduckdb`).
package duckdb

import (
	"solod.dev/so/c"
	"solod.dev/so/errors"
	"solod.dev/so/mem"
	"solod.dev/so/strings"
)

var (
	ErrOpen       = errors.New("duckdb: open failed")
	ErrClosed     = errors.New("duckdb: connection is closed")
	ErrQuery      = errors.New("duckdb: query failed")
	ErrPrepare    = errors.New("duckdb: prepare failed")
	ErrBind       = errors.New("duckdb: bind failed")
	ErrExec       = errors.New("duckdb: execute failed")
	ErrNullValue  = errors.New("duckdb: null value")
	ErrInvalidCol = errors.New("duckdb: invalid column index")
	ErrInvalidRow = errors.New("duckdb: invalid row index")
)

// Conn is a connection to a DuckDB database instance.
type Conn struct {
	db     dbHandle
	closed bool
}

// ConnResult is a helper struct for returning
// a Conn and an error from a function.
type ConnResult struct {
	val Conn
	err error
}

// Stmt is a prepared statement bound to a connection.
type Stmt struct {
	stmt   stmtHandle
	closed bool
}

// StmtResult is a helper struct for returning
// a Stmt and an error from a function.
type StmtResult struct {
	val Stmt
	err error
}

// Result owns a DuckDB query result.
type Result struct {
	res    resultHandle
	closed bool
}

// ResultResult is a helper struct for returning
// a Result and an error from a function.
type ResultResult struct {
	val Result
	err error
}

// Rows is a row iterator over a Result.
type Rows struct {
	result *Result
	row    int
}

// Open creates a new DuckDB connection.
//
// Use ":memory:" for in-memory databases.
func Open(path string) (Conn, error) {
	var db dbHandle
	if so_duckdb_open(path, &db) != 0 {
		return Conn{}, ErrOpen
	}
	return Conn{db: db}, nil
}

// Close closes the connection and frees underlying DuckDB resources.
func (c *Conn) Close() error {
	if c.closed {
		return nil
	}
	so_duckdb_close(&c.db)
	c.closed = true
	return nil
}

// Query executes SQL and returns a materialized result set.
func (c *Conn) Query(query string) (Result, error) {
	if c.closed {
		return Result{}, ErrClosed
	}
	var res resultHandle
	if so_duckdb_query(&c.db, query, &res) != 0 {
		return Result{}, ErrQuery
	}
	return Result{res: res}, nil
}

// Exec executes SQL and returns the number of changed rows.
func (c *Conn) Exec(query string) (int, error) {
	res, err := c.Query(query)
	if err != nil {
		return 0, err
	}
	defer res.Close()
	return res.RowsChanged(), nil
}

// Prepare creates a prepared statement.
func (c *Conn) Prepare(query string) (Stmt, error) {
	if c.closed {
		return Stmt{}, ErrClosed
	}
	var stmt stmtHandle
	if so_duckdb_prepare(&c.db, query, &stmt) != 0 {
		return Stmt{}, ErrPrepare
	}
	return Stmt{stmt: stmt}, nil
}

// Close releases prepared statement resources.
func (s *Stmt) Close() error {
	if s.closed {
		return nil
	}
	so_duckdb_stmt_close(&s.stmt)
	s.closed = true
	return nil
}

// ClearBindings clears all existing parameter bindings.
func (s *Stmt) ClearBindings() error {
	if s.closed {
		return ErrClosed
	}
	if so_duckdb_stmt_clear(&s.stmt) != 0 {
		return ErrBind
	}
	return nil
}

// BindNull binds NULL to parameter index (1-based).
func (s *Stmt) BindNull(index int) error {
	if s.closed {
		return ErrClosed
	}
	if so_duckdb_bind_null(&s.stmt, index) != 0 {
		return ErrBind
	}
	return nil
}

// BindBool binds a bool value to parameter index (1-based).
func (s *Stmt) BindBool(index int, value bool) error {
	if s.closed {
		return ErrClosed
	}
	if so_duckdb_bind_bool(&s.stmt, index, value) != 0 {
		return ErrBind
	}
	return nil
}

// BindInt64 binds an int64 value to parameter index (1-based).
func (s *Stmt) BindInt64(index int, value int64) error {
	if s.closed {
		return ErrClosed
	}
	if so_duckdb_bind_int64(&s.stmt, index, value) != 0 {
		return ErrBind
	}
	return nil
}

// BindFloat64 binds a float64 value to parameter index (1-based).
func (s *Stmt) BindFloat64(index int, value float64) error {
	if s.closed {
		return ErrClosed
	}
	if so_duckdb_bind_double(&s.stmt, index, value) != 0 {
		return ErrBind
	}
	return nil
}

// BindString binds a string value to parameter index (1-based).
func (s *Stmt) BindString(index int, value string) error {
	if s.closed {
		return ErrClosed
	}
	if so_duckdb_bind_varchar(&s.stmt, index, value) != 0 {
		return ErrBind
	}
	return nil
}

// Query executes the prepared statement and returns a result set.
func (s *Stmt) Query() (Result, error) {
	if s.closed {
		return Result{}, ErrClosed
	}
	var res resultHandle
	if so_duckdb_stmt_exec(&s.stmt, &res) != 0 {
		return Result{}, ErrExec
	}
	return Result{res: res}, nil
}

// Exec executes the prepared statement and returns changed row count.
func (s *Stmt) Exec() (int, error) {
	res, err := s.Query()
	if err != nil {
		return 0, err
	}
	defer res.Close()
	return res.RowsChanged(), nil
}

// PrepareError returns the latest prepare error for this statement.
func (s *Stmt) PrepareError() string {
	msg := so_duckdb_prepare_error(&s.stmt)
	if msg == nil {
		return ""
	}
	return c.String(msg)
}

// Close releases the underlying result data.
func (r *Result) Close() error {
	if r.closed {
		return nil
	}
	so_duckdb_result_close(&r.res)
	r.closed = true
	return nil
}

// Error returns the result-level error message, if any.
func (r *Result) Error() string {
	msg := so_duckdb_result_error(&r.res)
	if msg == nil {
		return ""
	}
	return c.String(msg)
}

// RowCount returns the number of rows in this result.
func (r *Result) RowCount() int {
	return so_duckdb_result_row_count(&r.res)
}

// RowsChanged returns number of rows changed by the statement.
func (r *Result) RowsChanged() int {
	return so_duckdb_result_rows_changed(&r.res)
}

// ColumnCount returns number of columns in this result.
func (r *Result) ColumnCount() int {
	return so_duckdb_result_column_count(&r.res)
}

// ColumnName returns a column name by index.
func (r *Result) ColumnName(col int) (string, error) {
	if col < 0 || col >= r.ColumnCount() {
		return "", ErrInvalidCol
	}
	name := so_duckdb_result_column_name(&r.res, col)
	if name == nil {
		return "", ErrInvalidCol
	}
	return c.String(name), nil
}

// IsNull reports whether a value at (row, col) is null.
func (r *Result) IsNull(row int, col int) (bool, error) {
	if row < 0 || row >= r.RowCount() {
		return false, ErrInvalidRow
	}
	if col < 0 || col >= r.ColumnCount() {
		return false, ErrInvalidCol
	}
	return so_duckdb_value_is_null(&r.res, col, row), nil
}

// Bool returns a bool value from (row, col).
func (r *Result) Bool(row int, col int) (bool, error) {
	isNull, err := r.IsNull(row, col)
	if err != nil {
		return false, err
	}
	if isNull {
		return false, ErrNullValue
	}
	return so_duckdb_value_bool(&r.res, col, row), nil
}

// Int64 returns an int64 value from (row, col).
func (r *Result) Int64(row int, col int) (int64, error) {
	isNull, err := r.IsNull(row, col)
	if err != nil {
		return 0, err
	}
	if isNull {
		return 0, ErrNullValue
	}
	return so_duckdb_value_int64(&r.res, col, row), nil
}

// Float64 returns a float64 value from (row, col).
func (r *Result) Float64(row int, col int) (float64, error) {
	isNull, err := r.IsNull(row, col)
	if err != nil {
		return 0, err
	}
	if isNull {
		return 0, ErrNullValue
	}
	return so_duckdb_value_double(&r.res, col, row), nil
}

// StringCopy returns a heap-allocated copy of a string value from (row, col).
// The caller owns the returned string and must free it using mem.FreeString.
func (r *Result) StringCopy(a mem.Allocator, row int, col int) (string, error) {
	isNull, err := r.IsNull(row, col)
	if err != nil {
		return "", err
	}
	if isNull {
		return "", ErrNullValue
	}
	ptr := so_duckdb_value_string(&r.res, col, row)
	if ptr == nil {
		return "", ErrQuery
	}
	tmp := c.String(ptr)
	out := strings.Clone(a, tmp)
	so_duckdb_string_free(ptr)
	return out, nil
}

// Rows creates a row iterator over this result.
func (r *Result) Rows() Rows {
	return Rows{
		result: r,
		row:    -1,
	}
}

// Next advances to the next row and reports whether one exists.
func (rs *Rows) Next() bool {
	if rs.result == nil {
		return false
	}
	rs.row++
	return rs.row < rs.result.RowCount()
}

// Row returns the current row index.
func (rs *Rows) Row() int {
	return rs.row
}

// IsNull reports whether current row value at col is null.
func (rs *Rows) IsNull(col int) (bool, error) {
	if rs.result == nil {
		return false, ErrQuery
	}
	return rs.result.IsNull(rs.row, col)
}

// Bool reads a bool from current row at col.
func (rs *Rows) Bool(col int) (bool, error) {
	if rs.result == nil {
		return false, ErrQuery
	}
	return rs.result.Bool(rs.row, col)
}

// Int64 reads an int64 from current row at col.
func (rs *Rows) Int64(col int) (int64, error) {
	if rs.result == nil {
		return 0, ErrQuery
	}
	return rs.result.Int64(rs.row, col)
}

// Float64 reads a float64 from current row at col.
func (rs *Rows) Float64(col int) (float64, error) {
	if rs.result == nil {
		return 0, ErrQuery
	}
	return rs.result.Float64(rs.row, col)
}

// StringCopy reads a string from current row at col and clones it.
// The caller owns the returned string and must free it using mem.FreeString.
func (rs *Rows) StringCopy(a mem.Allocator, col int) (string, error) {
	if rs.result == nil {
		return "", ErrQuery
	}
	return rs.result.StringCopy(a, rs.row, col)
}
