# set(PYBIND11_PYTHON_VERSION 2.7 3.6)
# add_subdirectory(../pybind11 pybind11)
# 
# pybind11_add_module(example example.cpp)


cmake_minimum_required(VERSION 2.8.12)
project(example)

add_subdirectory(dep/pybind11 REQUIRED)
pybind11_add_module(example example.cpp)
