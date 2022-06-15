cmake_minimum_required(VERSION 3.5)

project(test CXX)

set(CMAKE_BUILD_TYPE Debug)

aux_source_directory(${CMAKE_SOURCE_DIR}/test SRCTEST)

add_executable(${PROJECT_NAME} ${SRCTEST})

target_link_libraries(${PROJECT_NAME} PRIVATE gpio_controller)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/src)



