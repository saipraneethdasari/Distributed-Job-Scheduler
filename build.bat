@echo off
REM Build script for mini-scheduler project

echo Building mini-scheduler...

REM Create build directory if it doesn't exist
if not exist build mkdir build

REM Build scheduler
echo.
echo [1/2] Building scheduler...
g++ -std=c++17 -pthread scheduler/scheduler.cpp scheduler/server.cpp -lws2_32 -o build/scheduler.exe
if errorlevel 1 (
    echo ERROR: Failed to build scheduler
    exit /b 1
)
echo Scheduler built successfully: build/scheduler.exe

REM Build worker
echo.
echo [2/2] Building worker...
g++ -std=c++17 -pthread worker/worker.cpp -lws2_32 -o build/worker.exe
if errorlevel 1 (
    echo ERROR: Failed to build worker
    exit /b 1
)
echo Worker built successfully: build/worker.exe

echo.
echo All builds completed successfully!
echo.
echo Executables:
dir /b build\*.exe
