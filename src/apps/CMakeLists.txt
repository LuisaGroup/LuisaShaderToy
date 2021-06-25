find_package(OpenCV CONFIG)

if (OpenCV_FOUND)
    add_executable(fractal_pyramid fractal_pyramid.cpp)
    target_link_libraries(fractal_pyramid PRIVATE luisa::compute luisa-shader-toy-ext ${OpenCV_LIBS})
endif ()
