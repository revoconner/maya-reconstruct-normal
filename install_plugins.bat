@echo off
cd /d "%~dp0"

echo ============================================
echo Normal Reconstruct Z - Plugin Installer
echo ============================================
echo.
echo If your Maya version is 2025.3, enter: 2025
echo.
set /p MAYA_VER="Enter your Maya version (e.g. 2025): "

set MAYA_PLUGIN_PATH=C:\Program Files\Autodesk\Maya%MAYA_VER%\bin\plug-ins
set ARNOLD_SHADER_PATH=C:\Program Files\Autodesk\Arnold\maya%MAYA_VER%\shaders

echo.
echo Installing to:
echo   Maya: %MAYA_PLUGIN_PATH%
echo   Arnold: %ARNOLD_SHADER_PATH%
echo.

:: Check if Maya path exists
if not exist "%MAYA_PLUGIN_PATH%" (
    echo ERROR: Maya plugin path not found: %MAYA_PLUGIN_PATH%
    echo Please check your Maya version.
    pause
    exit /b 1
)

:: Check if Arnold path exists
if not exist "%ARNOLD_SHADER_PATH%" (
    echo WARNING: Arnold shader path not found: %ARNOLD_SHADER_PATH%
    echo Arnold shader will not be installed.
    echo.
)

:: Copy Maya viewport plugin
echo Copying Maya viewport plugin...
copy /Y "maya_viewport\output\NormalReconstructZ.mll" "%MAYA_PLUGIN_PATH%\"
if %errorlevel% neq 0 (
    echo ERROR: Failed to copy Maya plugin. Run as Administrator.
    pause
    exit /b 1
)

:: Copy Arnold shader if path exists
if exist "%ARNOLD_SHADER_PATH%" (
    echo Copying Arnold shader...
    copy /Y "arnold\output\aiNormalReconstructZ.dll" "%ARNOLD_SHADER_PATH%\"
    copy /Y "arnold\output\aiNormalReconstructZ.mtd" "%ARNOLD_SHADER_PATH%\"
    if %errorlevel% neq 0 (
        echo ERROR: Failed to copy Arnold shader. Run as Administrator.
        pause
        exit /b 1
    )
)

echo.
echo ============================================
echo Installation complete!
echo ============================================
echo Restart Maya to load the plugins.
pause
