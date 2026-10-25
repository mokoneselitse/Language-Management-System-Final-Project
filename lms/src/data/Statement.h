// Statement.h - wraps one prepared SQLite statement (sqlite3_stmt*).
//
// Values are always bound to ? placeholders with bind(), never glued into the
// SQL text, so user input cannot change the query (SQL-injection safe).
// The destructor calls sqlite3_finalize, so a Statement cleans up after itself.
#pragma once

#include <string>

struct sqlite3;
struct sqlite3_stmt;

class Statement {
public:
    Statement(sqlite3* db, const std::string& sql);
    ~Statement();

    // A Statement owns its sqlite3_stmt, so it can be moved but not copied.
    Statement(Statement&& other) noexcept;
    Statement(const Statement&) = delete;
    Statement& operator=(const Statement&) = delete;
    Statement& operator=(Statement&&) = delete;

    // Placeholders are numbered from 1, like in SQLite.
    Statement& bind(int index, int value);
    Statement& bind(int index, long long value);  // for Unix timestamps
    Statement& bind(int index, double value);
    Statement& bind(int index, const std::string& value);
    Statement& bindNull(int index);

    // Runs the statement. Returns true if a row is available, false when done.
    bool step();

    // Columns are numbered from 0, like in SQLite.
    int columnInt(int col);
    long long columnInt64(int col);  // for Unix timestamps
    double columnDouble(int col);
    std::string columnText(int col);
    bool isNull(int col);

private:
    void check(int resultCode);

    sqlite3* db;
    sqlite3_stmt* stmt = nullptr;
};
