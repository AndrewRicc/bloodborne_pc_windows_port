@echo off
rem Setup completo: controlla i requisiti, installa quello che manca e compila. Vedi GUIDA.md.
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0scripts\windows\setup.ps1" %*
exit /b %ERRORLEVEL%
