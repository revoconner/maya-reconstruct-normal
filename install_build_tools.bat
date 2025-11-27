@echo off
echo Installing Visual Studio 2022 Build Tools...
echo This is required to compile Maya plugins and Arnold shaders.
echo.

winget install Microsoft.VisualStudio.2022.BuildTools --override "--wait --passive --add Microsoft.VisualStudio.Workload.VCTools --includeRecommended"

echo.
echo Installation complete.
echo You may need to restart your terminal for changes to take effect.
pause
