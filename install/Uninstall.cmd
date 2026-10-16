@echo off
rem SeriousSpeedrun: removes the plugin (Bin\Plugins\SeriousSpeedrun.dll) and its desktop shortcuts; Classics Patch stays.
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0install.ps1" -Uninstall %*
pause
