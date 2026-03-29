#pragma once

#include <fastestapi/interfaces/i_query_builder.hpp>
#include <sstream>

namespace fastestapi {

class SqliteQueryBuilder : public IQueryBuilder {
public:
    std::string createTable(
            const std::string& table,
            const std::vector<FieldDef>& fields) const override {
        std::ostringstream ss;
        ss << "CREATE TABLE IF NOT EXISTS " << table << " (";
        for (size_t i = 0; i < fields.size(); ++i) {
            if (i > 0) ss << ", ";
            ss << fields[i].name << " " << fieldTypeSql(fields[i].type);
            for (auto& c : fields[i].constraints)
                ss << " " << constraintSql(c);
        }
        ss << ");";
        return ss.str();
    }

    std::string insert(
            const std::string& table,
            const std::vector<std::string>& columns) const override {
        std::ostringstream ss;
        ss << "INSERT INTO " << table << " (";
        for (size_t i = 0; i < columns.size(); ++i) {
            if (i > 0) ss << ", ";
            ss << columns[i];
        }
        ss << ") VALUES (";
        for (size_t i = 0; i < columns.size(); ++i) {
            if (i > 0) ss << ", ";
            ss << "?";
        }
        ss << ");";
        return ss.str();
    }

    std::string selectAll(const std::string& table) const override {
        return "SELECT * FROM " + table + ";";
    }

    std::string selectById(
            const std::string& table,
            const std::string& pkColumn) const override {
        return "SELECT * FROM " + table +
               " WHERE " + pkColumn + " = ?;";
    }

    std::string update(
            const std::string& table,
            const std::vector<std::string>& columns,
            const std::string& pkColumn) const override {
        std::ostringstream ss;
        ss << "UPDATE " << table << " SET ";
        for (size_t i = 0; i < columns.size(); ++i) {
            if (i > 0) ss << ", ";
            ss << columns[i] << " = ?";
        }
        ss << " WHERE " << pkColumn << " = ?;";
        return ss.str();
    }

    std::string deleteById(
            const std::string& table,
            const std::string& pkColumn) const override {
        return "DELETE FROM " + table +
               " WHERE " + pkColumn + " = ?;";
    }

private:
    static std::string fieldTypeSql(FieldType ft) {
        switch (ft) {
            case FieldType::Integer: return "INTEGER";
            case FieldType::Real:    return "REAL";
            case FieldType::Text:    return "TEXT";
        }
        return "TEXT";
    }

    static std::string constraintSql(FieldConstraint c) {
        switch (c) {
            case FieldConstraint::PrimaryKey:    return "PRIMARY KEY";
            case FieldConstraint::AutoIncrement: return "AUTOINCREMENT";
            case FieldConstraint::NotNull:       return "NOT NULL";
            case FieldConstraint::None:          return "";
        }
        return "";
    }
};

} // namespace fastestapi
