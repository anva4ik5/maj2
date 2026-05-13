@echo off
echo === GameCheat Build Script ===
echo.

REM Check for Visual Studio
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if exist "%VSWHERE%" (
    for /f "usebackq tokens=*" %%i in (`"%VSWHERE%" -latest -property installationPath`) do (
        set "VSPATH=%%i"
        echo Found Visual Studio: %%i
    )
)

if "%VSPATH%"=="" (
    echo Visual Studio not found!
    echo Please install Visual Studio Build Tools from:
    echo https://visualstudio.microsoft.com/downloads/
    echo.
    echo Select "Desktop development with C++" during installation
    pause
    exit /b 1
)

echo.
echo === Building Project ===

set "PROJECTDIR=%~dp0"
set "BUILDDIR=%PROJECTDIR%build"

if not exist "%BUILDDIR%" (
    mkdir "%BUILDDIR%"
    echo Created build directory
)

REM Check for CMake
where cmake >nul 2>&1
if %ERRORLEVEL%==0 (
    echo CMake found
    cd /d "%BUILDDIR%"
    cmake ..
    if %ERRORLEVEL%==0 (
        echo CMake configuration successful
        cmake --build . --config Release
        if %ERRORLEVEL%==0 (
            echo Build successful!
            echo Executable: %BUILDDIR%\bin\Release\GameCheat_Restored.exe
        ) else (
            echo Build failed
        )
    ) else (
        echo CMake configuration failed
    )
    cd /d "%PROJECTDIR%"
) else (
    echo CMake not found
    echo Attempting direct build with cl.exe...
    
    set "DEVCMD=%VSPATH%\VC\Auxiliary\Build\vcvars64.bat"
    set "BATCHFILE=%PROJECTDIR%build_temp.bat"
    
    echo @echo off > "%BATCHFILE%"
    echo call "%DEVCMD%" >> "%BATCHFILE%"
    echo cd /d "%PROJECTDIR%" >> "%BATCHFILE%"
    echo cl /EHsc /std:c++17 main.cpp src\*.cpp /link d3d11.lib d3dcompiler.lib wininet.lib ws2_32.lib psapi.lib /OUT:GameCheat_Restored.exe >> "%BATCHFILE%"
    
    echo Starting build...
    cmd /c "%BATCHFILE%"
    
    if exist "%PROJECTDIR%GameCheat_Restored.exe" (
        echo Build successful!
        echo Executable: %PROJECTDIR%GameCheat_Restored.exe
    ) else (
        echo Build failed
    )
    
    del "%BATCHFILE%" >nul 2>&1
)

echo.
echo === Done ===
echo To run, navigate to project directory and run GameCheat_Restored.exe
echo Don't forget to configure config.ini first
pause
