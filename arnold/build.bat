@echo off
cd /d "%~dp0"

if not exist build mkdir build
cd build

cmake -G "Visual Studio 17 2022" -A x64 .. --fresh
cmake --build . --config Release

cd ..

echo.
echo Copying to Arnold shaders folder...
copy /Y "output\aiNormalReconstructZ.dll" "C:\Program Files\Autodesk\Arnold\maya2025\shaders\"
copy /Y "output\aiNormalReconstructZ.mtd" "C:\Program Files\Autodesk\Arnold\maya2025\shaders\"

@REM to build for yourself change the maya2025 path to your own version

echo.
echo Build and install complete.
pause
