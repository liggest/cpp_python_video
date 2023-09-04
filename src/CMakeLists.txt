# 查找第三方依赖
if(WIN32)
    # set(Qt5_ROOT Z:/Software/Qt/Qt/5.14.2/msvc2017_64/)
    set(CMAKE_PREFIX_PATH Z:/Software/Qt/Qt/5.14.2/msvc2017_64/lib/cmake/Qt5)
    set(Qt5_DIR Z:/Software/Qt/Qt/5.14.2/msvc2017_64/)
    find_package(Qt5 COMPONENTS Core Widgets Gui REQUIRED)

    # find_package(PythonLibs REQUIRED)
    # set(PYTHON_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/py/.venv/include)
    # set(PYTHON_LIBRARIES ${CMAKE_SOURCE_DIR}/py/.venv/python310.dll)
    # set(ENV{PYTHONHOME} ${CMAKE_SOURCE_DIR}/py/.venv)
    set(Python_ROOT_DIR ${CMAKE_SOURCE_DIR}/py/.venv)
    find_package(Python COMPONENTS Interpreter Development NumPy)

    # message(STATUS  "Python_FOUND  ${Python_FOUND}")
    # message(STATUS  "Python_Interpreter_FOUND  ${Python_Interpreter_FOUND}")
    # message(STATUS  "Python_EXECUTABLE  ${Python_EXECUTABLE}")
    # message(STATUS  "Python_INTERPRETER_ID  ${Python_INTERPRETER_ID}")
    # message(STATUS  "Python_INCLUDE_DIRS  ${Python_INCLUDE_DIRS}")
    # message(STATUS  "Python_NumPy_INCLUDE_DIRS  ${Python_NumPy_INCLUDE_DIRS}")
    # message(STATUS  "Python_LIBRARIES  ${Python_LIBRARIES}")
    # message(STATUS  "Python_RUNTIME_LIBRARY_DIRS  ${Python_RUNTIME_LIBRARY_DIRS}")
    # message(STATUS  "Python_STDLIB  ${Python_STDLIB}")

    # set(CMAKE_TOOLCHAIN_FILE ${VCPKG_INSTALLED_DIR}/../scripts/buildsystems/vcpkg.cmake)
    set(cppzmq_DIR ${VCPKG_INSTALLED_DIR}/x64-windows/share/cppzmq)
    set(ZeroMQ_DIR ${VCPKG_INSTALLED_DIR}/x64-windows/share/zeromq)

elseif(APPLE)
#    set(PYTHON_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/py/.venv/include/python3.10)
#    set(PYTHON_LIBRARIES ${CMAKE_SOURCE_DIR}/py/.venv/lib/libpython3.10.dylib)

    set(Python_ROOT_DIR ${CMAKE_SOURCE_DIR}/py/.venv)
    find_package(Python COMPONENTS Interpreter Development NumPy)
    set(CMAKE_PREFIX_PATH "/Users/duolanda/Qt/6.3.1/macos/lib/cmake")
    find_package(Qt6 COMPONENTS Core Widgets Gui REQUIRED )
endif()

find_package(cppzmq)

# 生成可执行文件
add_executable(${PROJECT_NAME} main.cpp VideoWidget.cpp Plugin.cpp PyAPIPlugin.cpp PluginManager.cpp "ZeroMQPlugin.cpp")

# 链接头文件和库
    # include_directories(${PYTHON_INCLUDE_DIRS})
    include_directories(${Python_INCLUDE_DIRS})
    # include_directories("${CMAKE_SOURCE_DIR}/py/.venv/include")
    # include_directories("D:/Anaconda3/Lib/site-packages/numpy/core/include")
    # include_directories("${CMAKE_SOURCE_DIR}/py/.venv/Lib/site-packages/numpy/core/include")
    include_directories(${Python_NumPy_INCLUDE_DIRS})
    # target_link_libraries(${PROJECT_NAME} ${PYTHON_LIBRARIES} Qt5::Core Qt5::Widgets Qt5::Gui)
if(WIN32)
    target_link_libraries(${PROJECT_NAME} ${Python_LIBRARIES} Qt5::Core Qt5::Widgets Qt5::Gui cppzmq)
elseif(APPLE)
    target_link_libraries(${PROJECT_NAME} ${Python_LIBRARIES} Qt6::Core Qt6::Widgets Qt6::Gui cppzmq)
endif()


# 处理 windeployqt
if(WIN32)
    set(imported_location "Z:/Software/Qt/Qt/5.14.2/msvc2017_64/bin/windeployqt.exe")

    if(EXISTS ${imported_location})
        add_executable(Qt5::windeployqt IMPORTED)

        set_target_properties(Qt5::windeployqt PROPERTIES
            IMPORTED_LOCATION ${imported_location}
        )
    endif()

    add_custom_command(TARGET cpp_python_video POST_BUILD
            COMMAND Qt5::windeployqt --dir "${CMAKE_CURRENT_BINARY_DIR}/windeployqt"
            "$<TARGET_FILE_DIR:cpp_python_video>/$<TARGET_FILE_NAME:cpp_python_video>"
        )

    install(
        DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/windeployqt/"
        DESTINATION bin
    )
endif()

# 复制 exe 和 py
install(TARGETS cpp_python_video DESTINATION bin)
# install(FILES read_video.py DESTINATION bin)