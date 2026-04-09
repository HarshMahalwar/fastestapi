cmake_minimum_required(VERSION 3.16)
project(test_fastestapi LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(fastestapi REQUIRED)

add_executable(test_fastestapi src/test.cpp)
target_link_libraries(test_fastestapi PRIVATE fastestapi::fastestapi)
