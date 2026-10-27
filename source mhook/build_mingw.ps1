$MINGW = "C:\Programs\mingw64\bin"
$ROOT = "C:\mhook_build"
$OUT = "$ROOT\x64\Release"

if (-not (Test-Path $OUT)) { New-Item -ItemType Directory -Path $OUT -Force | Out-Null }

# Compile resources (without .ahk RCDATA entries that windres can't find)
& "$MINGW\windres.exe" -i "$ROOT\resource_nohk.rc" -o "$OUT\resource.o" --define UNICODE --define _UNICODE -J rc -I"$ROOT"
if ($LASTEXITCODE -ne 0) { throw "Resource compilation failed" }

# Build
& "$MINGW\g++.exe" `
    -O2 -s -m64 -mwindows `
    -DUNICODE -D_UNICODE `
    -DMINIZ_NO_ARCHIVE_APIS -DMINIZ_NO_STDIO `
    -I"$ROOT" -I"$ROOT\MHook64" `
    "$ROOT\Bitmap.cpp" `
    "$ROOT\CursorDot.cpp" `
    "$ROOT\EmbeddedExe.cpp" `
    "$ROOT\Gamepad.cpp" `
    "$ROOT\hh1.cpp" `
    "$ROOT\hh1a.cpp" `
    "$ROOT\hh2.cpp" `
    "$ROOT\hh3.cpp" `
    "$ROOT\hh4.cpp" `
    "$ROOT\hh5.cpp" `
    "$ROOT\hh6.cpp" `
    "$ROOT\HookHandler.cpp" `
    "$ROOT\HookProc.cpp" `
    "$ROOT\MagicWindow.cpp" `
    "$ROOT\MH001.cpp" `
    "$ROOT\MH002.cpp" `
    "$ROOT\MHKeypad.cpp" `
    "$ROOT\MHRepErr.cpp" `
    "$ROOT\MVector.cpp" `
    "$ROOT\Localization.cpp" `
    "$ROOT\RecentFiles.cpp" `
    "$ROOT\Settings.cpp" `
    "$ROOT\Settings2.cpp" `
    "$ROOT\miniz.c" `
    "$OUT\resource.o" `
    -lwinmm -lws2_32 `
    -o "$OUT\MHook64.exe"

if ($LASTEXITCODE -ne 0) { throw "Build failed" }

Write-Host "Done: $OUT\MHook64.exe"
