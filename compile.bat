@echo off
REM XShell v0.3.2 - Simple Compilation Script for Windows
REM This script provides basic compilation without advanced features

echo.
echo ========================================
echo XShell v0.3.2 - Simple Windows Build
echo ========================================
echo.

REM Create bin directory if it doesn't exist
if not exist "bin" mkdir bin

REM Check if GCC is available
gcc --version >nul 2>&1
if errorlevel 1 (
    echo ERROR: GCC not found in PATH
    echo Please install MinGW-w64 or MSYS2 and add to PATH
    echo Or use Visual Studio Developer Command Prompt with cl.exe
    pause
    exit /b 1
)

echo Building XShell with basic features...
echo.

REM Simple compilation with basic features
gcc -Iinclude -DXCODEX_ENABLE_COMPLETION src\*.c -o bin\Xshell.exe -lws2_32 -liphlpapi

if errorlevel 1 (
    echo.
    echo ERROR: Compilation failed!
    echo Check for missing dependencies or syntax errors.
    pause
    exit /b 1
)

echo.
echo ========================================
echo Build completed successfully!
echo ========================================
echo.
echo XShell.exe created in bin\ directory
echo.
echo To run XShell:
echo   bin\Xshell.exe
echo.
echo For full features, use build_windows.bat
echo.
pause
