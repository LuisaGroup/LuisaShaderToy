add_library(luisa-shader-toy-ext INTERFACE)

# OpenGL
find_package(OpenGL REQUIRED)
target_link_libraries(luisa-shader-toy-ext INTERFACE OpenGL::GL)

# glfw
find_package(GLFW3 CONFIG)
if (NOT GLFW3_FOUND)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
    set(BUILD_SHARED_LIBS OFF)
    add_subdirectory(glfw)
endif ()
target_link_libraries(luisa-shader-toy-ext INTERFACE glfw)

# glad
add_subdirectory(glad)
target_link_libraries(luisa-shader-toy-ext INTERFACE glad)

# ImGui
add_subdirectory(imgui)
target_link_libraries(luisa-shader-toy-ext INTERFACE imgui)

# stb
add_subdirectory(stb)
target_link_libraries(luisa-shader-toy-ext INTERFACE stb)
