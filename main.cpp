#include "chess.h"
#ifdef USE_SFML
#include "chess_gui.h"
#endif
#include <iostream>
#include <string>
#include <algorithm>
#include <cctype>

void print_help() {
    std::cout << "\n=== AJUDA DO JOGO DE XADREZ ===\n";
    std::cout << "Comandos disponíveis:\n";
    std::cout << "  <movimento>  - Faça um movimento (ex: e2e4, e7e5)\n";
    std::cout << "  undo         - Desfazer último movimento\n";
    std::cout << "  fen          - Mostrar posição em notação FEN\n";
    std::cout << "  help         - Mostrar esta ajuda\n";
    std::cout << "  quit         - Sair do jogo\n";
    std::cout << "\nNotação de movimentos:\n";
    std::cout << "  Formato: <origem><destino>[promoção]\n";
    std::cout << "  Exemplos: e2e4, e7e5, g1f3, e1g1 (roque)\n";
    std::cout << "  Promoção: e7e8q (peão promove a dama)\n";
    std::cout << "  Peças de promoção: n (cavalo), b (bispo), r (torre), q (dama)\n\n";
}

void print_game_status(const ChessBoard& board) {
    Color side = board.get_side_to_move();
    std::cout << "\n=== JOGO DE XADREZ ===\n";
    std::cout << "Vez de: " << (side == WHITE ? "BRANCAS" : "PRETAS") << "\n";
    std::cout << "Jogada: " << board.get_fullmove_number() << "\n";
    
    if (board.is_check(side)) {
        std::cout << "⚠ XEQUE!\n";
    }
    
    if (board.is_checkmate(side)) {
        std::cout << "\n*** XEQUE-MATE! ***\n";
        std::cout << "Vencedor: " << (side == WHITE ? "PRETAS" : "BRANCAS") << "\n";
    } else if (board.is_stalemate(side)) {
        std::cout << "\n*** EMPATE (AFOGAMENTO) ***\n";
    }
}

void print_legal_moves(const ChessBoard& board) {
    std::vector<Move> moves = board.generate_legal_moves();
    std::cout << "\nMovimentos legais (" << moves.size() << "):\n";
    
    if (moves.size() > 0) {
        int count = 0;
        for (const auto& move : moves) {
            std::cout << move.to_string() << " ";
            count++;
            if (count % 10 == 0) std::cout << "\n";
        }
        if (count % 10 != 0) std::cout << "\n";
    }
    std::cout << "\n";
}

int main(int argc, char* argv[]) {
    // Verificar se deve usar interface gráfica
    bool use_gui = false;
    if (argc > 1) {
        std::string arg = argv[1];
        if (arg == "--gui" || arg == "-g" || arg == "gui") {
            use_gui = true;
        }
    }
    
#ifdef USE_SFML
    if (use_gui) {
        try {
            ChessGUI gui;
            gui.run();
            return 0;
        } catch (const std::exception& e) {
            std::cerr << "Erro ao iniciar interface gráfica: " << e.what() << std::endl;
            std::cerr << "Iniciando modo console..." << std::endl;
        }
    }
#endif
    
    if (use_gui) {
        std::cout << "Interface gráfica não disponível. Compile com SFML habilitado.\n";
        std::cout << "Usando modo console...\n\n";
    }
    
    std::cout << "=== JOGO DE XADREZ EM C++ ===\n";
    std::cout << "Digite 'help' para ver os comandos disponíveis\n";
    std::cout << "Use --gui para interface gráfica (se compilado com SFML)\n\n";
    
    ChessBoard board;
    std::string input;
    
    while (true) {
        board.print_board();
        print_game_status(board);
        
        if (board.is_game_over()) {
            std::cout << "\nJogo terminado! Digite 'quit' para sair.\n";
        } else {
            print_legal_moves(board);
        }
        
        std::cout << "\n> ";
        std::getline(std::cin, input);
        
        // Converter para minúsculas
        std::transform(input.begin(), input.end(), input.begin(), ::tolower);
        
        // Remover espaços
        input.erase(std::remove_if(input.begin(), input.end(), ::isspace), input.end());
        
        if (input == "quit" || input == "q" || input == "exit") {
            std::cout << "Obrigado por jogar!\n";
            break;
        } else if (input == "help" || input == "h") {
            print_help();
        } else if (input == "undo" || input == "u") {
            if (board.get_fullmove_number() > 1 || board.get_side_to_move() == BLACK) {
                board.unmake_move();
                std::cout << "Movimento desfeito.\n";
            } else {
                std::cout << "Não há movimentos para desfazer.\n";
            }
        } else if (input == "fen") {
            std::cout << "\nFEN: " << board.to_fen() << "\n\n";
        } else if (input.length() >= 4) {
            // Tentar fazer um movimento
            Move move = Move::from_string(input);
            
            if (move.from == NO_SQUARE || move.to == NO_SQUARE) {
                std::cout << "Movimento inválido! Use o formato: e2e4\n";
                continue;
            }
            
            if (board.make_move(move)) {
                std::cout << "Movimento executado: " << move.to_string() << "\n";
            } else {
                std::cout << "Movimento ilegal! Tente novamente.\n";
            }
        } else {
            std::cout << "Comando não reconhecido. Digite 'help' para ajuda.\n";
        }
    }
    
    return 0;
}

