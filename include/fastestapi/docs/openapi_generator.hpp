#pragma once

#include <fastestapi/core/types.hpp>
#include <fastestapi/interfaces/i_endpoint.hpp>
#include <nlohmann/json.hpp>
#include <regex>
#include <string>
#include <vector>
#include <memory>
#include <algorithm>

namespace fastestapi {

// Application metadata for the OpenAPI info block
struct AppInfo {
    std::string title       = "FastestAPI";
    std::string version     = "1.0.0";
    std::string description = "Auto-generated API documentation";
};

class OpenApiGenerator {
public:
    explicit OpenApiGenerator(AppInfo info = {}) : info_(std::move(info)) {}

    /**
     * Build a complete OpenAPI 3.0.3 JSON spec from registered endpoints.
     */
    nlohmann::json generate(
        const std::vector<std::shared_ptr<IEndpoint>>& endpoints,
        const std::string& host = "localhost",
        int port = 8080) const
    {
        using json = nlohmann::json;

        json spec = {
            {"openapi", "3.0.3"},
            {"info", {
                {"title",       info_.title},
                {"version",     info_.version},
                {"description", info_.description}
            }},
            {"servers", json::array({
                {{"url", "http://" + host + ":" + std::to_string(port)}}
            })},
            {"paths",      json::object()},
            {"components", {{"schemas", json::object()}}}
        };

        // Error schema shared by all error responses
        spec["components"]["schemas"]["Error"] = {
            {"type", "object"},
            {"properties", {
                {"error", {{"type", "string"}}}
            }}
        };

        for (auto& ep : endpoints) {
            std::string path      = ep->path();
            std::string methodStr = toLower(methodToString(ep->method()));
            auto        paramNames = extractPathParams(path);

            json operation = json::object();

            // Summary & description
            std::string summary = ep->summary();
            if (summary.empty())
                summary = defaultSummary(ep->method(), path);
            operation["summary"] = summary;

            if (!ep->description().empty())
                operation["description"] = ep->description();

            // Tags
            auto tags = ep->tags();
            if (tags.empty())
                tags = {defaultTag(path)};
            operation["tags"] = tags;

            // Operation ID
            operation["operationId"] = operationId(ep->method(), path);

            // Path parameters
            if (!paramNames.empty()) {
                json params = json::array();
                for (auto& pn : paramNames) {
                    params.push_back({
                        {"name",     pn},
                        {"in",       "path"},
                        {"required", true},
                        {"schema",   {{"type", inferParamType(pn)}}}
                    });
                }
                operation["parameters"] = params;
            }

            // Request body (POST / PUT)
            json reqSchema = ep->requestSchema();
            if (!reqSchema.is_null()) {
                operation["requestBody"] = {
                    {"required", true},
                    {"content", {
                        {"application/json", {
                            {"schema", reqSchema}
                        }}
                    }}
                };
            }

            // Responses
            operation["responses"] = buildResponses(
                ep->method(), ep->responseSchema());

            spec["paths"][path][methodStr] = operation;
        }

        return spec;
    }

private:
    AppInfo info_;

    // Helpers
    static std::vector<std::string> extractPathParams(const std::string& path) {
        std::vector<std::string> params;
        std::regex re(R"(\{(\w+)\})");
        std::sregex_iterator it(path.begin(), path.end(), re);
        std::sregex_iterator end;
        for (; it != end; ++it)
            params.push_back((*it)[1].str());
        return params;
    }

    static std::string toLower(std::string s) {
        std::transform(s.begin(), s.end(), s.begin(), ::tolower);
        return s;
    }

    /// Infer OpenAPI type for common parameter names
    static std::string inferParamType(const std::string& name) {
        auto endsWith = [](const std::string& str, const std::string& suffix) -> bool {
            if (suffix.size() > str.size()) return false;
            return str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
        };

        if (name == "id" || endsWith(name, "_id") || endsWith(name, "Id"))
            return "integer";
        return "string";
    }


    /// First non-empty path segment -> tag  (e.g. "/items/{id}" -> "items")
    static std::string defaultTag(const std::string& path) {
        auto pos = path.find('/', 1);
        std::string segment = (pos != std::string::npos)
                              ? path.substr(1, pos - 1)
                              : path.substr(1);
        if (segment.empty()) segment = "default";
        return segment;
    }

