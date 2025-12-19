cmake_minimum_required(VERSION 3.25)

project(
    gruniq
    VERSION 0.0.1
    LANGUAGES CXX
)

find_package(PCRE2 REQUIRED)

add_executable(
    gruniq
    "${CMAKE_CURRENT_SOURCE_DIR}/src/build.cpp"
)

target_link_libraries(
    gruniq
    PRIVATE
    PCRE2::8BIT
)

target_include_directories(gruniq PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")

set_property(TARGET gruniq PROPERTY CXX_STANDARD 20)
