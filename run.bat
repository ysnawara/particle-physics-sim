@echo off
echo ============================================
echo   Particle Physics Simulator - Build Script
echo ============================================
echo.

:: Configure if build directory doesn't exist
if not exist "build" (
    echo [1/3] Configuring CMake...
    cmake -B build -G "Visual Studio 17 2022" -A x64
    if %ERRORLEVEL% neq 0 (
        echo ERROR: CMake configuration failed!
        pause
        exit /b 1
    )
) else (
    echo [1/3] Build directory exists, skipping configure.
)

:: Build
echo [2/3] Building Release...
cmake --build build --config Release
if %ERRORLEVEL% neq 0 (
    echo ERROR: Build failed!
    pause
    exit /b 1
)

:: Run
echo [3/3] Launching ParticleSim...
echo.
start "" "build\Release\ParticleSim.exe"
