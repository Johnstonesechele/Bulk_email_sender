@echo off
echo Building Bulk Email Manager for Windows...

REM Check if Qt6 is available
where qmake >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo Qt6 not found in PATH. Please install Qt6 and add it to PATH.
    echo You can download Qt6 from: https://www.qt.io/download
    pause
    exit /b 1
)

REM Check if CMake is available
where cmake >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo CMake not found in PATH. Please install CMake and add it to PATH.
    echo You can download CMake from: https://cmake.org/download/
    pause
    exit /b 1
)

REM Create build directory
if not exist build mkdir build
cd build

REM Configure with CMake
echo Configuring project...
cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release ..
if %ERRORLEVEL% NEQ 0 (
    echo CMake configuration failed!
    pause
    exit /b 1
)

REM Build the project
echo Building project...
cmake --build . --config Release
if %ERRORLEVEL% NEQ 0 (
    echo Build failed!
    pause
    exit /b 1
)

REM Copy Qt6 dependencies
echo Copying Qt6 dependencies...
windeployqt BulkEmailApp.exe

echo.
echo Build completed successfully!
echo Executable location: %CD%\BulkEmailApp.exe
echo.
echo To run the application, execute: BulkEmailApp.exe
echo.
pause