#pragma once

#include <fastestapi/core/types.hpp>
#include <string>
#include <vector>

namespace fastestapi {

class IDatabase {
public:
    virtual ~IDatabase() = default;

    virtual bool connect(const std::string& connectionString) = 0;
    virtual void disconnect() = 0;
    virtual bool isConnected() const = 0;

    virtual int execute(const std::string& sql,
                        const std::vector<DbValue>& params = {}) = 0;

    virtual std::vector<DbRow> query(const std::string& sql,
                                     const std::vector<DbValue>& params = {}) = 0;

    virtual int64_t lastInsertId() const = 0;
};

} // namespace fastestapi
