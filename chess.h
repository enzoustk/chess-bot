#ifndef CHESS_H
#define CHESS_H

#include <cstdint>
#include <string>
#include <vector>
#include <array>

// Tipos básicos
using Bitboard = uint64_t;
using Square = int;

// Constantes de cores
enum Color : int {
    WHITE = 0,
    BLACK = 1
};

// Constantes de peças
enum PieceType : int {
    PAWN = 0,
    KNIGHT = 1,
    BISHOP = 2,
    ROOK = 3,
    QUEEN = 4,
    KING = 5,
    NONE = 6
};

// Constantes de casas
enum SquareEnum : Square {
    A1 = 0, B1 = 1, C1 = 2, D1 = 3, E1 = 4, F1 = 5, G1 = 6, H1 = 7,
    A2 = 8, B2 = 9, C2 = 10, D2 = 11, E2 = 12, F2 = 13, G2 = 14, H2 = 15,
    A3 = 16, B3 = 17, C3 = 18, D3 = 19, E3 = 20, F3 = 21, G3 = 22, H3 = 23,
    A4 = 24, B4 = 25, C4 = 26, D4 = 27, E4 = 28, F4 = 29, G4 = 30, H4 = 31,
    A5 = 32, B5 = 33, C5 = 34, D5 = 35, E5 = 36, F5 = 37, G5 = 38, H5 = 39,
    A6 = 40, B6 = 41, C6 = 42, D6 = 43, E6 = 44, F6 = 45, G6 = 46, H6 = 47,
    A7 = 48, B7 = 49, C7 = 50, D7 = 51, E7 = 52, F7 = 53, G7 = 54, H7 = 55,
    A8 = 56, B8 = 57, C8 = 58, D8 = 59, E8 = 60, F8 = 61, G8 = 62, H8 = 63,
    NO_SQUARE = 64
};

// Estrutura para representar um movimento
struct Move {
    Square from;
    Square to;
    PieceType promotion;
    bool is_castle;
    bool is_en_passant;
    PieceType captured_piece;
    
    Move() : from(NO_SQUARE), to(NO_SQUARE), promotion(NONE), 
             is_castle(false), is_en_passant(false), captured_piece(NONE) {}
    
    Move(Square f, Square t, PieceType p = NONE) 
        : from(f), to(t), promotion(p), is_castle(false), 
          is_en_passant(false), captured_piece(NONE) {}
    
    bool operator==(const Move& other) const {
        return from == other.from && to == other.to && promotion == other.promotion;
    }
    
    std::string to_string() const;
    static Move from_string(const std::string& move_str);
};

// Classe principal do tabuleiro
class ChessBoard {
private:
    // Bitboards para cada tipo de peça e cor
    std::array<Bitboard, 6> pieces_white;  // [PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING]
    std::array<Bitboard, 6> pieces_black;
    
    // Bitboards combinados
    Bitboard all_white;
    Bitboard all_black;
    Bitboard all_pieces;
    
    // Estado do jogo
    Color side_to_move;
    Square en_passant_square;
    bool castling_rights[2][2]; // [color][king_side/queen_side]
    int halfmove_clock;
    int fullmove_number;
    
    // Histórico para undo
    struct GameState {
        Move move;
        Square en_passant_square;
        bool castling_rights[2][2];
        int halfmove_clock;
        PieceType captured_piece;
        Square captured_square;
    };
    std::vector<GameState> history;
    
    // Lookup tables para movimentos
    static std::array<Bitboard, 64> knight_moves;
    static std::array<Bitboard, 64> king_moves;
    static std::array<std::array<Bitboard, 64>, 4> pawn_attacks; // [color][square]
    static std::array<Bitboard, 64> pawn_pushes_white;
    static std::array<Bitboard, 64> pawn_pushes_black;
    
    // Funções auxiliares
    void initialize_lookup_tables();
    void update_bitboards();
    Bitboard get_attacks_by(Square sq, PieceType pt, Color c) const;
    Bitboard get_attacks_to(Square sq, Color attacker_color) const;
    bool is_square_attacked(Square sq, Color by_color) const;
    Bitboard get_pawn_attacks(Square sq, Color c) const;
    Bitboard get_knight_attacks(Square sq) const;
    Bitboard get_bishop_attacks(Square sq, Bitboard occupied) const;
    Bitboard get_rook_attacks(Square sq, Bitboard occupied) const;
    Bitboard get_queen_attacks(Square sq, Bitboard occupied) const;
    Bitboard get_king_attacks(Square sq) const;
    
    // Geração de movimentos
    void generate_pawn_moves(std::vector<Move>& moves, Color c) const;
    void generate_knight_moves(std::vector<Move>& moves, Color c) const;
    void generate_bishop_moves(std::vector<Move>& moves, Color c) const;
    void generate_rook_moves(std::vector<Move>& moves, Color c) const;
    void generate_queen_moves(std::vector<Move>& moves, Color c) const;
    void generate_king_moves(std::vector<Move>& moves, Color c) const;
    void generate_castling_moves(std::vector<Move>& moves, Color c) const;
    
    // Validação
    bool is_legal_move(const Move& move) const;
    void make_move_internal(const Move& move);
    
public:
    ChessBoard();
    ChessBoard(const std::string& fen);
    
    // Geração de movimentos legais
    std::vector<Move> generate_legal_moves() const;
    
    // Execução de movimentos
    bool make_move(const Move& move);
    void unmake_move();
    
    // Estado do jogo
    bool is_check(Color c) const;
    bool is_checkmate(Color c) const;
    bool is_stalemate(Color c) const;
    bool is_game_over() const;
    
    // Informações do tabuleiro
    PieceType get_piece(Square sq) const;
    Color get_piece_color(Square sq) const;
    Color get_side_to_move() const { return side_to_move; }
    int get_fullmove_number() const { return fullmove_number; }
    
    // Representação
    void print_board() const;
    std::string to_fen() const;
    void from_fen(const std::string& fen);
    
    // Utilitários
    static Square square_from_string(const std::string& str);
    static std::string square_to_string(Square sq);
    static int get_file(Square sq) { return sq & 7; }
    static int get_rank(Square sq) { return sq >> 3; }
    static Square make_square(int file, int rank) { return rank * 8 + file; }
};

#endif // CHESS_H

