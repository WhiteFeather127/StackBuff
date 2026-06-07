@echo off
REM 复制 WIC SDK 到项目目录
REM 请根据实际 SDK 位置修改 SOURCE_SDK_DIR

set SOURCE_SDK_DIR="C:\Program Files (x86)\万物互通_R4_260523A\SDK"
set TARGET_SDK_DIR="Source\SDK"

if not exist %SOURCE_SDK_DIR% (
    echo 错误: 找不到 SDK 源目录 %SOURCE_SDK_DIR%
    echo 请修改本脚本中的 SOURCE_SDK_DIR 为正确的 SDK 路径
    pause
    exit /b 1
)

echo 正在从 %SOURCE_SDK_DIR% 复制 SDK 到 %TARGET_SDK_DIR% ...

xcopy /E /I /Y %SOURCE_SDK_DIR%\*.h %TARGET_SDK_DIR%\
xcopy /E /I /Y %SOURCE_SDK_DIR%\*.cpp %TARGET_SDK_DIR%\
xcopy /E /I /Y %SOURCE_SDK_DIR%\ThirdParty\cJSON\* %TARGET_SDK_DIR%\ThirdParty\cJSON\
xcopy /E /I /Y %SOURCE_SDK_DIR%\LICENSE %TARGET_SDK_DIR%\

echo SDK 复制完成！
pause
