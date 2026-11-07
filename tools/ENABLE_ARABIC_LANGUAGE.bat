@echo off
chcp 65001 >nul
title Mineclonia Arabic - Enable language option
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0ENABLE_ARABIC_LANGUAGE.ps1"
pause
