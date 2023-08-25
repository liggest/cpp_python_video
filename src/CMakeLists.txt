find_package(PythonLibs REQUIRED)
set(Qt5_DIR D:/Qt/Qt5.14.2/5.14.2/msvc2017_64/lib/cmake/Qt5)
find_package(Qt5 COMPONENTS Core Widgets Gui REQUIRED)

add_executable(${PROJECT_NAME} main.cpp VideoWidget.cpp)

include_directories(${PYTHON_INCLUDE_DIRS})
include_directories("D:/Anaconda3/Lib/site-packages/numpy/core/include")
target_link_libraries(${PROJECT_NAME} ${PYTHON_LIBRARIES} Qt5::Core Qt5::Widgets Qt5::Gui)


# 处理 windeployqt
set(imported_location "D:/Qt/Qt5.14.2/5.14.2/msvc2017_64/bin/windeployqt.exe")

if(EXISTS ${imported_location})
    add_executable(Qt5::windeployqt IMPORTED)

    set_target_properties(Qt5::windeployqt PROPERTIES
        IMPORTED_LOCATION ${imported_location}
    )
endif()

add_custom_command(TARGET cpp_python_video POST_BUILD
        COMMAND Qt5::windeployqt --dir "${CMAKE_CURRENT_BINARY_DIR}/windeployqt" "$<TARGET_FILE_DIR:cpp_python_video>/$<TARGET_FILE_NAME:cpp_python_video>"
    )

install(
    DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/windeployqt/"
    DESTINATION bin
)

# 复制 exe 和 py
install(TARGETS cpp_python_video DESTINATION bin)
install(FILES read_video.py DESTINATION bin)

