net session >nul 2>nul
if %errorlevel% neq 0 (
    echo %errorlevel%
    powershell -Command "Start-Process '%~f0' -Verb RunAs -WorkingDirectory '%~dp0'"
    exit /b 
)
cd /d "%~dp0"
wpr -start CPU
build\clang-debug\PathTracer.exe
wpr -stop trace.etl