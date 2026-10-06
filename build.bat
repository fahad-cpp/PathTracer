@echo off
set "PRESET=clang-release"
if exist build\%PRESET%\PathTracer.exe del build\%PRESET%\PathTracer.exe
cmake --preset %PRESET% >nul
cmake --build --preset %PRESET% 2>nul
copy build\%PRESET%\compile_commands.json build\ >nul
echo PathTracer.exe::
build\%PRESET%\PathTracer.exe