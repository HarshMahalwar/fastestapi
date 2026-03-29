include(FetchContent)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.14.0
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(unit_tests
    test_items_api.cpp
)

target_link_libraries(unit_tests
    PRIVATE
        gtest_main
        httplib::httplib          # from parent project's FetchContent
        nlohmann_json::nlohmann_json
)

target_compile_features(unit_tests PRIVATE cxx_std_17)

include(GoogleTest)
gtest_discover_tests(unit_tests)