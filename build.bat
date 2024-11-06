cd build
cmake -G "Visual Studio 17 2022" -A x64 .. --fresh
pause
cmake --build . --config Release
pause