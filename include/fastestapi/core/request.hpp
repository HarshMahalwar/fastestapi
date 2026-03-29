#pragma once

#include <fastestapi/core/types.hpp>
#include <nlohmann/json.hpp>
#include <string>
#include <unordered_map>

namespace fastestapi {

class Request {
public:
    Request() = default;
    Request(HttpMethod method, std::string path, std::string body)
        : method_(method), path_(std::move(path)), body_(std::move(body)) {}

    HttpMethod  method() const { return method_; }
    const std::string& path()   const { return path_; }
    const std::string& body()   const { return body_; }

    void setParam(const std::string& key, const std::string& val) {
        params_[key] = val;
    }
    std::string param(const std::string& key) const {
        auto it = params_.find(key);
        return (it != params_.end()) ? it->second : "";
    }
    bool hasParam(const std::string& key) const {
        return params_.count(key) > 0;
    }

    nlohmann::json json() const {
        try { return nlohmann::json::parse(body_); }
        catch (...) { return nlohmann::json(); }
    }

private:
    HttpMethod   method_ = HttpMethod::UNKNOWN;
    std::string  path_;
    std::string  body_;
    std::unordered_map<std::string, std::string> params_;
};

} // namespace fastestapi
