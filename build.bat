@echo off
set "CONFIG=Debug"
if exist build\PathTracer.exe del build\PathTracer.exe
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=%CONFIG% -DCMAKE_EXPORT_COMPILE_COMMANDS=ON >nul
cmake --build build --config %CONFIG% --parallel 2>nul
echo PathTracer.exe::
build\PathTracer.exe