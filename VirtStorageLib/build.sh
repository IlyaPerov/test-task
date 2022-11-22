#!/bin/bash 

mkdir build
cd build

cmake ..
cmake --build .

#Run tests
cd tests
ctest