#pragma once

#include <fastestapi/core/types.hpp>
#include <string>
#include <vector>

namespace fastestapi {

class IQueryBuilder {
public:
    virtual ~IQueryBuilder() = default;

    virtual std::string createTable(
        const std::string& table,
        const std::vector<FieldDef>& fields) const = 0;

    virtual std::string insert(
        const std::string& table,
        const std::vector<std::string>& columns) const = 0;

    virtual std::string selectAll(const std::string& table) const = 0;

    virtual std::string selectById(
        const std::string& table,
        const std::string& pkColumn) const = 0;

    virtual std::string update(
        const std::string& table,
        const std::vector<std::string>& columns,
        const std::string& pkColumn) const = 0;

    virtual std::string deleteById(
        const std::string& table,
        const std::string& pkColumn) const = 0;
};

} // namespace fastestapi
