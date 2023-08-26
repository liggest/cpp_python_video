cmake_minimum_required (VERSION 3.8)

project(cpp_python_video)

set(CMAKE_CXX_STANDARD 14)
if(APPLE)
    set(CMAKE_OSX_ARCHITECTURES "x86_64")
endif()

    add_subdirectory(src)

install(DIRECTORY resource DESTINATION .)
# install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_SOURCE_DIR}/py ${CMAKE_INSTALL_PREFIX}/py)")