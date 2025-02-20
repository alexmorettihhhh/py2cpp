cmake_minimum_required(VERSION 3.10)
project(PythonToCppDecompiler)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(py2cpp py_to_cpp_decompiler.cpp) 