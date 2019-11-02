@echo off
mkdir Win64Build
cd Win64Build
cmake -G "Visual Studio 16 2019" -A x64 ../
pause