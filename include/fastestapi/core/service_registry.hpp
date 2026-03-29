#pragma once

#include <fastestapi/interfaces/i_database.hpp>
#include <fastestapi/interfaces/i_query_builder.hpp>
#include <memory>
#include <stdexcept>

namespace fastestapi {

class ServiceRegistry {
public:
    static ServiceRegistry& instance() {
        static ServiceRegistry reg;
        return reg;
    }

    void setDatabase(std::shared_ptr<IDatabase> db) { db_ = std::move(db); }
    void setQueryBuilder(std::shared_ptr<IQueryBuilder> qb) { qb_ = std::move(qb); }

    IDatabase& database() const {
        if (!db_) throw std::runtime_error("Database not registered");
        return *db_;
    }
    IQueryBuilder& queryBuilder() const {
        if (!qb_) throw std::runtime_error("QueryBuilder not registered");
        return *qb_;
    }

private:
    ServiceRegistry() = default;
    std::shared_ptr<IDatabase>     db_;
    std::shared_ptr<IQueryBuilder> qb_;
};

} // namespace fastestapi
