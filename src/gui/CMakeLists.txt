set(LUISA_SHADER_TOY_GUI_SOURCES
        framerate.cpp framerate.h
        window.cpp window.h
        hid.h
        imgui_impl_glfw.cpp imgui_impl_glfw.h
        gl_texture.cpp gl_texture.h
        shader_toy.cpp shader_toy.h)

add_library(luisa-shader-toy-gui SHARED ${LUISA_SHADER_TOY_GUI_SOURCES})
target_link_libraries(luisa-shader-toy-gui PUBLIC luisa-shader-toy-ext luisa::compute)
target_compile_definitions(luisa-shader-toy-gui PUBLIC GLFW_INCLUDE_NONE)
set_target_properties(luisa-shader-toy-gui PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)

find_package(OpenCV CONFIG)
if (OpenCV_FOUND)
    target_link_libraries(luisa-shader-toy-gui PUBLIC ${OpenCV_LIBS})
    target_compile_definitions(luisa-shader-toy-gui PUBLIC LUISA_SHADERTOY_HAS_OPENCV=1)
endif ()
