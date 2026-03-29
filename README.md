# FastestAPI: C++ Web Framework

A lightweight C++ REST framework inspired by Python's FastAPI


## Dependencies

| Library | Purpose | Acquisition |
|---|---|---|
| [cpp-httplib](https://github.com/yhirose/cpp-httplib) | HTTP server | CMake FetchContent |
| [nlohmann/json](https://github.com/nlohmann/json) | JSON parsing | CMake FetchContent |
| SQLite3 | Database | System package |

## Build & Run

### Prerequisites

- C++17 compiler (GCC 9+, Clang 10+, MSVC 2019+)
- CMake 3.16+
- SQLite3 development package

### Install SQLite3 (if needed)

**Ubuntu / Debian:**
```bash
sudo apt-get install cmake
sudo apt-get install libsqlite3-dev
```

### Build

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

### Run

```bash
./example
```

The server starts on `http://localhost:8080`.

### Test with curl

```bash
# Create
curl -X POST http://localhost:8080/items \
     -H "Content-Type: application/json" \
     -d '{"name":"Widget","price":9.99}'

# List all
curl http://localhost:8080/items

# Get one
curl http://localhost:8080/items/1

# Update
curl -X PUT http://localhost:8080/items/1 \
     -H "Content-Type: application/json" \
     -d '{"price":12.50}'

# Delete
curl -X DELETE http://localhost:8080/items/1
```

## Extending

### Add a new model

```cpp
struct User : public Model<User> {
    int         id = 0;
    std::string username;
    std::string email;

    static std::string tableName()  { return "users"; }
    static std::string primaryKey() { return "id"; }
    static std::vector<FieldDef> fieldDefs() {
        return {
            {"id",       FieldType::Integer, {FieldConstraint::PrimaryKey,
                                               FieldConstraint::AutoIncrement}},
            {"username", FieldType::Text,    {FieldConstraint::NotNull}},
            {"email",    FieldType::Text,    {FieldConstraint::NotNull}},
        };
    }
    // ... implement toJson, fromJson, fromRow, merge,
    //     writableColumns, writableValues
};
```

### Add a new endpoint

```cpp
class GetUser : public GetEndpoint {
public:
    GetUser() : GetEndpoint("/users/{id}") {}
    Response handle(const Request& req) override {
        int id = std::stoi(req.param("id"));
        auto user = User::get(id);
        if (!user) return Response::notFound();
        return Response::json(user->toJson());
    }
};

// Register in main:
app.model<User>();
app.route<GetUser>();
```

### Swap database backend

Implement `IDatabase` for PostgreSQL, MySQL, etc. and inject it:
```cpp
auto pg = std::make_shared<PostgresDatabase>();
ServiceRegistry::instance().setDatabase(pg);
```
