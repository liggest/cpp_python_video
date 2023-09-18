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

## Useage

### ZeroMQPairPlugin

C++ & Python communicate as pair.

``` bash
pdm run python -m video_reader.zeromq_pair
```

### ZeroMQPlugin

Python control everything, C++ is only a player to receive data.


``` bash
pdm run python -m video_reader.zeromq
```

### ZeroMQIOPlugin

C++ read data from microphone, send to python and python will send back, then C++ plays audio that just record.

``` bash
pdm run python -m video_reader.zeromq_io
```