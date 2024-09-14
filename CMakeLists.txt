cmake_minimum_required(VERSION 3.29)
project(ict C)

set(CMAKE_C_STANDARD 23)

add_executable(${PROJECT_NAME} main.c)
target_link_libraries(${PROJECT_NAME} m)
