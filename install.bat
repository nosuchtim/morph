@echo off
set build=vs2019\build\morph\x64\Release
set bin=..\viztopia\bin
copy /Y %build%\morph.exe %bin%
copy /Y %build%\LibSensel*.dll %bin%
