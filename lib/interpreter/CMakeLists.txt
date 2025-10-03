add_library(interpreter interpreter.h interpreter.cpp)
target_include_directories(interpreter PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(interpreter PRIVATE parser AST)
