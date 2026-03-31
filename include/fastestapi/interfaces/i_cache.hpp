#pragma once

#include <optional>
#include <string>

namespace fastestapi {

class ICache {
public:
    virtual ~ICache() = default;

    virtual bool connect(const std::string& host, int port) = 0;
    virtual void disconnect() = 0;
    virtual bool isConnected() const = 0;

    virtual bool authenticate(const std::string& password) = 0;

    virtual bool set(const std::string& key,
                     const std::string& value,
                     int ttlSeconds = 0) = 0;

    virtual std::optional<std::string> get(const std::string& key) = 0;
    virtual bool del(const std::string& key) = 0;
    virtual bool exists(const std::string& key) = 0;
};

} // namespace fastestapi