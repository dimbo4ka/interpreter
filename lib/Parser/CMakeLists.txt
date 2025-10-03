add_library(parser Parser.hpp Parser.cpp)
target_include_directories(parser PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(parser PRIVATE lexer AST)
