#include "Statement.h"

#include <sqlite3.h>

#include "Errors.h"

Statement::Statement(sqlite3* db, const std::string& sql) : db(db) {
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        throw DatabaseError("Could not prepare SQL: " + std::string(sqlite3_errmsg(db)));
    }
}

Statement::~Statement() { sqlite3_finalize(stmt); }  // finalize(nullptr) is a no-op

Statement::Statement(Statement&& other) noexcept : db(other.db), stmt(other.stmt) {
    other.stmt = nullptr;
}

void Statement::check(int resultCode) {
    if (resultCode != SQLITE_OK) {
        throw DatabaseError("Could not bind value: " + std::string(sqlite3_errmsg(db)));
    }
}

Statement& Statement::bind(int index, int value) {
    check(sqlite3_bind_int(stmt, index, value));
    return *this;
}

Statement& Statement::bind(int index, long long value) {
    check(sqlite3_bind_int64(stmt, index, value));
    return *this;
}

Statement& Statement::bind(int index, double value) {
    check(sqlite3_bind_double(stmt, index, value));
    return *this;
}

Statement& Statement::bind(int index, const std::string& value) {
    check(sqlite3_bind_text(stmt, index, value.c_str(), -1, SQLITE_TRANSIENT));
    return *this;
}

Statement& Statement::bindNull(int index) {
    check(sqlite3_bind_null(stmt, index));
    return *this;
}

bool Statement::step() {
    int rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) return true;
    if (rc == SQLITE_DONE) return false;
    throw DatabaseError(sqlite3_errmsg(db));
}

int Statement::columnInt(int col) { return sqlite3_column_int(stmt, col); }

long long Statement::columnInt64(int col) { return sqlite3_column_int64(stmt, col); }

double Statement::columnDouble(int col) { return sqlite3_column_double(stmt, col); }

std::string Statement::columnText(int col) {
    const unsigned char* text = sqlite3_column_text(stmt, col);
    if (text == nullptr) return "";
    return reinterpret_cast<const char*>(text);
}

bool Statement::isNull(int col) { return sqlite3_column_type(stmt, col) == SQLITE_NULL; }
