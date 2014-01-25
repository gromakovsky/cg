cmake_minimum_required(VERSION 2.6 FATAL_ERROR)

project(exam_examples)

add_executable(rasterization triangle_rasterization.cpp)
target_link_libraries(rasterization visualization ${GMP_LIBRARIES})

add_executable(polygons_distance polygons_distance.cpp)
target_link_libraries(polygons_distance visualization ${GMP_LIBRARIES})
