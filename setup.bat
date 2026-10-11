@echo off
rem Full setup: checks the requirements, installs what is missing and builds. See GUIDE.md.
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0scripts\windows\setup.ps1" %*
exit /b %ERRORLEVEL%
