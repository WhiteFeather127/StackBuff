@echo off
cd /d "%~dp0"

echo ========================================
echo StackBuffs Build Script
echo ========================================

set VSWHERE="%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist %VSWHERE% (
    echo Error: vswhere not found
    pause
    exit /b 1
)

for /f "usebackq tokens=*" %%i in (`%VSWHERE% -latest -products * -requires Microsoft.Component.MSBuild -property installationPath`) do (
    set VSROOT=%%i
)

if not defined VSROOT (
    echo Error: Visual Studio not found
    pause
    exit /b 1
)

echo Visual Studio: %VSROOT%

call "%VSROOT%\Common7\Tools\VsDevCmd.bat" -arch=x86

echo.
echo Building Release/Win32...
msbuild StackBuffs.vcxproj "/p:Configuration=Release" "/p:Platform=Win32" "/t:Build" "/v:minimal"

if %ERRORLEVEL% EQU 0 (
    echo.
    echo Build SUCCEEDED!
) else (
    echo.
    echo Build FAILED with error code %ERRORLEVEL%
)

pause
