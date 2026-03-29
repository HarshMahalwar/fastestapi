#include <gtest/gtest.h>
#include <httplib.h>
#include <nlohmann/json.hpp>
#include <thread>
#include <chrono>

using json = nlohmann::json;

class ItemsAPITest : public ::testing::Test {
protected:
    httplib::Client cli{"localhost", 8080};

    void SetUp() override {
        cli.set_connection_timeout(5);
        cli.set_read_timeout(5);
    }

    // Helper: create an item and return its id
    int createItem(const std::string& name, double price) {
        json body = {{"name", name}, {"price", price}};
        auto res = cli.Post("/items", body.dump(), "application/json");
        EXPECT_NE(res, nullptr);
        if (!res) return -1;
        auto j = json::parse(res->body);
        return j.value("id", -1);
    }

    // Helper: delete an item
    void deleteItem(int id) {
        cli.Delete(("/items/" + std::to_string(id)).c_str());
    }
};

TEST_F(ItemsAPITest, CreateItemReturnsSuccess) {
    json body = {{"name", "TestWidget"}, {"price", 5.50}};
    auto res = cli.Post("/items", body.dump(), "application/json");
    ASSERT_TRUE(res);
    EXPECT_TRUE(res->status == 200 || res->status == 201);

    auto j = json::parse(res->body);
    EXPECT_EQ(j["name"], "TestWidget");
    EXPECT_DOUBLE_EQ(j["price"].get<double>(), 5.50);
    EXPECT_TRUE(j.contains("id"));

    deleteItem(j["id"].get<int>());
}

TEST_F(ItemsAPITest, CreateItemWithMissingFieldsFails) {
    // Sending empty JSON — should fail validation
    auto res = cli.Post("/items", "{}", "application/json");
    ASSERT_NE(res, nullptr);
    EXPECT_GE(res->status, 400);
}

TEST_F(ItemsAPITest, CreateItemWithInvalidJsonFails) {
    auto res = cli.Post("/items", "not json at all", "application/json");
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 400);
}

TEST_F(ItemsAPITest, GetExistingItemReturns200) {
    int id = createItem("Readable", 1.00);
    ASSERT_GT(id, 0);

    auto res = cli.Get(("/items/" + std::to_string(id)).c_str());
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 200);

    auto j = json::parse(res->body);
    EXPECT_EQ(j["name"], "Readable");

    deleteItem(id);
}

TEST_F(ItemsAPITest, GetNonExistentItemReturns404) {
    auto res = cli.Get("/items/999999");
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 404);
}

TEST_F(ItemsAPITest, ListItemsReturnsArray) {
    int id = createItem("Listed", 2.00);

    auto res = cli.Get("/items");
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 200);

    auto j = json::parse(res->body);
    EXPECT_TRUE(j.is_array());
    EXPECT_GE(j.size(), 1u);

    deleteItem(id);
}

TEST_F(ItemsAPITest, UpdateItemChangesFields) {
    int id = createItem("BeforeUpdate", 3.00);
    ASSERT_GT(id, 0);

    json patch = {{"price", 7.77}};
    auto res = cli.Put(
        ("/items/" + std::to_string(id)).c_str(),
        patch.dump(), "application/json"
    );
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 200);

    // Verify the update persisted
    auto verify = cli.Get(("/items/" + std::to_string(id)).c_str());
    ASSERT_NE(verify, nullptr);
    auto j = json::parse(verify->body);
    EXPECT_DOUBLE_EQ(j["price"].get<double>(), 7.77);

    deleteItem(id);
}

TEST_F(ItemsAPITest, UpdateNonExistentItemReturns404) {
    json patch = {{"price", 1.00}};
    auto res = cli.Put("/items/999999", patch.dump(), "application/json");
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 404);
}

TEST_F(ItemsAPITest, DeleteItemReturns200) {
    int id = createItem("Deletable", 4.00);
    ASSERT_GT(id, 0);

    auto res = cli.Delete(("/items/" + std::to_string(id)).c_str());
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 204);

    // Confirm it's gone
    auto verify = cli.Get(("/items/" + std::to_string(id)).c_str());
    ASSERT_NE(verify, nullptr);
    EXPECT_EQ(verify->status, 404);
}

TEST_F(ItemsAPITest, DeleteNonExistentItemReturns404) {
    auto res = cli.Delete("/items/999999");
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 404);
}

TEST_F(ItemsAPITest, ResponsesAreJson) {
    int id = createItem("ContentCheck", 6.00);
    auto res = cli.Get(("/items/" + std::to_string(id)).c_str());
    ASSERT_NE(res, nullptr);
    EXPECT_NE(res->get_header_value("Content-Type").find("application/json"),
              std::string::npos);
    deleteItem(id);
}