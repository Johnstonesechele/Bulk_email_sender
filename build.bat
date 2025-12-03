@echo off
echo Building Bulk Email Manager...

REM Check if Qt6 is installed
if not exist "C:\Qt\6.9.1\mingw_64\bin\qmake.exe" (
    echo Qt6 not found at C:\Qt\6.9.1\mingw_64
    echo Please install Qt6 or update the path in this script
    pause
    exit /b 1
)

REM Create build directory
if not exist "build" mkdir build
cd build

REM Configure with CMake
echo Configuring with CMake...
cmake .. -DCMAKE_PREFIX_PATH="C:/Qt/6.9.1/mingw_64" -G "MinGW Makefiles"

if %ERRORLEVEL% neq 0 (
    echo CMake configuration failed
    pause
    exit /b 1
)

REM Build the project
echo Building project...
mingw32-make

if %ERRORLEVEL% neq 0 (
    echo Build failed
    pause
    exit /b 1
)

echo Build completed successfully!
echo Executable location: build\BulkEmailApp.exe

REM Copy Qt DLLs if needed
echo Copying Qt DLLs...
if exist "C:\Qt\6.9.1\mingw_64\bin\Qt6Core.dll" (
    copy "C:\Qt\6.9.1\mingw_64\bin\Qt6Core.dll" "."
    copy "C:\Qt\6.9.1\mingw_64\bin\Qt6Widgets.dll" "."
    copy "C:\Qt\6.9.1\mingw_64\bin\Qt6Network.dll" "."
    copy "C:\Qt\6.9.1\mingw_64\bin\Qt6Sql.dll" "."
    copy "C:\Qt\6.9.1\mingw_64\bin\Qt6Gui.dll" "."
)

echo.
echo Build completed! You can now run BulkEmailApp.exe from the Release folder.
pause