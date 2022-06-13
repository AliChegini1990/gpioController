cmake_minimum_required(VERSION 3.5)

project(gpio_controller CXX)

# debug mode enable
set(CMAKE_BUILD_TYPE Debug)

aux_source_directory(src SRC)

add_library(${PROJECT_NAME} ${SRC})

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 14)

# current directory
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/src)

# libsoc 
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/libsoc/include/)
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/libsoc/lib/)
target_link_libraries(${PROJECT_NAME} ${PROJECT_SOURCE_DIR}/libsoc/lib/libsoc.a)

# compile options
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wpedantic -Wextra)

target_link_libraries(${PROJECT_NAME} -lpthread)


