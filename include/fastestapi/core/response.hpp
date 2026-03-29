#pragma once

#include <nlohmann/json.hpp>
#include <string>

namespace fastestapi {

class Response {
public:
    Response() = default;
    Response(int status, nlohmann::json body, std::string contentType = "application/json")
        : status_(status), body_(std::move(body)), contentType_(std::move(contentType)) {}

    int status() const { return status_; }
    const nlohmann::json& bodyJson() const { return body_; }
    std::string bodyString() const { return body_.dump(); }
    const std::string&  contentType() const { return contentType_; }

    static Response json(const nlohmann::json& data, int status = 200) {
        return {status, data};
    }
    static Response created(const nlohmann::json& data) {
        return {201, data};
    }
    static Response noContent() {
        return {204, nlohmann::json()};
    }
    static Response badRequest(const std::string& msg) {
        return {400, {{"error", msg}}};
    }
    static Response notFound(const std::string& msg = "Resource not found") {
        return {404, {{"error", msg}}};
    }
    static Response serverError(const std::string& msg = "Internal server error") {
        return {500, {{"error", msg}}};
    }

private:
    int            status_      = 200;
    nlohmann::json body_;
    std::string    contentType_ = "application/json";
};

} // namespace fastestapi