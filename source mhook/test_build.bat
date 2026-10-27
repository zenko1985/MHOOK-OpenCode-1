@echo off
echo Starting g++ test...
C:\Programs\mingw64\bin\g++.exe --version
echo Compiling test.c...
C:\Programs\mingw64\bin\g++.exe -c test.c -o test_final.o
if errorlevel 1 (
    echo COMPILATION FAILED
) else (
    echo COMPILATION SUCCEEDED
)
if exist test_final.o (
    echo FILE EXISTS
) else (
    echo FILE MISSING
)
