function(luisa_shader_toy_add_executable name)
    add_executable(${name} ${name}.cpp)
    target_link_libraries(${name} PRIVATE luisa::shader-toy)
endfunction()

luisa_shader_toy_add_executable(fractal_pyramid)
luisa_shader_toy_add_executable(energy_plant)
luisa_shader_toy_add_executable(star_nest)
luisa_shader_toy_add_executable(protean_clouds)
