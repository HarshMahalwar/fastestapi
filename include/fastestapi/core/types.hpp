#pragma once

#include <nlohmann/json.hpp>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace fastestapi {

using DbValue = std::variant<std::nullptr_t, int, int64_t, double, std::string>;
using DbRow = std::unordered_map<std::string, DbValue>;

struct CacheConfig {
    std::string host       = "127.0.0.1";
    int         port       = 6379;
    std::string password;                  // empty = no AUTH
    int         defaultTtl = 0;            // 0 = no expiry
};

enum class FieldType { Integer, Real, Text };

enum class FieldConstraint { None, PrimaryKey, AutoIncrement, NotNull };

struct FieldDef {
    std::string              name;
    FieldType                type;
    std::vector<FieldConstraint> constraints;

    bool hasPrimaryKey() const {
        for (auto& c : constraints)
            if (c == FieldConstraint::PrimaryKey) return true;
        return false;
    }
    bool hasAutoIncrement() const {
        for (auto& c : constraints)
            if (c == FieldConstraint::AutoIncrement) return true;
        return false;
    }
};

enum class HttpMethod { GET, POST, PUT, DELETE_, UNKNOWN };

inline HttpMethod methodFromString(const std::string& m) {
    if (m == "GET")    return HttpMethod::GET;
    if (m == "POST")   return HttpMethod::POST;
    if (m == "PUT")    return HttpMethod::PUT;
    if (m == "DELETE") return HttpMethod::DELETE_;
    return HttpMethod::UNKNOWN;
}

inline std::string methodToString(HttpMethod m) {
    switch (m) {
        case HttpMethod::GET:     return "GET";
        case HttpMethod::POST:    return "POST";
        case HttpMethod::PUT:     return "PUT";
        case HttpMethod::DELETE_: return "DELETE";
        default:                  return "UNKNOWN";
    }
}

} // namespace fastestapi