    /// e.g. GET /items/{id} -> "getItemsById"
    static std::string operationId(HttpMethod method, const std::string& path) {
        std::string id = toLower(methodToString(method));
        std::string cleaned = path;

        // Remove braces
        cleaned.erase(
            std::remove_if(cleaned.begin(), cleaned.end(),
                           [](char c) { return c == '{' || c == '}'; }),
            cleaned.end());

        // Split by '/' and CamelCase each segment
        std::istringstream ss(cleaned);
        std::string segment;
        while (std::getline(ss, segment, '/')) {
            if (segment.empty()) continue;
            segment[0] = static_cast<char>(std::toupper(segment[0]));
            id += segment;
        }
        return id;
    }

    static std::string defaultSummary(HttpMethod method, const std::string& path) {
        std::string tag = defaultTag(path);
        switch (method) {
            case HttpMethod::GET:
                return extractPathParams(path).empty()
                       ? "List all " + tag
                       : "Get a " + tag.substr(0, tag.size() > 1 ? tag.size() - 1 : tag.size()) + " by ID";
            case HttpMethod::POST:
                return "Create a new " + tag.substr(0, tag.size() > 1 ? tag.size() - 1 : tag.size());
            case HttpMethod::PUT:
                return "Update a " + tag.substr(0, tag.size() > 1 ? tag.size() - 1 : tag.size());
            case HttpMethod::DELETE_:
                return "Delete a " + tag.substr(0, tag.size() > 1 ? tag.size() - 1 : tag.size());
            default:
                return methodToString(method) + " " + path;
        }
    }

    static nlohmann::json buildResponses(HttpMethod method,
                                         const nlohmann::json& responseSchema) {
        using json = nlohmann::json;
        json responses = json::object();

        switch (method) {
            case HttpMethod::GET: {
                json ok = {{"description", "Successful response"}};
                if (!responseSchema.is_null()) {
                    ok["content"] = {{"application/json", {{"schema", responseSchema}}}};
                }
                responses["200"] = ok;
                // GET with path params can 404
                responses["404"] = {
                    {"description", "Resource not found"},
                    {"content", {{"application/json", {{"schema", {{"$ref", "#/components/schemas/Error"}}}}}}}
                };
                break;
            }
            case HttpMethod::POST: {
                json created = {{"description", "Resource created"}};
                if (!responseSchema.is_null()) {
                    created["content"] = {{"application/json", {{"schema", responseSchema}}}};
                }
                responses["201"] = created;
                responses["400"] = {
                    {"description", "Invalid input"},
                    {"content", {{"application/json", {{"schema", {{"$ref", "#/components/schemas/Error"}}}}}}}
                };
                break;
            }
            case HttpMethod::PUT: {
                json ok = {{"description", "Resource updated"}};
                if (!responseSchema.is_null()) {
                    ok["content"] = {{"application/json", {{"schema", responseSchema}}}};
                }
                responses["200"] = ok;
                responses["400"] = {
                    {"description", "Invalid input"},
                    {"content", {{"application/json", {{"schema", {{"$ref", "#/components/schemas/Error"}}}}}}}
                };
                responses["404"] = {
                    {"description", "Resource not found"},
                    {"content", {{"application/json", {{"schema", {{"$ref", "#/components/schemas/Error"}}}}}}}
                };
                break;
            }
            case HttpMethod::DELETE_: {
                responses["204"] = {{"description", "Resource deleted"}};
                responses["404"] = {
                    {"description", "Resource not found"},
                    {"content", {{"application/json", {{"schema", {{"$ref", "#/components/schemas/Error"}}}}}}}
                };
                break;
            }
            default:
                responses["200"] = {{"description", "OK"}};
                break;
        }

        // All endpoints can 500
        responses["500"] = {
            {"description", "Internal server error"},
            {"content", {{"application/json", {{"schema", {{"$ref", "#/components/schemas/Error"}}}}}}}
        };

        return responses;
    }
};

} // namespace fastestapi