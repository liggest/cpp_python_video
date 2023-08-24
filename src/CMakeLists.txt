find_package(PythonLibs REQUIRED)
set(Qt5_DIR D:/Qt/Qt5.14.2/5.14.2/msvc2017_64/lib/cmake/Qt5)
find_package(Qt5 COMPONENTS Core Widgets Gui REQUIRED)

add_executable(${PROJECT_NAME} main.cpp VideoWidget.cpp)

include_directories(${PYTHON_INCLUDE_DIRS})
include_directories("D:/Anaconda3/Lib/site-packages/numpy/core/include")
target_link_libraries(${PROJECT_NAME} ${PYTHON_LIBRARIES} Qt5::Core Qt5::Widgets Qt5::Gui)

add_custom_command(
    TARGET cpp_python_video POST_BUILD 
	COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt5::Core> $<TARGET_FILE_DIR:cpp_python_video>
	COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt5::Widgets> $<TARGET_FILE_DIR:cpp_python_video>
	COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt5::Gui> $<TARGET_FILE_DIR:cpp_python_video>
)

install(TARGETS cpp_python_video DESTINATION bin)
install(FILES read_video.py DESTINATION bin)
install(FILES ../resource/video.mp4 DESTINATION resource)
install(FILES ${CMAKE_BINARY_DIR}/src/Qt5Core.dll DESTINATION bin)
install(FILES ${CMAKE_BINARY_DIR}/src/Qt5Gui.dll DESTINATION bin)
install(FILES ${CMAKE_BINARY_DIR}/src/Qt5Widgets.dll DESTINATION bin)

