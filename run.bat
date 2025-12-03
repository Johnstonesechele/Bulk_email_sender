@echo off
echo Starting Bulk Email Manager...
echo.

REM Change to build directory
cd /d "%~dp0build"

REM Check if executable exists
if not exist "BulkEmailApp.exe" (
    echo ERROR: BulkEmailApp.exe not found!
    echo Please build the application first using build.bat
    pause
    exit /b 1
)

REM Check for required Qt DLLs
if not exist "Qt6Core.dll" (
    echo ERROR: Qt DLLs not found!
    echo Please run build.bat to copy Qt libraries
    pause
    exit /b 1
)

REM Check for platform plugins
if not exist "platforms\qwindows.dll" (
    echo ERROR: Qt platform plugins not found!
    echo Please run build.bat to copy Qt plugins
    pause
    exit /b 1
)

echo All dependencies found. Starting application...
echo.

REM Start the application
start "Bulk Email Manager" /wait BulkEmailApp.exe

REM Check exit code
if %ERRORLEVEL% equ 0 (
    echo Application closed normally.
) else (
    echo Application exited with error code: %ERRORLEVEL%
    echo.
    echo This could indicate:
    echo - Missing Qt dependencies
    echo - Database initialization issues
    echo - Configuration problems
    echo.
    echo Try rebuilding with build.bat
    pause
)
