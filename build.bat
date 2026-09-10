@echo off
"C:\Program Files\Microsoft Visual Studio\18\Insiders\MSBuild\Current\Bin\MSBuild.exe" "C:\Programs\mhook\source mhook\MHook64.sln" /p:Configuration=Release /p:Platform=x64 /verbosity:minimal
REM UPX disabled - it destroys CFG/CET security metadata. Uncomment below only if needed:
REM "C:\Programs\mhook\source mhook\upx_temp\upx-4.2.4-win64\upx.exe" -9 "C:\Programs\mhook\source mhook\x64\Release\MHook64.exe"
