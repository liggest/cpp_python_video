# $Env:ZMQ_PREFIX = "Z:\Software\vcpkg\vcpkg\packages\zeromq_x64-windows"
# path\to\zmq\bin, path\to\zmq\lib, path\to\zmq\include ...
# cp $Env:ZMQ_PREFIX\bin\libzmq*.dll $Env:ZMQ_PREFIX\lib
# If libzmq is built locally, use these ↑

$Env:PDM_NO_BINARY = "pyzmq"
pdm add pyzmq
