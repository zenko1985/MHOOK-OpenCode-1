@echo off
set MINGW=C:\Programs\mingw64\bin
set ROOT=C:\mhook_build
set OUT=%ROOT%\x64\Release

if not exist "%OUT%" mkdir "%OUT%"

:: Compile all source files to object files
for %%f in (
    Bitmap.cpp CursorDot.cpp EmbeddedExe.cpp Gamepad.cpp
    hh1.cpp hh1a.cpp hh2.cpp hh3.cpp hh4.cpp hh5.cpp hh6.cpp
    HookHandler.cpp HookProc.cpp MagicWindow.cpp
    MH001.cpp MH002.cpp MHKeypad.cpp MHRepErr.cpp MVector.cpp
    Localization.cpp RecentFiles.cpp Settings.cpp Settings2.cpp
    miniz.c
) do (
    echo Compiling %%f...
    %MINGW%\g++.exe -B %MINGW% -O2 -s -m64 -mwindows -static-libgcc -static-libstdc++ -DUNICODE -D_UNICODE -DMINIZ_NO_ARCHIVE_APIS -DMINIZ_NO_STDIO -w -I%ROOT% -I%ROOT%\MHook64 -c %ROOT%\%%f -o %OUT%\%%~nf.o
    if errorlevel 1 (
        echo ERROR compiling %%f
        pause
        exit /b 1
    )
)

:: Compile resources
echo Compiling resources...
%MINGW%\windres.exe -i %ROOT%\resource_nohk.rc -o %OUT%\resource.o --define UNICODE --define _UNICODE -J rc -I%ROOT%
if errorlevel 1 (
    echo ERROR compiling resources
    pause
    exit /b 1
)

:: Link
echo Linking...
%MINGW%\g++.exe -B %MINGW% -O2 -s -m64 -mwindows -static-libgcc -static-libstdc++ -w -o %OUT%\MHook64.exe %OUT%\Bitmap.o %OUT%\CursorDot.o %OUT%\EmbeddedExe.o %OUT%\Gamepad.o %OUT%\hh1.o %OUT%\hh1a.o %OUT%\hh2.o %OUT%\hh3.o %OUT%\hh4.o %OUT%\hh5.o %OUT%\hh6.o %OUT%\HookHandler.o %OUT%\HookProc.o %OUT%\MagicWindow.o %OUT%\MH001.o %OUT%\MH002.o %OUT%\MHKeypad.o %OUT%\MHRepErr.o %OUT%\MVector.o %OUT%\Localization.o %OUT%\RecentFiles.o %OUT%\Settings.o %OUT%\Settings2.o %OUT%\miniz.o %OUT%\resource.o -lwinmm -lws2_32 -lshlwapi
if errorlevel 1 (
    echo ERROR linking
    pause
    exit /b 1
)

echo Done: %OUT%\MHook64.exe
pause
