#include <fastestapi/application.hpp>
#include <fastestapi/orm/model.hpp>
#include <fastestapi/routing/endpoint.hpp>

using namespace fastestapi;
using json = nlohmann::json;

namespace ItemSchema {
    inline json item() {
        return {
            {"type", "object"},
            {"properties", {
                {"id",    {{"type", "integer"}}},
                {"name",  {{"type", "string"}}},
                {"price", {{"type", "number"}}}
            }}
        };
    }
    inline json itemArray() {
        return {{"type", "array"}, {"items", item()}};
    }
    inline json createRequest() {
        return {
            {"type", "object"},
            {"required", json::array({"name", "price"})},
            {"properties", {
                {"name",  {{"type", "string"},  {"example", "Widget"}}},
                {"price", {{"type", "number"},  {"example", 9.99}}}
            }}
        };
    }
    inline json updateRequest() {
        return {
            {"type", "object"},
            {"properties", {
                {"name",  {{"type", "string"}}},
                {"price", {{"type", "number"}}}
            }}
        };
    }
}

struct Item : public Model<Item> {
    int         id    = 0;
    std::string name;
    double      price = 0.0;

    static std::string tableName()  { return "items"; }
    static std::string primaryKey() { return "id"; }

    static std::vector<FieldDef> fieldDefs() {
        return {
            {"id",    FieldType::Integer, {FieldConstraint::PrimaryKey,
                                           FieldConstraint::AutoIncrement}},
            {"name",  FieldType::Text,    {FieldConstraint::NotNull}},
            {"price", FieldType::Real,    {FieldConstraint::NotNull}},
        };
    }

    json toJson() const {
        return {{"id", id}, {"name", name}, {"price", price}};
    }
    static Item fromJson(const json& j) {
        Item it;
        if (j.contains("name"))  it.name  = j.at("name").get<std::string>();
        if (j.contains("price")) it.price = j.at("price").get<double>();
        return it;
    }
    static Item fromRow(const DbRow& row) {
        Item it;
        auto getInt = [&](const std::string& col) -> int {
            auto iter = row.find(col);
            if (iter == row.end()) return 0;
            if (std::holds_alternative<int64_t>(iter->second))
                return static_cast<int>(std::get<int64_t>(iter->second));
            if (std::holds_alternative<int>(iter->second))
                return std::get<int>(iter->second);
            return 0;
        };
        auto getDouble = [&](const std::string& col) -> double {
            auto iter = row.find(col);
            if (iter == row.end()) return 0.0;
            if (std::holds_alternative<double>(iter->second))
                return std::get<double>(iter->second);
            if (std::holds_alternative<int64_t>(iter->second))
                return static_cast<double>(std::get<int64_t>(iter->second));
            return 0.0;
        };
        auto getString = [&](const std::string& col) -> std::string {
            auto iter = row.find(col);
            if (iter == row.end()) return "";
            if (std::holds_alternative<std::string>(iter->second))
                return std::get<std::string>(iter->second);
            return "";
        };
        it.id    = getInt("id");
        it.name  = getString("name");
        it.price = getDouble("price");
        return it;
    }
    static Item merge(const Item& existing, const json& j) {
        Item it = existing;
        if (j.contains("name"))  it.name  = j.at("name").get<std::string>();
        if (j.contains("price")) it.price = j.at("price").get<double>();
        return it;
    }
    std::vector<std::string> writableColumns() const { return {"name", "price"}; }
    std::vector<DbValue>     writableValues()  const { return {name, price}; }
};

static std::string itemCacheKey(int id) {
    return "item:" + std::to_string(id);
}

class ListItems : public GetEndpoint {
public:
    ListItems() : GetEndpoint("/items") {}

    std::string              summary()        const override { return "List all items"; }
    std::vector<std::string> tags()           const override { return {"Items"}; }
    json                     responseSchema() const override { return ItemSchema::itemArray(); }

    Response handle(const Request&) override {
        auto items = Item::getAll();
        json arr = json::array();
        for (auto& item : items) arr.push_back(item.toJson());
        return Response::json(arr);
    }
};

class GetItem : public GetEndpoint {
public:
    GetItem() : GetEndpoint("/items/{id}") {}

    std::string              summary()        const override { return "Get an item by ID"; }
    std::vector<std::string> tags()           const override { return {"Items"}; }
    json                     responseSchema() const override { return ItemSchema::item(); }

