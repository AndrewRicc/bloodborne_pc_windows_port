@echo off
setlocal
cd /d "%~dp0"

set "PYTHON_EXE="
REM python_path.txt is written by the Windows installer with the Python it found.
if exist "python_path.txt" set /p PYTHON_EXE=<python_path.txt
if defined PYTHON_EXE if not exist "%PYTHON_EXE%" set "PYTHON_EXE="
for %%P in (
    "C:\Program Files\Python314\python.exe"
    "C:\Program Files\Python313\python.exe"
    "C:\Program Files\Python312\python.exe"
    "C:\Program Files\Python311\python.exe"
    "C:\Program Files\Python310\python.exe"
    "%LocalAppData%\Programs\Python\Python314\python.exe"
    "%LocalAppData%\Programs\Python\Python313\python.exe"
    "%LocalAppData%\Programs\Python\Python312\python.exe"
    "%LocalAppData%\Programs\Python\Python311\python.exe"
    "%LocalAppData%\Programs\Python\Python310\python.exe"
) do (
    if not defined PYTHON_EXE if exist %%P set "PYTHON_EXE=%%~P"
)

rem Any python.exe on PATH that really runs Python 3.10+. This covers the Microsoft Store install
rem (%LocalAppData%\Microsoft\WindowsApps\python.exe is an app execution alias, not a real file)
rem and rejects the Store's placeholder alias that exists when Python is not installed.
if not defined PYTHON_EXE (
    for /f "delims=" %%I in ('where python.exe 2^>nul') do (
        if not defined PYTHON_EXE (
            "%%~I" -c "import sys; sys.exit(0 if sys.version_info >= (3, 10) else 1)" >nul 2>nul
            if not errorlevel 1 set "PYTHON_EXE=%%~I"
        )
    )
)

if not defined PYTHON_EXE (
    echo [ERROR] Python 3.10+ not found. Install it from python.org or the Microsoft Store.
    pause
    exit /b 1
)

REM Bloodborne.exe (the desktop shortcut) asks for no console window: use pythonw when it exists.
REM Next to the chosen python.exe first; else any pythonw.exe on PATH (the Microsoft Store alias
REM folder has none that "if exist" can see).
set "PYTHONW_EXE="
if "%BB_LAUNCHER_NO_CONSOLE%"=="1" for %%I in ("%PYTHON_EXE%") do if exist "%%~dpIpythonw.exe" set "PYTHONW_EXE=%%~dpIpythonw.exe"
if "%BB_LAUNCHER_NO_CONSOLE%"=="1" if not defined PYTHONW_EXE for /f "delims=" %%I in ('where pythonw.exe 2^>nul') do if not defined PYTHONW_EXE set "PYTHONW_EXE=%%~I"
if defined PYTHONW_EXE set "PYTHON_EXE=%PYTHONW_EXE%"

start "" "%PYTHON_EXE%" launcher.py
