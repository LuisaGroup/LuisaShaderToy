include_directories("${CMAKE_CURRENT_SOURCE_DIR}")

add_subdirectory(compute)
add_subdirectory(ext)
add_subdirectory(gui)

add_library(luisa::shader-toy ALIAS luisa-shader-toy-gui)

add_subdirectory(apps)
