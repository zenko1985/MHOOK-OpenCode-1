@echo off
chcp 65001 >nul
setlocal enabledelayedexpansion

set "ROOT=C:\Programs\mhook\source mhook"
set "MINGW=C:\Programs\mingw64\bin"
set "OUT=%ROOT%\x64\Release"

if not exist "%OUT%" mkdir "%OUT%"

:: Компиляция ресурсов (resource.rc в UTF-16 LE)
"%MINGW%\windres.exe" -i "%ROOT%\resource.rc" -o "%OUT%\resource.o" --define UNICODE --define _UNICODE -J rc

if errorlevel 1 (
    echo ОШИБКА: компиляция ресурсов
    pause
    exit /b 1
)

:: Сборка
"%MINGW%\g++.exe" -O2 -s -m64 -mwindows -DUNICODE -D_UNICODE -DMINIZ_NO_ARCHIVE_APIS -DMINIZ_NO_STDIO ^
    -I"%ROOT%" -I"%ROOT%\MHook64" ^
    "%ROOT%\Bitmap.cpp" ^
    "%ROOT%\CursorDot.cpp" ^
    "%ROOT%\EmbeddedExe.cpp" ^
    "%ROOT%\Gamepad.cpp" ^
    "%ROOT%\hh1.cpp" ^
    "%ROOT%\hh1a.cpp" ^
    "%ROOT%\hh2.cpp" ^
    "%ROOT%\hh3.cpp" ^
    "%ROOT%\hh4.cpp" ^
    "%ROOT%\hh5.cpp" ^
    "%ROOT%\hh6.cpp" ^
    "%ROOT%\HookHandler.cpp" ^
    "%ROOT%\HookProc.cpp" ^
    "%ROOT%\MagicWindow.cpp" ^
    "%ROOT%\MH001.cpp" ^
    "%ROOT%\MH002.cpp" ^
    "%ROOT%\MHKeypad.cpp" ^
    "%ROOT%\MHRepErr.cpp" ^
    "%ROOT%\MVector.cpp" ^
    "%ROOT%\Localization.cpp" ^
    "%ROOT%\RecentFiles.cpp" ^
    "%ROOT%\Settings.cpp" ^
    "%ROOT%\Settings2.cpp" ^
    "%ROOT%\miniz.c" ^
    "%OUT%\resource.o" ^
    -lwinmm -lws2_32 ^
    -o "%OUT%\MHook64.exe"

if errorlevel 1 (
    echo ОШИБКА: сборка
    pause
    exit /b 1
)

echo Готово: %OUT%\MHook64.exe
pause
