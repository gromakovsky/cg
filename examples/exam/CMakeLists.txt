cmake_minimum_required(VERSION 2.6 FATAL_ERROR)

project(triangle_rasterization_example)

add_executable(rasterization triangle_rasterization.cpp)
target_link_libraries(rasterization visualization ${GMP_LIBRARIES})
