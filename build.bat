@echo off
set "SLN=%~dp0MHook64.sln"
set "MSBUILD=C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\amd64\MSBuild.exe"
"%MSBUILD%" "%SLN%" /p:Configuration=Release /p:Platform=x64 /verbosity:minimal
"%MSBUILD%" "%SLN%" /p:Configuration=Release_Win7 /p:Platform=x64 /verbosity:minimal
REM UPX disabled - it destroys CFG/CET security metadata. Uncomment below only if needed:
REM "%~dp0upx_temp\upx-4.2.4-win64\upx.exe" -9 "%~dp0x64\Release\MHook64.exe"
REM "%~dp0upx_temp\upx-4.2.4-win64\upx.exe" -9 "%~dp0x64\Release_Win7\MHook64_Win7.exe"
