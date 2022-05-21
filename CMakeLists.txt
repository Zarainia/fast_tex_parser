cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 20)

# set the project name
project(fast_tex_parser)

# add the executable
find_package(PythonLibs REQUIRED)
include_directories(${PYTHON_INCLUDE_DIRS})
add_subdirectory(extern/pybind11)
pybind11_add_module(fast_tex_parser src/fast_tex_parser.cpp)
include_directories("include/")
target_link_libraries(fast_tex_parser PRIVATE ${MY_LIBRARIES})