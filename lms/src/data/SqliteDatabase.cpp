#include "SqliteDatabase.h"

#include <sqlite3.h>

#include "Errors.h"
#include "Schema.h"

SqliteDatabase::SqliteDatabase(const std::string& path) {
    if (sqlite3_open(path.c_str(), &handle) != SQLITE_OK) {
        std::string message = sqlite3_errmsg(handle);
        sqlite3_close(handle);
        throw DatabaseError("Could not open database " + path + ": " + message);
    }
    execute("PRAGMA foreign_keys = ON;");  // SQLite turns foreign keys off by default
    execute(SCHEMA_SQL);
}

SqliteDatabase::~SqliteDatabase() { sqlite3_close(handle); }

Statement SqliteDatabase::prepare(const std::string& sql) { return Statement(handle, sql); }

void SqliteDatabase::execute(const std::string& sql) {
    char* error = nullptr;
    if (sqlite3_exec(handle, sql.c_str(), nullptr, nullptr, &error) != SQLITE_OK) {
        std::string message = error != nullptr ? error : "unknown error";
        sqlite3_free(error);
        throw DatabaseError(message);
    }
}

void SqliteDatabase::runInTransaction(std::function<void()> work) {
    execute("BEGIN;");
    try {
        work();
        execute("COMMIT;");
    } catch (...) {
        execute("ROLLBACK;");  // undo everything done inside work()
        throw;
    }
}

int SqliteDatabase::lastInsertId() { return static_cast<int>(sqlite3_last_insert_rowid(handle)); }
