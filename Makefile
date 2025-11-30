# Makefile para o Jogo de Xadrez em C++

CXX = g++
CXXFLAGS = -std=c++17 -O3 -Wall -Wextra -march=native -flto

# Detectar se SFML está disponível
SFML_AVAILABLE = $(shell pkg-config --exists sfml-all 2>/dev/null && echo "yes" || echo "no")

# Para Windows (MinGW/MSVC)
ifeq ($(OS),Windows_NT)
    CXXFLAGS += -static-libgcc -static-libstdc++
    TARGET = chess.exe
    # Tentar encontrar SFML no Windows (ajuste os caminhos conforme necessário)
    SFML_INCLUDE = -IC:/SFML/include
    SFML_LIBS = -LC:/SFML/lib -lsfml-graphics -lsfml-window -lsfml-system
    # Se SFML não estiver no caminho padrão, descomente e ajuste:
    # SFML_AVAILABLE = yes
    # CXXFLAGS += $(SFML_INCLUDE)
    # LDFLAGS += $(SFML_LIBS)
else
    TARGET = chess
    ifeq ($(SFML_AVAILABLE),yes)
        CXXFLAGS += $(shell pkg-config --cflags sfml-all)
        LDFLAGS += $(shell pkg-config --libs sfml-all)
    endif
endif

SRCDIR = .
SOURCES = chess.cpp main.cpp
GUI_SOURCES = chess_gui.cpp

# Se SFML estiver disponível, incluir GUI
ifeq ($(SFML_AVAILABLE),yes)
    SOURCES += $(GUI_SOURCES)
    CXXFLAGS += -DUSE_SFML
endif

OBJECTS = $(SOURCES:.cpp=.o)

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJECTS) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET) chess.exe

run: $(TARGET)
	./$(TARGET)

# Compilação rápida para debug
debug: CXXFLAGS = -std=c++17 -g -Wall -Wextra -DDEBUG
debug: $(TARGET)

# Compilação com otimizações máximas
release: CXXFLAGS = -std=c++17 -O3 -Wall -Wextra -march=native -flto -DNDEBUG
release: clean $(TARGET)

# Compilação com interface gráfica (requer SFML)
gui: CXXFLAGS += -DUSE_SFML
gui: LDFLAGS += -lsfml-graphics -lsfml-window -lsfml-system
# CHANGE 1: Add chess_engine.o to the dependencies line below
gui: chess_gui.o chess.o main.o chess_engine.o
# CHANGE 2: Add chess_engine.o to the compile command line below
	$(CXX) $(CXXFLAGS) -o $(TARGET) chess.o main.o chess_gui.o chess_engine.o $(LDFLAGS)
	@echo "Compilado com suporte a interface gráfica!"
	@echo "Execute com: ./$(TARGET) --gui"

# ========================================
# Compilação do executável UCI para Lichess
# ========================================

UCI_TARGET = chess_uci
UCI_SOURCES = lichess/uci_main.cpp lichess/uci_interface.cpp chess.cpp chess_engine.cpp
UCI_OBJECTS = lichess/uci_main.o lichess/uci_interface.o chess.o chess_engine.o

# Compilar executável UCI (sem SFML)
$(UCI_TARGET): lichess/uci_main.o lichess/uci_interface.o chess.o chess_engine.o
	$(CXX) $(CXXFLAGS) -o $(UCI_TARGET) lichess/uci_main.o lichess/uci_interface.o chess.o chess_engine.o
	@echo "Executável UCI compilado com sucesso!"
	@echo "Teste com: echo -e 'uci\nisready\nposition startpos\ngo depth 5\nquit' | ./$(UCI_TARGET)"

# Target para compilar apenas o UCI
uci: $(UCI_TARGET)

# Limpar também o UCI
clean:
	rm -f $(OBJECTS) $(UCI_OBJECTS) $(TARGET) $(UCI_TARGET) chess.exe chess_uci.exe

