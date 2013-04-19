cmake_minimum_required(VERSION 2.6 FATAL_ERROR)

project(operations-example)

add_executable(triangle_contains_point triangle_contains_point.cpp)
target_link_libraries(triangle_contains_point visualization ${GMP_LIBRARIES})

add_executable(segment_intersects_segment segment_intersects_segment.cpp)
target_link_libraries(segment_intersects_segment visualization ${GMP_LIBRARIES})