    Response handle(const Request& req) override {
        int id = 0;
        try { id = std::stoi(req.param("id")); }
        catch (...) { return Response::badRequest("Parameter 'id' must be an integer"); }

        // Try cache first
        auto& registry = ServiceRegistry::instance();
        if (registry.hasCache()) {
            auto cached = registry.cache().get(itemCacheKey(id));
            if (cached) return Response::json(json::parse(*cached));
        }

        auto item = Item::get(id);
        if (!item) return Response::notFound("Item not found");

        // Populate cache
        if (registry.hasCache())
            registry.cache().set(itemCacheKey(id), item->toJson().dump(), 60);

        return Response::json(item->toJson());
    }
};

class CreateItem : public PostEndpoint {
public:
    CreateItem() : PostEndpoint("/items") {}

    std::string              summary()        const override { return "Create a new item"; }
    std::vector<std::string> tags()           const override { return {"Items"}; }
    json                     requestSchema()  const override { return ItemSchema::createRequest(); }
    json                     responseSchema() const override { return ItemSchema::item(); }

    Response handle(const Request& req) override {
        auto body = req.json();
        if (body.is_null() || body.empty())
            return Response::badRequest("Request body must be valid JSON");
        if (!body.contains("name") || !body["name"].is_string())
            return Response::badRequest("Field 'name' (string) is required");
        if (!body.contains("price") || !body["price"].is_number())
            return Response::badRequest("Field 'price' (number) is required");

        try {
            auto created = Item::create(body);

            auto& registry = ServiceRegistry::instance();
            if (registry.hasCache())
                registry.cache().set(itemCacheKey(created.id),
                                     created.toJson().dump(), 60);

            return Response::created(created.toJson());
        } catch (const std::exception& e) {
            return Response::serverError(e.what());
        }
    }
};

class UpdateItem : public PutEndpoint {
public:
    UpdateItem() : PutEndpoint("/items/{id}") {}

    std::string              summary()        const override { return "Update an existing item"; }
    std::vector<std::string> tags()           const override { return {"Items"}; }
    json                     requestSchema()  const override { return ItemSchema::updateRequest(); }
    json                     responseSchema() const override { return ItemSchema::item(); }

    Response handle(const Request& req) override {
        int id = 0;
        try { id = std::stoi(req.param("id")); }
        catch (...) { return Response::badRequest("Parameter 'id' must be an integer"); }

        auto body = req.json();
        if (body.is_null() || body.empty())
            return Response::badRequest("Request body must be valid JSON");
        if (body.contains("name") && !body["name"].is_string())
            return Response::badRequest("Field 'name' must be a string");
        if (body.contains("price") && !body["price"].is_number())
            return Response::badRequest("Field 'price' must be a number");

        try {
            auto updated = Item::update(id, body);
            if (!updated) return Response::notFound("Item not found");

            auto& registry = ServiceRegistry::instance();
            if (registry.hasCache())
                registry.cache().set(itemCacheKey(id),
                                     updated->toJson().dump(), 60);

            return Response::json(updated->toJson());
        } catch (const std::exception& e) {
            return Response::serverError(e.what());
        }
    }
};

class DeleteItem : public DeleteEndpoint {
public:
    DeleteItem() : DeleteEndpoint("/items/{id}") {}

    std::string              summary()  const override { return "Delete an item"; }
    std::vector<std::string> tags()     const override { return {"Items"}; }

    Response handle(const Request& req) override {
        int id = 0;
        try { id = std::stoi(req.param("id")); }
        catch (...) { return Response::badRequest("Parameter 'id' must be an integer"); }

        try {
            bool removed = Item::remove(id);
            if (!removed) return Response::notFound("Item not found");

            auto& registry = ServiceRegistry::instance();
            if (registry.hasCache())
                registry.cache().del(itemCacheKey(id));

            return Response::noContent();
        } catch (const std::exception& e) {
            return Response::serverError(e.what());
        }
    }
};

int main() {
    Application app("fastestapi.db", {
        .title       = "Item Store API",
        .version     = "1.0.0",
        .description = "A sample CRUD API built with FastestAPI"
    });

    // Optional: enable Redis cache
    app.cache({
        .host       = "127.0.0.1",
        .port       = 6379,
        .password   = "",        // set if Redis requires AUTH
        .defaultTtl = 60         // seconds
    });

    app.model<Item>();

    app.route<ListItems>();
    app.route<GetItem>();
    app.route<CreateItem>();
    app.route<UpdateItem>();
    app.route<DeleteItem>();

    app.listen("0.0.0.0", 8080);

    return 0;
}
