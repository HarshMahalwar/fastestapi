#pragma once

#include <fastestapi/interfaces/i_cache.hpp>
#include <Poco/Redis/Client.h>
#include <Poco/Redis/Command.h>
#include <iostream>
#include <memory>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <string>

namespace fastestapi {

class RedisCache : public ICache {
public:
    ~RedisCache() override { disconnect(); }

    bool connect(const std::string& host, int port) override {
        std::lock_guard<std::mutex> lock(mutex_);
        if (client_) disconnect_internal();
        try {
            client_ = std::make_unique<Poco::Redis::Client>(host, port);
            std::cout << "[FastestAPI] Connected to Redis: "
                      << host << ":" << port << "\n";
            return true;
        } catch (const Poco::Exception& e) {
            client_.reset();
            throw std::runtime_error("Redis connect failed: "
                                     + e.displayText());
        }
    }

    void disconnect() override {
        std::lock_guard<std::mutex> lock(mutex_);
        disconnect_internal();
    }

    bool isConnected() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return client_ != nullptr;
    }

    bool authenticate(const std::string& password) override {
        std::lock_guard<std::mutex> lock(mutex_);
        ensureConnected();
        try {
            Poco::Redis::Command cmd("AUTH");
            cmd << password;
            std::string reply = client_->execute<std::string>(cmd);
            return reply == "OK";
        } catch (const Poco::Exception& e) {
            throw std::runtime_error("Redis AUTH failed: "
                                     + e.displayText());
        }
    }

    bool set(const std::string& key, const std::string& value,
             int ttlSeconds) override {
        std::lock_guard<std::mutex> lock(mutex_);
        ensureConnected();
        try {
            Poco::Redis::Command cmd("SET");
            cmd << key << value;
            if (ttlSeconds > 0)
                cmd << "EX" << std::to_string(ttlSeconds);
            std::string reply = client_->execute<std::string>(cmd);
            return reply == "OK";
        } catch (const Poco::Exception& e) {
            throw std::runtime_error("Redis SET failed: "
                                     + e.displayText());
        }
    }

    std::optional<std::string> get(const std::string& key) override {
        std::lock_guard<std::mutex> lock(mutex_);
        ensureConnected();
        try {
            Poco::Redis::Command cmd("GET");
            cmd << key;
            Poco::Redis::BulkString reply =
                client_->execute<Poco::Redis::BulkString>(cmd);
            if (reply.isNull()) return std::nullopt;
            return reply.value();
        } catch (const Poco::Exception& e) {
            throw std::runtime_error("Redis GET failed: "
                                     + e.displayText());
        }
    }

    bool del(const std::string& key) override {
        std::lock_guard<std::mutex> lock(mutex_);
        ensureConnected();
        try {
            Poco::Redis::Command cmd("DEL");
            cmd << key;
            Poco::Int64 removed = client_->execute<Poco::Int64>(cmd);
            return removed > 0;
        } catch (const Poco::Exception& e) {
            throw std::runtime_error("Redis DEL failed: "
                                     + e.displayText());
        }
    }

    bool exists(const std::string& key) override {
        std::lock_guard<std::mutex> lock(mutex_);
        ensureConnected();
        try {
            Poco::Redis::Command cmd("EXISTS");
            cmd << key;
            Poco::Int64 count = client_->execute<Poco::Int64>(cmd);
            return count > 0;
        } catch (const Poco::Exception& e) {
            throw std::runtime_error("Redis EXISTS failed: "
                                     + e.displayText());
        }
    }

private:
    std::unique_ptr<Poco::Redis::Client> client_;
    mutable std::mutex                   mutex_;

    void disconnect_internal() {
        if (client_) {
            client_->disconnect();
            client_.reset();
        }
    }

    void ensureConnected() const {
        if (!client_)
            throw std::runtime_error("Redis client is not connected");
    }
};

} // namespace fastestapi