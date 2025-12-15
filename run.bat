@echo off
:: Tenta rodar o comando 'run' do seu makefile
:: Se você instalou o MinGW, o comando geralmente é mingw32-make ou apenas make
make run

:: Se der erro (compilação falhou), pausa para você ler o erro
if %errorlevel% neq 0 (
    echo.
    echo Ocorreu um erro na compilacao!
    pause
)