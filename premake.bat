@echo off
Tools\premake5.exe --file=enchantedforest_premake.lua vs2015
if %ERRORLEVEL% NEQ 0 PAUSE