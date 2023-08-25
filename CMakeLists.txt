
cmake_minimum_required (VERSION 3.8)

project(cpp_python_video)

set(CMAKE_CXX_STANDARD 14)


add_subdirectory(src)

install(DIRECTORY resource DESTINATION .)