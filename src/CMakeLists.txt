find_package(PythonLibs REQUIRED)
set(Qt5_DIR D:/Qt/Qt5.14.2/5.14.2/msvc2017_64/lib/cmake/Qt5)
find_package(Qt5 COMPONENTS Core Widgets REQUIRED)

add_executable(${PROJECT_NAME} main.cpp VideoWidget.cpp)

include_directories(${PYTHON_INCLUDE_DIRS})
include_directories("D:/Anaconda3/Lib/site-packages/numpy/core/include")
target_link_libraries(${PROJECT_NAME} ${PYTHON_LIBRARIES} Qt5::Core Qt5::Widgets)