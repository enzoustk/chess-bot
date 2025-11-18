@echo off
echo Testando compilacao...
g++ -std=c++17 -O3 -Wall -Wextra -o chess.exe chess.cpp main.cpp
if %ERRORLEVEL% EQU 0 (
    echo Compilacao bem-sucedida!
    echo Executavel criado: chess.exe
) else (
    echo Erro na compilacao!
    exit /b 1
)

