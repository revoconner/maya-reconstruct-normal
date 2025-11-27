@echo off
cd /d "%~dp0"

echo ============================================
echo Building Maya Viewport Plugin...
echo ============================================

cd maya_viewport
if not exist build mkdir build
cd build
cmake -G "Visual Studio 17 2022" -A x64 .. --fresh
cmake --build . --config Release
cd ..\..

echo.
echo ============================================
echo Building Arnold Shader...
echo ============================================

cd arnold
if not exist build mkdir build
cd build
cmake -G "Visual Studio 17 2022" -A x64 .. --fresh
cmake --build . --config Release
cd ..\..

echo.
echo ============================================
echo Build complete!
echo ============================================
echo Maya plugin: maya_viewport\output\NormalReconstructZ.mll
echo Arnold shader: arnold\output\aiNormalReconstructZ.dll
echo Arnold metadata: arnold\output\aiNormalReconstructZ.mtd
echo.
echo Run install_plugins.bat to copy to Maya folders.
pause
