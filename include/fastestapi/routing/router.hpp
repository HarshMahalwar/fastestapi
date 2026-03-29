#pragma once

#include <fastestapi/interfaces/i_router.hpp>
#include <iostream>
#include <memory>
#include <regex>
#include <string>
#include <vector>

namespace fastestapi {

class Router : public IRouter {
public:
    void addEndpoint(std::shared_ptr<IEndpoint> ep) override {
        RouteEntry entry;
        entry.endpoint = std::move(ep);
        entry.method   = entry.endpoint->method();

        std::string pattern = entry.endpoint->path();
        std::regex paramRe(R"(\{(\w+)\})");
        std::sregex_iterator it(pattern.begin(), pattern.end(), paramRe);
        std::sregex_iterator end;

        for (; it != end; ++it)
            entry.paramNames.push_back((*it)[1].str());

        std::string regexStr = "^" +
            std::regex_replace(pattern, paramRe, "([^/]+)") + "$";
        entry.pathRegex = std::regex(regexStr);

        std::cout << "[FastestAPI] Route registered: "
                  << methodToString(entry.method)
                  << " " << entry.endpoint->path() << "\n";

        routes_.push_back(std::move(entry));
    }

    Response dispatch(const Request& req) const override {
        for (auto& route : routes_) {
            if (route.method != req.method()) continue;

            std::smatch match;
            std::string p = req.path();
            if (!std::regex_match(p, match, route.pathRegex)) continue;

            Request enriched = req;
            for (size_t i = 0; i < route.paramNames.size(); ++i)
                enriched.setParam(route.paramNames[i], match[i + 1].str());

            try {
                return route.endpoint->handle(enriched);
            } catch (const std::exception& e) {
                return Response::serverError(e.what());
            }
        }
        return Response::notFound("No route matches "
                                  + methodToString(req.method())
                                  + " " + req.path());
    }

    std::vector<std::shared_ptr<IEndpoint>> endpoints() const override {
        std::vector<std::shared_ptr<IEndpoint>> eps;
        eps.reserve(routes_.size());
        for (auto& r : routes_)
            eps.push_back(r.endpoint);
        return eps;
    }

private:
    struct RouteEntry {
        std::shared_ptr<IEndpoint>   endpoint;
        HttpMethod                   method;
        std::regex                   pathRegex;
        std::vector<std::string>     paramNames;
    };
    std::vector<RouteEntry> routes_;
};

} // namespace fastestapi
