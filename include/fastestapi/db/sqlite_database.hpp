#pragma once

#include <fastestapi/interfaces/i_database.hpp>
#include <sqlite3.h>
#include <stdexcept>
#include <string>
#include <iostream>

namespace fastestapi {

class SqliteDatabase : public IDatabase {
public:
    ~SqliteDatabase() override { disconnect(); }

    bool connect(const std::string& path) override {
        if (db_) disconnect();
        int rc = sqlite3_open(path.c_str(), &db_);
        if (rc != SQLITE_OK) {
            std::string err = sqlite3_errmsg(db_);
            sqlite3_close(db_);
            db_ = nullptr;
            throw std::runtime_error("SQLite open failed: " + err);
        }
        execute("PRAGMA journal_mode=WAL;");
        execute("PRAGMA foreign_keys=ON;");
        std::cout << "[FastestAPI] Connected to SQLite: " << path << "\n";
        return true;
    }

    void disconnect() override {
        if (db_) { sqlite3_close(db_); db_ = nullptr; }
    }

    bool isConnected() const override { return db_ != nullptr; }

    int execute(const std::string& sql,
                const std::vector<DbValue>& params = {}) override {
        sqlite3_stmt* stmt = prepare(sql);
        bindParams(stmt, params);

        int rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE && rc != SQLITE_ROW) {
            std::string err = sqlite3_errmsg(db_);
            sqlite3_finalize(stmt);
            throw std::runtime_error("SQL execute error: " + err);
        }
        sqlite3_finalize(stmt);
        return sqlite3_changes(db_);
    }

    std::vector<DbRow> query(const std::string& sql,
                             const std::vector<DbValue>& params = {}) override {
        sqlite3_stmt* stmt = prepare(sql);
        bindParams(stmt, params);

        std::vector<DbRow> rows;
        int colCount = sqlite3_column_count(stmt);

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            DbRow row;
            for (int i = 0; i < colCount; ++i) {
                std::string colName = sqlite3_column_name(stmt, i);
                int colType = sqlite3_column_type(stmt, i);
                switch (colType) {
                    case SQLITE_INTEGER:
                        row[colName] = static_cast<int64_t>(sqlite3_column_int64(stmt, i));
                        break;
                    case SQLITE_FLOAT:
                        row[colName] = sqlite3_column_double(stmt, i);
                        break;
                    case SQLITE_TEXT:
                        row[colName] = std::string(
                            reinterpret_cast<const char*>(sqlite3_column_text(stmt, i)));
                        break;
                    default:
                        row[colName] = nullptr;
                        break;
                }
            }
            rows.push_back(std::move(row));
        }
        sqlite3_finalize(stmt);
        return rows;
    }

    int64_t lastInsertId() const override {
        return sqlite3_last_insert_rowid(db_);
    }

private:
    sqlite3* db_ = nullptr;

    sqlite3_stmt* prepare(const std::string& sql) const {
        sqlite3_stmt* stmt = nullptr;
        int rc = sqlite3_prepare_v2(db_, sql.c_str(),
                                    static_cast<int>(sql.size()), &stmt, nullptr);
        if (rc != SQLITE_OK) {
            throw std::runtime_error(
                std::string("SQL prepare error: ") + sqlite3_errmsg(db_));
        }
        return stmt;
    }

    void bindParams(sqlite3_stmt* stmt,
                    const std::vector<DbValue>& params) const {
        for (size_t i = 0; i < params.size(); ++i) {
            int idx = static_cast<int>(i) + 1;
            const auto& val = params[i];

            if (std::holds_alternative<std::nullptr_t>(val)) {
                sqlite3_bind_null(stmt, idx);
            } else if (std::holds_alternative<int>(val)) {
                sqlite3_bind_int(stmt, idx, std::get<int>(val));
            } else if (std::holds_alternative<int64_t>(val)) {
                sqlite3_bind_int64(stmt, idx, std::get<int64_t>(val));
            } else if (std::holds_alternative<double>(val)) {
                sqlite3_bind_double(stmt, idx, std::get<double>(val));
            } else if (std::holds_alternative<std::string>(val)) {
                const auto& s = std::get<std::string>(val);
                sqlite3_bind_text(stmt, idx, s.c_str(),
                                  static_cast<int>(s.size()), SQLITE_TRANSIENT);
            }
        }
    }
};

} // namespace fastestapi
