cmake_minimum_required(VERSION 3.20)
project(dander LANGUAGES CXX)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_compile_options(-Wall -Wextra -Wpedantic)


find_package(SQLite3 REQUIRED)
find_package(OpenCV REQUIRED)
find_package(nlohmann_json REQUIRED)
add_executable(dander-ingest 
    src/main.cpp
    src/storage/sqlite_store.cpp
)
target_include_directories(dander-ingest PRIVATE src)
target_link_libraries(dander-ingest PRIVATE SQLite3::SQLite3)

add_executable(dander-viewer
    src/viewer/main.cpp
    src/storage/sqlite_reader.cpp
)
target_compile_definitions(dander-viewer PRIVATE
    DANDER_PROJECT_ROOT="${CMAKE_SOURCE_DIR}"
)
target_include_directories(dander-viewer PRIVATE src)
target_link_libraries(dander-viewer PRIVATE 
    ${OpenCV_LIBS}
    SQLite3::SQLite3
)


include_directories(include)


add_executable(test_pms7003m_parse tests/test_pms7003m_parse.cpp)
add_executable(test_byte_order   tests/test_byte_order.cpp)
add_executable(test_udp_packet   tests/test_udp_packet.cpp)

enable_testing()
add_test(NAME pms7003m_parse COMMAND test_pms7003m_parse)
add_test(NAME byte_order     COMMAND test_byte_order)
add_test(NAME udp_packet     COMMAND test_udp_packet)


if(APPLE)
    execute_process(COMMAND brew --prefix onnxruntime
        OUTPUT_VARIABLE ONNXRUNTIME_PREFIX OUTPUT_STRIP_TRAILING_WHITESPACE)
    list(APPEND CMAKE_PREFIX_PATH "${ONNXRUNTIME_PREFIX}")
endif()
find_package(onnxruntime REQUIRED)

add_executable(ort_smoke src/inference/ort_smoke.cpp)
target_compile_definitions(ort_smoke PRIVATE DANDER_PROJECT_ROOT="${CMAKE_SOURCE_DIR}")
target_link_libraries(ort_smoke PRIVATE 
    onnxruntime::onnxruntime   
)


add_executable(render_field
    src/viewer/render_field.cpp
    src/inference/field_model.cpp)
target_include_directories(render_field PRIVATE src)
target_compile_definitions(render_field PRIVATE DANDER_PROJECT_ROOT="${CMAKE_SOURCE_DIR}")
target_link_libraries(render_field PRIVATE
    ${OpenCV_LIBS}
    onnxruntime::onnxruntime
    nlohmann_json::nlohmann_json)