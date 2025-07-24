@echo off
REM XShell v0.3.2 - Full Windows Build Script
REM This script builds XShell with all available features

echo.
echo ========================================
echo XShell v0.3.2 - Full Windows Build
echo ========================================
echo.

REM Create bin directory if it doesn't exist
if not exist "bin" mkdir bin

REM Check if GCC is available
gcc --version >nul 2>&1
if errorlevel 1 (
    echo ERROR: GCC not found in PATH
    echo Please install MinGW-w64 or MSYS2 and add to PATH
    echo.
    echo Alternative: Use Visual Studio Developer Command Prompt
    echo and compile with cl.exe instead of gcc
    pause
    exit /b 1
)

echo Checking for optional dependencies...
echo.

REM Initialize flags
set "EXTRA_FLAGS="
set "EXTRA_LIBS="
set "LUA_AVAILABLE=false"
set "JSON_AVAILABLE=false"

REM Check for Lua development libraries
echo Checking for Lua support...
pkg-config --exists lua5.4 >nul 2>&1
if not errorlevel 1 (
    echo   - Lua 5.4 found
    set "EXTRA_FLAGS=%EXTRA_FLAGS% -DXCODEX_ENABLE_LUA -DXCODEX_ENABLE_COMPLETION"
    set "EXTRA_LIBS=%EXTRA_LIBS% -llua5.4"
    set "LUA_AVAILABLE=true"
    goto :lua_done
)

pkg-config --exists lua >nul 2>&1
if not errorlevel 1 (
    echo   - Lua found
    set "EXTRA_FLAGS=%EXTRA_FLAGS% -DXCODEX_ENABLE_LUA -DXCODEX_ENABLE_COMPLETION"
    set "EXTRA_LIBS=%EXTRA_LIBS% -llua"
    set "LUA_AVAILABLE=true"
    goto :lua_done
)

REM Fallback Lua check
if exist "C:\msys64\mingw64\include\lua.h" (
    echo   - Lua found in MSYS2
    set "EXTRA_FLAGS=%EXTRA_FLAGS% -DXCODEX_ENABLE_LUA -DXCODEX_ENABLE_COMPLETION"
    set "EXTRA_LIBS=%EXTRA_LIBS% -llua"
    set "LUA_AVAILABLE=true"
    goto :lua_done
)

echo   - Lua not found (plugin system disabled)

:lua_done

REM Check for JSON-C libraries
echo Checking for JSON-C support...
pkg-config --exists json-c >nul 2>&1
if not errorlevel 1 (
    echo   - JSON-C found
    set "EXTRA_FLAGS=%EXTRA_FLAGS% -DXCODEX_ENABLE_LSP"
    set "EXTRA_LIBS=%EXTRA_LIBS% -ljson-c"
    set "JSON_AVAILABLE=true"
) else (
    echo   - JSON-C not found (LSP support disabled)
)

echo.
echo Building XShell with the following features:
echo   - Core XShell functionality: YES
echo   - XCodex text editor: YES
echo   - Basic completion: YES
echo   - Lua plugin system: %LUA_AVAILABLE%
echo   - LSP integration: %JSON_AVAILABLE%
echo   - Cross-platform support: YES
echo   - Network commands: YES
echo   - Encryption: YES
echo.

REM Build command
echo Compiling...
gcc -Iinclude -Wall -Wextra -g -std=c11 %EXTRA_FLAGS% -DXCODEX_ENABLE_COMPLETION src\*.c -o bin\Xshell.exe -lws2_32 -liphlpapi %EXTRA_LIBS%

if errorlevel 1 (
    echo.
    echo ========================================
    echo ERROR: Full build failed!
    echo ========================================
    echo.
    echo Trying simple build as fallback...
    echo.
    
    gcc -Iinclude -DXCODEX_ENABLE_COMPLETION src\*.c -o bin\Xshell.exe -lws2_32 -liphlpapi
    
    if errorlevel 1 (
        echo Simple build also failed. Check your environment.
        pause
        exit /b 1
    ) else (
        echo Simple build succeeded (limited features)
    )
) else (
    echo.
    echo ========================================
    echo Full build completed successfully!
    echo ========================================
)

echo.
echo XShell.exe created in bin\ directory
echo.
echo Features built:
if "%LUA_AVAILABLE%"=="true" (
    echo   ✓ Lua plugin system enabled
) else (
    echo   ✗ Lua plugin system disabled
)
if "%JSON_AVAILABLE%"=="true" (
    echo   ✓ LSP integration enabled
) else (
    echo   ✗ LSP integration disabled
)
echo   ✓ XCodex modal editor
echo   ✓ Syntax highlighting ^(14+ languages^)
echo   ✓ Cross-platform compatibility
echo   ✓ Network commands
echo   ✓ Encryption features
echo.
echo To run XShell:
echo   bin\Xshell.exe
echo.
echo To test XCodex editor:
echo   bin\Xshell.exe
echo   ^(then type: xcodex test.c^)
echo.
pause
