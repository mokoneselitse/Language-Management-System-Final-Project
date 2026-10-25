// SqliteDatabase.h - owns the connection to lms.db.
// Opening the database also creates the tables if they do not exist yet.
#pragma once

#include <functional>
#include <string>

#include "IUnitOfWork.h"
#include "Statement.h"

struct sqlite3;

class SqliteDatabase : public IUnitOfWork {
public:
    explicit SqliteDatabase(const std::string& path);
    ~SqliteDatabase() override;

    SqliteDatabase(const SqliteDatabase&) = delete;
    SqliteDatabase& operator=(const SqliteDatabase&) = delete;

    Statement prepare(const std::string& sql);
    void execute(const std::string& sql);  // for SQL without parameters
    void runInTransaction(std::function<void()> work) override;
    int lastInsertId();

private:
    sqlite3* handle = nullptr;
};
