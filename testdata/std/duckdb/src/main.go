package main

import (
	"solod.dev/so/duckdb"
	"solod.dev/so/mem"
	"solod.dev/so/strings"
)

func main() {
	db, err := duckdb.Open(":memory:")
	if err != nil {
		panic("duckdb open failed")
	}
	defer db.Close()

	{
		// Smoke-test query path.
		res, err := db.Query("SELECT version()")
		if err != nil {
			panic("duckdb query failed")
		}
		defer res.Close()

		if res.RowCount() != 1 {
			panic("expected one row")
		}
		if res.ColumnCount() != 1 {
			panic("expected one column")
		}

		name, err := res.ColumnName(0)
		if err != nil {
			panic("column name failed")
		}
		if len(name) == 0 {
			panic("empty column name")
		}

		version, err := res.StringCopy(mem.System, 0, 0)
		if err != nil {
			panic("version read failed")
		}
		if len(version) == 0 {
			panic("empty version")
		}
		if !strings.Contains(version, "v") {
			panic("unexpected version format")
		}
		println("duckdb version:", version)
		mem.FreeString(mem.System, version)
	}

	{
		// Prepared statements + typed binds/reads.
		_, err := db.Exec("CREATE TABLE people(id BIGINT, name VARCHAR, score DOUBLE)")
		if err != nil {
			panic("create table failed")
		}

		stmt, err := db.Prepare("INSERT INTO people VALUES (?, ?, ?)")
		if err != nil {
			panic("prepare insert failed")
		}
		defer stmt.Close()

		err = stmt.BindInt64(1, 1)
		if err != nil {
			panic("bind int failed")
		}
		err = stmt.BindString(2, "Alice")
		if err != nil {
			panic("bind string failed")
		}
		err = stmt.BindFloat64(3, 9.5)
		if err != nil {
			panic("bind float failed")
		}
		_, err = stmt.Exec()
		if err != nil {
			panic("insert exec failed")
		}

		err = stmt.ClearBindings()
		if err != nil {
			panic("clear bindings failed")
		}

		err = stmt.BindInt64(1, 2)
		if err != nil {
			panic("bind int #2 failed")
		}
		err = stmt.BindString(2, "Bob")
		if err != nil {
			panic("bind string #2 failed")
		}
		err = stmt.BindFloat64(3, 7.25)
		if err != nil {
			panic("bind float #2 failed")
		}
		_, err = stmt.Exec()
		if err != nil {
			panic("insert exec #2 failed")
		}

		rowsRes, err := db.Query("SELECT id, name, score FROM people ORDER BY id")
		if err != nil {
			panic("select rows failed")
		}
		defer rowsRes.Close()

		if rowsRes.RowCount() != 2 {
			panic("expected two rows")
		}

		rows := rowsRes.Rows()
		count := 0
		for rows.Next() {
			id, err := rows.Int64(0)
			if err != nil {
				panic("read id failed")
			}
			name, err := rows.StringCopy(mem.System, 1)
			if err != nil {
				panic("read name failed")
			}
			score, err := rows.Float64(2)
			if err != nil {
				panic("read score failed")
			}

			println("row:", id, name, score)
			mem.FreeString(mem.System, name)
			count++
		}
		if count != 2 {
			panic("unexpected row count")
		}
	}
}
