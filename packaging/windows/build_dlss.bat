@echo off
rem Builds DLSS for this installed copy of the port (NVIDIA RTX cards). See README_RELEASE.txt.
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0scripts\windows\build_dlss.ps1" -Pause %*
if not errorlevel 1 pause
