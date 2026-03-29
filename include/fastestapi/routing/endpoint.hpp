#pragma once

#include <fastestapi/interfaces/i_endpoint.hpp>
#include <string>

namespace fastestapi {

class GetEndpoint : public IEndpoint {
public:
    explicit GetEndpoint(std::string path) : path_(std::move(path)) {}
    HttpMethod  method() const override { return HttpMethod::GET; }
    std::string path()   const override { return path_; }
private:
    std::string path_;
};

class PostEndpoint : public IEndpoint {
public:
    explicit PostEndpoint(std::string path) : path_(std::move(path)) {}
    HttpMethod  method() const override { return HttpMethod::POST; }
    std::string path()   const override { return path_; }
private:
    std::string path_;
};

class PutEndpoint : public IEndpoint {
public:
    explicit PutEndpoint(std::string path) : path_(std::move(path)) {}
    HttpMethod  method() const override { return HttpMethod::PUT; }
    std::string path()   const override { return path_; }
private:
    std::string path_;
};

class DeleteEndpoint : public IEndpoint {
public:
    explicit DeleteEndpoint(std::string path) : path_(std::move(path)) {}
    HttpMethod  method() const override { return HttpMethod::DELETE_; }
    std::string path()   const override { return path_; }
private:
    std::string path_;
};

} // namespace fastestapi
