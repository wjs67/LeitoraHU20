@echo off
REM ============================================================================
REM Direct Compilation (WITHOUT CMake) - LeitoraHU20
REM ============================================================================
REM This script compiles the project directly with MSVC, without needing CMake
REM Usage: compile_direct.bat
REM ============================================================================
setlocal enabledelayedexpansion

echo.
echo ============================================================================
echo  LeitoraHU20 - Direct Compilation (WITHOUT CMake)
echo ============================================================================
echo  Directory: %CD%
echo.

REM Check if we are in the correct directory
if not exist "LeitoraHU20.cpp" (
    echo ERROR: LeitoraHU20.cpp not found!
    echo Execute this script in the project root directory.
    exit /b 1
)

REM Try to find Visual Studio 2022
set "VCVARS="
if exist "C:\Program Files\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat" (
    set "VCVARS=C:\Program Files\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
    echo Visual Studio 2022 Build Tools found
    goto :found
)

if exist "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" (
    set "VCVARS=C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
    echo Visual Studio 2022 Community found
    goto :found
)

if exist "C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars64.bat" (
    set "VCVARS=C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars64.bat"
    echo Visual Studio 2022 Professional found
    goto :found
)

REM Try Visual Studio 2019
if exist "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvars64.bat" (
    set "VCVARS=C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
    echo Visual Studio 2019 Build Tools found
    goto :found
)

if exist "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat" (
    set "VCVARS=C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"
    echo Visual Studio 2019 Community found
    goto :found
)

REM Try Visual Studio 18 (Build Tools)
if exist "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build\vcvars64.bat" (
    set "VCVARS=C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
    echo Visual Studio 18 Build Tools found
    goto :found
)

echo.
echo ERROR: Visual Studio not found!
echo.
echo Solution:
echo   1. Install: Visual Studio Build Tools 2022
echo   2. Select: "Desktop development with C++"
echo   3. Restart the computer
echo.
echo Download: https://visualstudio.microsoft.com/downloads/
echo.
exit /b 1

:found
echo.
echo Activating compiler environment...
call "!VCVARS!"

echo.
echo Compiling...
cl.exe /W4 /O2 /EHa /Fe:LeitoraHU20.exe LeitoraHU20.cpp kernel32.lib user32.lib

if errorlevel 1 (
    echo.
    echo ============================================================================
    echo  ERROR: Compilation failed!
    echo ============================================================================
    echo.
    echo For more information about the error, check the output above.
    echo.
    exit /b 1
)

echo.
echo ============================================================================
echo  Checking native DLLs...
echo ============================================================================
echo.

REM Check required DLLs
for %%f in (sgfplib.dll sgbledev.dll sgfdusdax64.dll sgfpamx.dll sgwsqlib.dll) do (
    if exist "%%f" (
        echo   OK: %%f
    ) else (
        echo   WARNING: %%f (not found - will need to copy manually)
    )
)

echo.
echo ============================================================================
echo  COMPILATION COMPLETED SUCCESSFULLY!
echo ============================================================================
echo.
echo Executable: %CD%\LeitoraHU20.exe
echo.
echo Next steps:
echo   1. Make sure all 5 DLLs are in the same directory
echo   2. Run: LeitoraHU20.exe
echo   3. If it works, prepare for distribution as ZIP
echo.
echo Files needed to distribute:
echo   - LeitoraHU20.exe
echo   - sgfplib.dll
echo   - sgbledev.dll
echo   - sgfdusdax64.dll
echo   - sgfpamx.dll
echo   - sgwsqlib.dll
echo.

dir LeitoraHU20.exe

pause
