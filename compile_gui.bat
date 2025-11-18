@echo off
REM Script para compilar o jogo com interface gráfica (requer SFML)

echo Compilando jogo de xadrez com interface grafica...
echo.

REM Ajuste estes caminhos conforme sua instalacao do SFML
set SFML_DIR=C:\SFML
set SFML_INCLUDE=-I%SFML_DIR%\include
set SFML_LIBS=-L%SFML_DIR%\lib -lsfml-graphics -lsfml-window -lsfml-system

REM Verificar se SFML existe
if not exist "%SFML_DIR%\include\SFML" (
    echo ERRO: SFML nao encontrado em %SFML_DIR%
    echo.
    echo Por favor:
    echo 1. Baixe SFML de https://www.sfml-dev.org/download.php
    echo 2. Extraia para C:\SFML
    echo 3. Ou ajuste a variavel SFML_DIR neste script
    echo.
    pause
    exit /b 1
)

echo Compilando...
g++ -std=c++17 -O3 -Wall -Wextra -DUSE_SFML %SFML_INCLUDE% -o chess.exe chess.cpp main.cpp chess_gui.cpp %SFML_LIBS% -static-libgcc -static-libstdc++

if %ERRORLEVEL% EQU 0 (
    echo.
    echo Compilacao bem-sucedida!
    echo.
    echo Para executar com interface grafica:
    echo   chess.exe --gui
    echo.
    echo Para executar em modo console:
    echo   chess.exe
    echo.
) else (
    echo.
    echo Erro na compilacao!
    echo Verifique se o SFML esta instalado corretamente.
    echo.
    pause
    exit /b 1
)

pause

