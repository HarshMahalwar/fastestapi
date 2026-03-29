#pragma once

#include <fastestapi/core/service_registry.hpp>
#include <fastestapi/core/types.hpp>
#include <nlohmann/json.hpp>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

namespace fastestapi {

template <typename T>
class Model {
public:
    virtual ~Model() = default;

    static void createTable() {
        auto& db = ServiceRegistry::instance().database();
        auto& qb = ServiceRegistry::instance().queryBuilder();
        std::string sql = qb.createTable(T::tableName(), T::fieldDefs());
        db.execute(sql);
    }

    static T create(const nlohmann::json& data) {
        auto& db = ServiceRegistry::instance().database();
        auto& qb = ServiceRegistry::instance().queryBuilder();

        T instance = T::fromJson(data);
        std::string sql = qb.insert(T::tableName(), instance.writableColumns());
        db.execute(sql, instance.writableValues());

        int64_t newId = db.lastInsertId();
        auto result = get(static_cast<int>(newId));
        if (!result) throw std::runtime_error("Failed to read back created row");
        return *result;
    }

    static std::optional<T> get(int id) {
        auto& db = ServiceRegistry::instance().database();
        auto& qb = ServiceRegistry::instance().queryBuilder();

        std::string sql = qb.selectById(T::tableName(), T::primaryKey());
        auto rows = db.query(sql, {id});

        if (rows.empty()) return std::nullopt;
        return T::fromRow(rows.front());
    }

    static std::vector<T> getAll() {
        auto& db = ServiceRegistry::instance().database();
        auto& qb = ServiceRegistry::instance().queryBuilder();

        std::string sql = qb.selectAll(T::tableName());
        auto rows = db.query(sql);

        std::vector<T> items;
        items.reserve(rows.size());
        for (auto& row : rows)
            items.push_back(T::fromRow(row));
        return items;
    }

    static std::optional<T> update(int id, const nlohmann::json& data) {
        auto existing = get(id);
        if (!existing) return std::nullopt;

        auto& db = ServiceRegistry::instance().database();
        auto& qb = ServiceRegistry::instance().queryBuilder();

        T instance = T::merge(*existing, data);

        auto cols = instance.writableColumns();
        auto vals = instance.writableValues();
        vals.push_back(id);

        std::string sql = qb.update(T::tableName(), cols, T::primaryKey());
        db.execute(sql, vals);

        return get(id);
    }

    static bool remove(int id) {
        auto existing = get(id);
        if (!existing) return false;

        auto& db = ServiceRegistry::instance().database();
        auto& qb = ServiceRegistry::instance().queryBuilder();

        std::string sql = qb.deleteById(T::tableName(), T::primaryKey());
        db.execute(sql, {id});
        return true;
    }
};

} // namespace fastestapi
