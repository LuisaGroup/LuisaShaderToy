if (APPLE)
    enable_language(OBJC OBJCXX)
endif ()

add_library(luisa-shader-toy-ext INTERFACE)

# OpenGL
find_package(OpenGL REQUIRED)
target_link_libraries(luisa-shader-toy-ext INTERFACE OpenGL::GL)

# glfw
if (NOT APPLE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
    set(GLFW_LIBRARY_TYPE SHARED CACHE STRING "" FORCE)
    add_subdirectory(glfw)
else ()
    find_package(GLFW3 REQUIRED)
endif ()
target_link_libraries(luisa-shader-toy-ext INTERFACE glfw)

# glad
add_subdirectory(glad)
target_link_libraries(luisa-shader-toy-ext INTERFACE glad)

# ImGui
add_subdirectory(imgui)
target_link_libraries(luisa-shader-toy-ext INTERFACE imgui)
