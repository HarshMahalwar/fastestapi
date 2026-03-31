cmake_minimum_required(VERSION 3.16)
project(FastestAPI VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# cpp-httplib (header-only HTTP server)
include(FetchContent)
FetchContent_Declare(
    httplib
    GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
    GIT_TAG        v0.40.0
)
FetchContent_MakeAvailable(httplib)

# nlohmann/json (header-only JSON library)
FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        v3.12.0
)
FetchContent_MakeAvailable(json)

# SQLite3
find_package(SQLite3 REQUIRED)
find_package(Poco REQUIRED COMPONENTS Redis Net Foundation)

add_library(fastestapi INTERFACE)
target_include_directories(fastestapi INTERFACE
    ${PROJECT_SOURCE_DIR}/include
)
target_link_libraries(fastestapi INTERFACE
    httplib::httplib
    nlohmann_json::nlohmann_json
    SQLite::SQLite3
    Poco::Redis
    Poco::Net
    Poco::Foundation
)


add_executable(example examples/main.cpp)
target_link_libraries(example PRIVATE fastestapi)

option(BUILD_TESTS "Build test suite" OFF)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()