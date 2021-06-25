add_library(stb SHARED stb.cpp)
target_include_directories(stb PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
set_target_properties(stb PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
