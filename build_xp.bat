@echo off
set "SLN=%~dp0MHook64.sln"
set "MSBUILD=C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\amd64\MSBuild.exe"
"%MSBUILD%" "%SLN%" /p:Configuration=Release_XP /p:Platform=x64 /verbosity:minimal
