cd build
cmake -G "Visual Studio 17 2022" -A x64 .. --fresh
cmake --build . --config Release
cd ..
pause