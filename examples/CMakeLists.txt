add_subdirectory(viewer)
add_subdirectory(operations)
add_subdirectory(orientation)
add_subdirectory(convex_hull)
add_subdirectory(convex)

add_executable(visibility_graph visibility_graph.cpp)
target_link_libraries(visibility_graph visualization)
