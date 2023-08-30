# cpp_python_video

Try to transfer video data from python to cpp

## Necessities

- CMake
- Qt

## Before Build

- Revise `src/CMakeLists.txt`    
  Change variable values (like `CMAKE_PREFIX_PATH` `Qt5_DIR` `imported_location`)

## After Build

- Prepare [python environment](py/README.md)
- Create symbolic link of `py` directory to output folders (like `build` and `out`)

