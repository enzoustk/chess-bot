// Exemplo de uso da biblioteca de xadrez
// Este arquivo demonstra como usar a classe ChessBoard programaticamente

#include "chess.h"
#include <iostream>

int main() {
    // Criar um tabuleiro com a posição inicial
    ChessBoard board;
    
    std::cout << "=== Exemplo de Uso da Biblioteca de Xadrez ===\n\n";
    
    // Mostrar tabuleiro inicial
    std::cout << "Posição inicial:\n";
    board.print_board();
    std::cout << "FEN: " << board.to_fen() << "\n\n";
    
    // Fazer alguns movimentos
    std::cout << "Fazendo movimentos...\n";
    
    Move move1 = Move::from_string("e2e4");
    if (board.make_move(move1)) {
        std::cout << "Movimento 1: " << move1.to_string() << "\n";
        board.print_board();
    }
    
    Move move2 = Move::from_string("e7e5");
    if (board.make_move(move2)) {
        std::cout << "Movimento 2: " << move2.to_string() << "\n";
        board.print_board();
    }
    
    Move move3 = Move::from_string("g1f3");
    if (board.make_move(move3)) {
        std::cout << "Movimento 3: " << move3.to_string() << "\n";
        board.print_board();
    }
    
    // Mostrar movimentos legais disponíveis
    std::vector<Move> legal_moves = board.generate_legal_moves();
    std::cout << "\nMovimentos legais disponíveis (" << legal_moves.size() << "):\n";
    for (const auto& move : legal_moves) {
        std::cout << move.to_string() << " ";
    }
    std::cout << "\n\n";
    
    // Verificar estado do jogo
    std::cout << "Estado do jogo:\n";
    std::cout << "  Xeque: " << (board.is_check(board.get_side_to_move()) ? "Sim" : "Não") << "\n";
    std::cout << "  Xeque-mate: " << (board.is_checkmate(board.get_side_to_move()) ? "Sim" : "Não") << "\n";
    std::cout << "  Afogamento: " << (board.is_stalemate(board.get_side_to_move()) ? "Sim" : "Não") << "\n";
    std::cout << "  Jogo terminado: " << (board.is_game_over() ? "Sim" : "Não") << "\n\n";
    
    // Desfazer último movimento
    std::cout << "Desfazendo último movimento...\n";
    board.unmake_move();
    board.print_board();
    
    // Criar tabuleiro a partir de FEN
    std::cout << "\nCriando tabuleiro a partir de FEN...\n";
    std::string fen = "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1";
    ChessBoard board_from_fen(fen);
    board_from_fen.print_board();
    std::cout << "FEN: " << board_from_fen.to_fen() << "\n";
    
    return 0;
}

