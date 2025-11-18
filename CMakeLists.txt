cmake_minimum_required(VERSION 3.10)
project(ChessGame)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Otimizações para performance máxima
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -flto -DNDEBUG")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")

# Procurar SFML
find_package(SFML 2.5 COMPONENTS system window graphics QUIET)

# Arquivos fonte
set(SOURCES
    chess.cpp
    main.cpp
)

# Se SFML foi encontrado, adicionar GUI
if(SFML_FOUND)
    message(STATUS "SFML encontrado - Interface gráfica habilitada")
    add_definitions(-DUSE_SFML)
    list(APPEND SOURCES chess_gui.cpp)
else()
    message(STATUS "SFML não encontrado - Apenas modo console disponível")
    message(STATUS "Para habilitar GUI, instale SFML e reexecute cmake")
endif()

# Executável
add_executable(chess ${SOURCES})

# Linkar SFML se disponível
if(SFML_FOUND)
    target_link_libraries(chess sfml-graphics sfml-window sfml-system)
endif()

# Para Windows
if(WIN32)
    set_target_properties(chess PROPERTIES
        LINK_FLAGS "-static-libgcc -static-libstdc++"
    )
endif()

# Otimizações específicas do compilador
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(chess PRIVATE -march=native)
endif()

if(MSVC)
    target_compile_options(chess PRIVATE /O2 /GL)
    set_target_properties(chess PROPERTIES
        LINK_FLAGS "/LTCG"
    )
endif()

