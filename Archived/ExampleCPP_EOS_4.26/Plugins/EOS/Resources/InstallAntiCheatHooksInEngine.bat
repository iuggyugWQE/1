@echo off
powershell -ExecutionPolicy Bypass -File "%~dp0\InstallAntiCheatHooksInEngine.ps1" -ProjectDir %1
exit %errorlevel%