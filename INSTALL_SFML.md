# Instalação do SFML para Interface Gráfica

Para usar a interface gráfica do jogo de xadrez, você precisa instalar a biblioteca SFML.

## Windows

### Opção 1: Usando vcpkg (Recomendado)
```bash
# Instalar vcpkg (se ainda não tiver)
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat

# Instalar SFML
.\vcpkg install sfml:x64-windows
```

### Opção 2: Download Manual
1. Baixe SFML de: https://www.sfml-dev.org/download.php
2. Extraia para `C:\SFML`
3. Configure o Makefile com os caminhos corretos:
   ```makefile
   SFML_INCLUDE = -IC:/SFML/include
   SFML_LIBS = -LC:/SFML/lib -lsfml-graphics -lsfml-window -lsfml-system
   ```

### Opção 3: Usando MSYS2
```bash
pacman -S mingw-w64-x86_64-sfml
```

## Linux (Ubuntu/Debian)
```bash
sudo apt-get update
sudo apt-get install libsfml-dev
```

## Linux (Fedora)
```bash
sudo dnf install SFML-devel
```

## macOS
```bash
# Usando Homebrew
brew install sfml
```

## Compilação

### Com Makefile
```bash
# Compilar com GUI (requer SFML)
make gui

# Ou compilar manualmente
g++ -std=c++17 -O3 -DUSE_SFML -IC:/SFML/include -o chess.exe chess.cpp main.cpp chess_gui.cpp -LC:/SFML/lib -lsfml-graphics -lsfml-window -lsfml-system
```

### Com CMake
```bash
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

## Execução

```bash
# Modo gráfico
./chess --gui

# Modo console (sempre disponível)
./chess
```

## Notas

- Se SFML não estiver instalado, o jogo ainda funcionará no modo console
- A interface gráfica é opcional e não é necessária para jogar
- No Windows, certifique-se de copiar as DLLs do SFML para o mesmo diretório do executável

