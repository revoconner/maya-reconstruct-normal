@echo off
cd /d "%~dp0"

if not exist build mkdir build
cd build

cmake -G "Visual Studio 17 2022" -A x64 .. --fresh
cmake --build . --config Release

cd ..

echo.
echo Build complete.
echo Output: output\NormalReconstructZ.mll
pause
