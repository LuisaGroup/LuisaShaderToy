add_library(glad SHARED glad/glad.c glad/glad.h glad/khrplatform.h)
target_link_libraries(glad PUBLIC OpenGL::GL)
target_include_directories(glad PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
set_target_properties(glad PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
