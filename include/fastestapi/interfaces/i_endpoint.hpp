#pragma once

#include <fastestapi/core/request.hpp>
#include <fastestapi/core/response.hpp>
#include <fastestapi/core/types.hpp>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace fastestapi {

class IEndpoint {
public:
    virtual ~IEndpoint() = default;

    // Required
    virtual HttpMethod    method()  const = 0;
    virtual std::string   path()    const = 0;
    virtual Response      handle(const Request& req) = 0;

    // Optional OpenAPI metadata
    virtual std::string              summary()        const { return ""; }
    virtual std::string              description()    const { return ""; }
    virtual std::vector<std::string> tags()           const { return {}; }
    virtual nlohmann::json           requestSchema()  const { return nullptr; }
    virtual nlohmann::json           responseSchema() const { return nullptr; }
};

} // namespace fastestapi
