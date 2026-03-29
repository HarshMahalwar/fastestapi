#pragma once

#include <fastestapi/core/service_registry.hpp>
#include <fastestapi/core/types.hpp>
#include <fastestapi/core/request.hpp>
#include <fastestapi/core/response.hpp>
#include <fastestapi/db/sqlite_database.hpp>
#include <fastestapi/db/sqlite_query_builder.hpp>
#include <fastestapi/docs/openapi_generator.hpp>
#include <fastestapi/docs/swagger_ui.hpp>
#include <fastestapi/interfaces/i_endpoint.hpp>
#include <fastestapi/routing/router.hpp>
#include <httplib.h>
#include <iostream>
#include <memory>
#include <string>

namespace fastestapi {

class Application {
public:
    explicit Application(const std::string& dbPath = "fastestapi.db",
                         AppInfo info = {})
        : appInfo_(std::move(info))
    {
        db_ = std::make_shared<SqliteDatabase>();
        db_->connect(dbPath);

        qb_ = std::make_shared<SqliteQueryBuilder>();
        router_ = std::make_shared<Router>();

        ServiceRegistry::instance().setDatabase(db_);
        ServiceRegistry::instance().setQueryBuilder(qb_);
    }

    template <typename ModelType>
    Application& model() {
        ModelType::createTable();
        std::cout << "[FastestAPI] Table ready: "
                  << ModelType::tableName() << "\n";
        return *this;
    }

    Application& endpoint(std::shared_ptr<IEndpoint> ep) {
        router_->addEndpoint(std::move(ep));
        return *this;
    }

    template <typename EndpointType, typename... Args>
    Application& route(Args&&... args) {
        router_->addEndpoint(
            std::make_shared<EndpointType>(std::forward<Args>(args)...));
        return *this;
    }

    /// Disable auto-generated docs (enabled by default)
    Application& disableDocs() { docsEnabled_ = false; return *this; }

    void listen(const std::string& host, int port) {
        httplib::Server svr;

        // Documentation endpoints (/docs and /openapi.json)
        if (docsEnabled_) {
            // Pre-generate the spec so it captures all registered routes
            OpenApiGenerator gen(appInfo_);
            std::string specJson = gen.generate(
                router_->endpoints(), host, port).dump(2);

            std::string uiHtml = swaggerUiHtml("/openapi.json", appInfo_.title + " — Docs");

            svr.Get("/openapi.json",
                [specJson](const httplib::Request&, httplib::Response& res) {
                    res.set_content(specJson, "application/json");
                });

            svr.Get("/docs",
                [uiHtml](const httplib::Request&, httplib::Response& res) {
                    res.set_content(uiHtml, "text/html");
                });

            std::cout << "[FastestAPI] Docs available at http://"
                      << host << ":" << port << "/docs\n";
            std::cout << "[FastestAPI] OpenAPI spec at  http://"
                      << host << ":" << port << "/openapi.json\n";
        }

        // Catch-all handlers that delegate to our Router
        auto handler = [this](const httplib::Request& httpReq,
                              httplib::Response& httpRes,
                              HttpMethod method) {
            Request req(method, httpReq.path, httpReq.body);
            Response res = router_->dispatch(req);

            httpRes.status = res.status();
            httpRes.set_content(res.bodyString(), res.contentType());
        };

        svr.Get(".*",    [&](const httplib::Request& r, httplib::Response& s) {
            handler(r, s, HttpMethod::GET);
        });
        svr.Post(".*",   [&](const httplib::Request& r, httplib::Response& s) {
            handler(r, s, HttpMethod::POST);
        });
        svr.Put(".*",    [&](const httplib::Request& r, httplib::Response& s) {
            handler(r, s, HttpMethod::PUT);
        });
        svr.Delete(".*", [&](const httplib::Request& r, httplib::Response& s) {
            handler(r, s, HttpMethod::DELETE_);
        });

        std::cout << "\n[FastestAPI] Listening on http://"
                  << host << ":" << port << "\n\n";
        svr.listen(host, port);
    }

private:
    std::shared_ptr<IDatabase>     db_;
    std::shared_ptr<IQueryBuilder> qb_;
    std::shared_ptr<Router>        router_;
    AppInfo                        appInfo_;
    bool                           docsEnabled_ = true;
};

} // namespace fastestapi
