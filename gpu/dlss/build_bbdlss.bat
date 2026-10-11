@echo off
rem bbport: builds out\bbdlss.dll (MSVC) and copies nvngx_dlss.dll next to bbport.exe.
rem   DLSS_SDK  NVIDIA DLSS SDK checkout (github.com/NVIDIA/DLSS), default modules\dlss-sdk,
rem             else ..\dlss-sdk
rem   VK_INC    folder holding vulkan\vulkan.h and vk_video\, default C:\msys64\mingw64\include
rem             (only those two folders are copied, the MinGW CRT headers are not used)
setlocal enabledelayedexpansion
cd /d "%~dp0..\.."
if not defined DLSS_SDK if exist "%CD%\modules\dlss-sdk\include\nvsdk_ngx_vk.h" set "DLSS_SDK=%CD%\modules\dlss-sdk"
if not defined DLSS_SDK set "DLSS_SDK=%CD%\..\dlss-sdk"
if not defined VK_INC set "VK_INC=C:\msys64\mingw64\include"
if not exist "%DLSS_SDK%\include\nvsdk_ngx_vk.h" (
    echo [ERROR] DLSS SDK not found in %DLSS_SDK%
    exit /b 1
)
rem No parenthesized blocks below: "(x86)" in the expanded path would close them.
if defined VCINSTALLDIR goto have_msvc
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%VSWHERE%" goto no_msvc
for /f "usebackq delims=" %%i in (`""!VSWHERE!" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath"`) do set "VSDIR=%%i"
if not defined VSDIR goto no_msvc
call "%VSDIR%\VC\Auxiliary\Build\vcvars64.bat" >nul
goto have_msvc
:no_msvc
echo [ERROR] Visual Studio with C++ tools not found
exit /b 1
:have_msvc
set "OBJ=out\dlss"
if not exist "%OBJ%\include" mkdir "%OBJ%\include"
xcopy /e /i /q /y "%VK_INC%\vulkan" "%OBJ%\include\vulkan" >nul
xcopy /e /i /q /y "%VK_INC%\vk_video" "%OBJ%\include\vk_video" >nul
cl /nologo /O2 /MT /EHsc /std:c++17 /W3 /LD /DNOMINMAX /D_CRT_SECURE_NO_WARNINGS /DVK_USE_PLATFORM_WIN32_KHR ^
   /I"%OBJ%\include" /I"%DLSS_SDK%\include" /Fo"%OBJ%\\" /Fe"out\bbdlss.dll" ^
   gpu\dlss\bbdlss.cpp ^
   /link /NOLOGO "%DLSS_SDK%\lib\Windows_x86_64\x64\nvsdk_ngx_s.lib" advapi32.lib user32.lib version.lib
if errorlevel 1 exit /b 1
copy /y "%DLSS_SDK%\lib\Windows_x86_64\rel\nvngx_dlss.dll" out\ >nul
echo Built out\bbdlss.dll (+ nvngx_dlss.dll)
