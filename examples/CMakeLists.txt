project(examples LANGUAGES CXX)

macro(add_example)
  cmake_parse_arguments(ARGV "" "NAME" "SOURCES" ${ARGN})

  add_executable(${ARGV_NAME} ${ARGV_SOURCES})

  target_link_libraries(${ARGV_NAME} PRIVATE saturn glfw)
  target_compile_definitions(${ARGV_NAME} PRIVATE SATURN_INCLUDE_GLFW)
endmacro()

add_example(NAME basic SOURCES "basic/main.cpp")
