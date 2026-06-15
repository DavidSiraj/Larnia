@echo off
chcp 65001 >nul
echo.
echo Larnia - generovanie Poppins fontov pre LVGL
echo.
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0tools\generate_poppins_lvgl.ps1"
echo.
pause
