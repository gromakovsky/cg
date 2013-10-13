cmake_minimum_required(VERSION 2.6 FATAL_ERROR)

project(visibility-example)

add_executable(visibility_graph_slow visibility_graph_slow.cpp)
target_link_libraries(visibility_graph_slow visualization ${GMP_LIBRARIES})

add_executable(shortest_path shortest_path.cpp)
target_link_libraries(shortest_path visualization ${GMP_LIBRARIES})
