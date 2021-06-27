add_executable(fractal_pyramid fractal_pyramid.cpp)
target_link_libraries(fractal_pyramid PRIVATE luisa::shader-toy)

add_executable(energy_plant energy_plant.cpp)
target_link_libraries(energy_plant PRIVATE luisa::shader-toy)
