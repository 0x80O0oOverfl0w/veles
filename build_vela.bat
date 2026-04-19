@echo off
call "E:\Program Files (x86)\Microsoft Visual Studio\2017\Professional\VC\Auxiliary\Build\vcvarsall.bat" amd64
cd /d E:\src\vveles
cmake -B build -DCMAKE_BUILD_TYPE=Debug -DCMAKE_PREFIX_PATH=D:\Qt\Qt5.12.2\5.12.2\msvc2017_64
cmake --build build