@echo off
rem SeriousSpeedrun - speedrun timer, LiveSplit and speedometer for Serious Sam Classic: double-click to install
rem (or drop the game's folder on this file). Finds TFE/TSE in Steam or GOG, adds Classics Patch 1.9.4 if missing.
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0install.ps1" %*
pause
