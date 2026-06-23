cmake_minimum_required(VERSION 3.20)
project(dander LANGUAGES CXX)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_compile_options(-Wall -Wextra -Wpedantic)


find_package(SQLite3 REQUIRED)
add_executable(dander-ingest 
    src/main.cpp
    src/storage/sqlite_store.cpp
)
target_include_directories(dander-ingest PRIVATE src)
target_link_libraries(dander-ingest PRIVATE SQLite::SQLite3)


include_directories(include)


add_executable(test_pms7003m_parse tests/test_pms7003m_parse.cpp)
add_executable(test_byte_order   tests/test_byte_order.cpp)
add_executable(test_udp_packet   tests/test_udp_packet.cpp)

enable_testing()
add_test(NAME pms7003m_parse COMMAND test_pms7003m_parse)
add_test(NAME byte_order     COMMAND test_byte_order)
add_test(NAME udp_packet     COMMAND test_udp_packet)