#pragma once

#include <fastestapi/interfaces/i_endpoint.hpp>
#include <fastestapi/core/request.hpp>
#include <fastestapi/core/response.hpp>
#include <memory>
#include <vector>

namespace fastestapi {

class IRouter {
public:
    virtual ~IRouter() = default;

    virtual void     addEndpoint(std::shared_ptr<IEndpoint> endpoint) = 0;
    virtual Response dispatch(const Request& req) const = 0;

    // Expose registered endpoints for OpenAPI generation
    virtual std::vector<std::shared_ptr<IEndpoint>> endpoints() const = 0;
};

} // namespace fastestapi
