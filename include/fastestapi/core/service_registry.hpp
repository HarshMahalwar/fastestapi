#pragma once

#include <fastestapi/interfaces/i_cache.hpp>
#include <fastestapi/interfaces/i_database.hpp>
#include <fastestapi/interfaces/i_query_builder.hpp>
#include <memory>
#include <mutex>
#include <stdexcept>

namespace fastestapi {

class ServiceRegistry {
public:
    static ServiceRegistry& instance() {
        static ServiceRegistry reg;
        return reg;
    }

    void setDatabase(std::shared_ptr<IDatabase> db) {
        std::lock_guard<std::mutex> lock(mutex_);
        db_ = std::move(db);
    }
    IDatabase& database() const {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!db_) throw std::runtime_error("No database registered");
        return *db_;
    }

    void setQueryBuilder(std::shared_ptr<IQueryBuilder> qb) {
        std::lock_guard<std::mutex> lock(mutex_);
        qb_ = std::move(qb);
    }
    IQueryBuilder& queryBuilder() const {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!qb_) throw std::runtime_error("No query builder registered");
        return *qb_;
    }

    void setCache(std::shared_ptr<ICache> cache) {
        std::lock_guard<std::mutex> lock(mutex_);
        cache_ = std::move(cache);
    }
    bool hasCache() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return cache_ != nullptr;
    }
    ICache& cache() const {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!cache_) throw std::runtime_error("No cache registered");
        return *cache_;
    }

private:
    ServiceRegistry() = default;
    ServiceRegistry(const ServiceRegistry&) = delete;
    ServiceRegistry& operator=(const ServiceRegistry&) = delete;

    std::shared_ptr<IDatabase>     db_;
    std::shared_ptr<IQueryBuilder> qb_;
    std::shared_ptr<ICache>        cache_;
    mutable std::mutex             mutex_;
};

} // namespace fastestapi
