#include "chess.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cstring>
#ifdef _MSC_VER
#include <intrin.h>
#endif

// Inicialização das lookup tables estáticas
std::array<Bitboard, 64> ChessBoard::knight_moves;
std::array<Bitboard, 64> ChessBoard::king_moves;
std::array<std::array<Bitboard, 64>, 4> ChessBoard::pawn_attacks;
std::array<Bitboard, 64> ChessBoard::pawn_pushes_white;
std::array<Bitboard, 64> ChessBoard::pawn_pushes_black;

// Constantes para máscaras
const Bitboard FILE_A = 0x0101010101010101ULL;
const Bitboard FILE_H = 0x8080808080808080ULL;
const Bitboard RANK_1 = 0x00000000000000FFULL;
const Bitboard RANK_8 = 0xFF00000000000000ULL;
const Bitboard RANK_4 = 0x00000000FF000000ULL;
const Bitboard RANK_5 = 0x000000FF00000000ULL;

// Funções auxiliares de bitboard
inline Bitboard set_bit(Square sq) { return 1ULL << sq; }
inline bool get_bit(Bitboard bb, Square sq) { return (bb >> sq) & 1; }
inline Bitboard clear_bit(Bitboard bb, Square sq) { return bb & ~set_bit(sq); }

// Funções portáveis para operações bitboard
#ifdef _MSC_VER
    inline int pop_count(Bitboard bb) { return (int)__popcnt64(bb); }
    inline Square lsb(Bitboard bb) {
        if (bb == 0) return NO_SQUARE;
        unsigned long index;
        _BitScanForward64(&index, bb);
        return (Square)index;
    }
    inline Square msb(Bitboard bb) {
        if (bb == 0) return NO_SQUARE;
        unsigned long index;
        _BitScanReverse64(&index, bb);
        return (Square)index;
    }
#else
    inline int pop_count(Bitboard bb) { return __builtin_popcountll(bb); }
    inline Square lsb(Bitboard bb) { 
        if (bb == 0) return NO_SQUARE;
        return __builtin_ctzll(bb); 
    }
    inline Square msb(Bitboard bb) { 
        if (bb == 0) return NO_SQUARE;
        return 63 - __builtin_clzll(bb); 
    }
#endif

// Magic numbers para bispos e torres (simplificado - usando lookup direto)
Bitboard ChessBoard::get_bishop_attacks(Square sq, Bitboard occupied) const {
    Bitboard attacks = 0;
    int rank = get_rank(sq);
    int file = get_file(sq);
    
    // Diagonal principal (noroeste-sudeste)
    for (int r = rank + 1, f = file + 1; r < 8 && f < 8; r++, f++) {
        Square s = make_square(f, r);
        attacks |= set_bit(s);
        if (get_bit(occupied, s)) break;
    }
    for (int r = rank - 1, f = file - 1; r >= 0 && f >= 0; r--, f--) {
        Square s = make_square(f, r);
        attacks |= set_bit(s);
        if (get_bit(occupied, s)) break;
    }
    
    // Diagonal secundária (nordeste-sudoeste)
    for (int r = rank + 1, f = file - 1; r < 8 && f >= 0; r++, f--) {
        Square s = make_square(f, r);
        attacks |= set_bit(s);
        if (get_bit(occupied, s)) break;
    }
    for (int r = rank - 1, f = file + 1; r >= 0 && f < 8; r--, f++) {
        Square s = make_square(f, r);
        attacks |= set_bit(s);
        if (get_bit(occupied, s)) break;
    }
    
    return attacks;
}

Bitboard ChessBoard::get_rook_attacks(Square sq, Bitboard occupied) const {
    Bitboard attacks = 0;
    int rank = get_rank(sq);
    int file = get_file(sq);
    
    // Horizontal
    for (int f = file + 1; f < 8; f++) {
        Square s = make_square(f, rank);
        attacks |= set_bit(s);
        if (get_bit(occupied, s)) break;
    }
    for (int f = file - 1; f >= 0; f--) {
        Square s = make_square(f, rank);
        attacks |= set_bit(s);
        if (get_bit(occupied, s)) break;
    }
    
    // Vertical
    for (int r = rank + 1; r < 8; r++) {
        Square s = make_square(file, r);
        attacks |= set_bit(s);
        if (get_bit(occupied, s)) break;
    }
    for (int r = rank - 1; r >= 0; r--) {
        Square s = make_square(file, r);
        attacks |= set_bit(s);
        if (get_bit(occupied, s)) break;
    }
    
    return attacks;
}

Bitboard ChessBoard::get_queen_attacks(Square sq, Bitboard occupied) const {
    return get_bishop_attacks(sq, occupied) | get_rook_attacks(sq, occupied);
}

Bitboard ChessBoard::get_knight_attacks(Square sq) const {
    return knight_moves[sq];
}

Bitboard ChessBoard::get_king_attacks(Square sq) const {
    return king_moves[sq];
}

Bitboard ChessBoard::get_pawn_attacks(Square sq, Color c) const {
    if (c == WHITE) {
        return pawn_attacks[0][sq];
    } else {
        return pawn_attacks[1][sq];
    }
}

void ChessBoard::initialize_lookup_tables() {
    // Inicializar movimentos de cavalo
    for (Square sq = 0; sq < 64; sq++) {
        Bitboard moves = 0;
        int rank = get_rank(sq);
        int file = get_file(sq);
        
        int offsets[8][2] = {{-2, -1}, {-2, 1}, {-1, -2}, {-1, 2},
                             {1, -2}, {1, 2}, {2, -1}, {2, 1}};
        
        for (int i = 0; i < 8; i++) {
            int r = rank + offsets[i][0];
            int f = file + offsets[i][1];
            if (r >= 0 && r < 8 && f >= 0 && f < 8) {
                moves |= set_bit(make_square(f, r));
            }
        }
        knight_moves[sq] = moves;
    }
    
    // Inicializar movimentos de rei
    for (Square sq = 0; sq < 64; sq++) {
        Bitboard moves = 0;
        int rank = get_rank(sq);
        int file = get_file(sq);
        
        for (int dr = -1; dr <= 1; dr++) {
            for (int df = -1; df <= 1; df++) {
                if (dr == 0 && df == 0) continue;
                int r = rank + dr;
                int f = file + df;
                if (r >= 0 && r < 8 && f >= 0 && f < 8) {
                    moves |= set_bit(make_square(f, r));
                }
            }
        }
        king_moves[sq] = moves;
    }
    
    // Inicializar ataques de peão
    for (Square sq = 0; sq < 64; sq++) {
        int rank = get_rank(sq);
        int file = get_file(sq);
        
        // Ataques brancos (norte)
        Bitboard white_attacks = 0;
        if (rank < 7) {
            if (file > 0) white_attacks |= set_bit(make_square(file - 1, rank + 1));
            if (file < 7) white_attacks |= set_bit(make_square(file + 1, rank + 1));
        }
        pawn_attacks[0][sq] = white_attacks;
        
        // Ataques pretos (sul)
        Bitboard black_attacks = 0;
        if (rank > 0) {
            if (file > 0) black_attacks |= set_bit(make_square(file - 1, rank - 1));
            if (file < 7) black_attacks |= set_bit(make_square(file + 1, rank - 1));
        }
        pawn_attacks[1][sq] = black_attacks;
        
        // Empurrões de peão
        pawn_pushes_white[sq] = (rank < 7) ? set_bit(make_square(file, rank + 1)) : 0;
        if (rank == 1) pawn_pushes_white[sq] |= set_bit(make_square(file, rank + 2));
        
        pawn_pushes_black[sq] = (rank > 0) ? set_bit(make_square(file, rank - 1)) : 0;
        if (rank == 6) pawn_pushes_black[sq] |= set_bit(make_square(file, rank - 2));
    }
}

ChessBoard::ChessBoard() {
    // Inicializar lookup tables
    initialize_lookup_tables();
    
    // Posição inicial padrão
    pieces_white[PAWN] = 0x000000000000FF00ULL;
    pieces_white[KNIGHT] = 0x0000000000000042ULL;
    pieces_white[BISHOP] = 0x0000000000000024ULL;
    pieces_white[ROOK] = 0x0000000000000081ULL;
    pieces_white[QUEEN] = 0x0000000000000008ULL;
    pieces_white[KING] = 0x0000000000000010ULL;
    
    pieces_black[PAWN] = 0x00FF000000000000ULL;
    pieces_black[KNIGHT] = 0x4200000000000000ULL;
    pieces_black[BISHOP] = 0x2400000000000000ULL;
    pieces_black[ROOK] = 0x8100000000000000ULL;
    pieces_black[QUEEN] = 0x0800000000000000ULL;
    pieces_black[KING] = 0x1000000000000000ULL;
    
    update_bitboards();
    
    side_to_move = WHITE;
    en_passant_square = NO_SQUARE;
    castling_rights[WHITE][0] = true;  // king side
    castling_rights[WHITE][1] = true;  // queen side
    castling_rights[BLACK][0] = true;
    castling_rights[BLACK][1] = true;
    halfmove_clock = 0;
    fullmove_number = 1;
}

ChessBoard::ChessBoard(const std::string& fen) {
    initialize_lookup_tables();
    from_fen(fen);
}

void ChessBoard::update_bitboards() {
    all_white = 0;
    all_black = 0;
    
    for (int i = 0; i < 6; i++) {
        all_white |= pieces_white[i];
        all_black |= pieces_black[i];
    }
    
    all_pieces = all_white | all_black;
}

PieceType ChessBoard::get_piece(Square sq) const {
    Bitboard sq_bb = set_bit(sq);
    
    for (int i = 0; i < 6; i++) {
        if (pieces_white[i] & sq_bb) return static_cast<PieceType>(i);
        if (pieces_black[i] & sq_bb) return static_cast<PieceType>(i);
    }
    
    return NONE;
}

Color ChessBoard::get_piece_color(Square sq) const {
    Bitboard sq_bb = set_bit(sq);
    if (all_white & sq_bb) return WHITE;
    if (all_black & sq_bb) return BLACK;
    return WHITE; // default
}

Bitboard ChessBoard::get_attacks_by(Square sq, PieceType pt, Color c) const {
    switch (pt) {
        case PAWN:
            return get_pawn_attacks(sq, c);
        case KNIGHT:
            return get_knight_attacks(sq);
        case BISHOP:
            return get_bishop_attacks(sq, all_pieces);
        case ROOK:
            return get_rook_attacks(sq, all_pieces);
        case QUEEN:
            return get_queen_attacks(sq, all_pieces);
        case KING:
            return get_king_attacks(sq);
        default:
            return 0;
    }
}

Bitboard ChessBoard::get_attacks_to(Square sq, Color attacker_color) const {
    Bitboard attacks = 0;
    const auto& pieces = (attacker_color == WHITE) ? pieces_white : pieces_black;
    
    // Peões
    Bitboard pawn_att = get_pawn_attacks(sq, attacker_color == WHITE ? BLACK : WHITE);
    attacks |= pawn_att & pieces[PAWN];
    
    // Cavalos
    Bitboard knight_att = get_knight_attacks(sq);
    attacks |= knight_att & pieces[KNIGHT];
    
    // Bispos
    Bitboard bishop_att = get_bishop_attacks(sq, all_pieces);
    attacks |= bishop_att & (pieces[BISHOP] | pieces[QUEEN]);
    
    // Torres
    Bitboard rook_att = get_rook_attacks(sq, all_pieces);
    attacks |= rook_att & (pieces[ROOK] | pieces[QUEEN]);
    
    // Rei
    Bitboard king_att = get_king_attacks(sq);
    attacks |= king_att & pieces[KING];
    
    return attacks;
}

bool ChessBoard::is_square_attacked(Square sq, Color by_color) const {
    return get_attacks_to(sq, by_color) != 0;
}

bool ChessBoard::is_check(Color c) const {
    Square king_sq = NO_SQUARE;
    const auto& pieces = (c == WHITE) ? pieces_white : pieces_black;
    Bitboard king_bb = pieces[KING];
    
    if (king_bb == 0) return false;
    king_sq = lsb(king_bb);
    
    return is_square_attacked(king_sq, c == WHITE ? BLACK : WHITE);
}

void ChessBoard::generate_pawn_moves(std::vector<Move>& moves, Color c) const {
    const auto& pieces = (c == WHITE) ? pieces_white : pieces_black;
    Bitboard pawns = pieces[PAWN];
    Bitboard enemies = (c == WHITE) ? all_black : all_white;
    
    while (pawns) {
        Square from = lsb(pawns);
        pawns &= pawns - 1; // clear lsb
        
        // Empurrão simples
        Square single_push_sq = (c == WHITE) ? make_square(get_file(from), get_rank(from) + 1) 
                                             : make_square(get_file(from), get_rank(from) - 1);
        if (single_push_sq < 64 && !get_bit(all_pieces, single_push_sq)) {
            // Promoção
            if ((c == WHITE && get_rank(single_push_sq) == 7) || (c == BLACK && get_rank(single_push_sq) == 0)) {
                for (int pt = KNIGHT; pt <= QUEEN; pt++) {
                    moves.push_back(Move(from, single_push_sq, static_cast<PieceType>(pt)));
                }
            } else {
                moves.push_back(Move(from, single_push_sq));
                
                // Empurrão duplo (só se o simples foi possível)
                Square double_push_sq = NO_SQUARE;
                if (c == WHITE && get_rank(from) == 1) {
                    double_push_sq = make_square(get_file(from), 3);
                } else if (c == BLACK && get_rank(from) == 6) {
                    double_push_sq = make_square(get_file(from), 4);
                }
                
                if (double_push_sq != NO_SQUARE && !get_bit(all_pieces, double_push_sq)) {
                    moves.push_back(Move(from, double_push_sq));
                }
            }
        }
        
        // Capturas
        Bitboard attacks = get_pawn_attacks(from, c);
        attacks &= enemies;
        
        while (attacks) {
            Square to = lsb(attacks);
            attacks &= attacks - 1;
            
            // Promoção
            if ((c == WHITE && get_rank(to) == 7) || (c == BLACK && get_rank(to) == 0)) {
                for (int pt = KNIGHT; pt <= QUEEN; pt++) {
                    moves.push_back(Move(from, to, static_cast<PieceType>(pt)));
                }
            } else {
                moves.push_back(Move(from, to));
            }
        }
        
        // En passant
        if (en_passant_square != NO_SQUARE) {
            Bitboard ep_attacks = get_pawn_attacks(from, c);
            if (ep_attacks & set_bit(en_passant_square)) {
                Move ep_move(from, en_passant_square);
                ep_move.is_en_passant = true;
                moves.push_back(ep_move);
            }
        }
    }
}

void ChessBoard::generate_knight_moves(std::vector<Move>& moves, Color c) const {
    const auto& pieces = (c == WHITE) ? pieces_white : pieces_black;
    Bitboard knights = pieces[KNIGHT];
    Bitboard friends = (c == WHITE) ? all_white : all_black;
    
    while (knights) {
        Square from = lsb(knights);
        knights &= knights - 1;
        
        Bitboard attacks = get_knight_attacks(from);
        attacks &= ~friends;
        
        while (attacks) {
            Square to = lsb(attacks);
            attacks &= attacks - 1;
            moves.push_back(Move(from, to));
        }
    }
}

void ChessBoard::generate_bishop_moves(std::vector<Move>& moves, Color c) const {
    const auto& pieces = (c == WHITE) ? pieces_white : pieces_black;
    Bitboard bishops = pieces[BISHOP];
    Bitboard friends = (c == WHITE) ? all_white : all_black;
    
    while (bishops) {
        Square from = lsb(bishops);
        bishops &= bishops - 1;
        
        Bitboard attacks = get_bishop_attacks(from, all_pieces);
        attacks &= ~friends;
        
        while (attacks) {
            Square to = lsb(attacks);
            attacks &= attacks - 1;
            moves.push_back(Move(from, to));
        }
    }
}

void ChessBoard::generate_rook_moves(std::vector<Move>& moves, Color c) const {
    const auto& pieces = (c == WHITE) ? pieces_white : pieces_black;
    Bitboard rooks = pieces[ROOK];
    Bitboard friends = (c == WHITE) ? all_white : all_black;
    
    while (rooks) {
        Square from = lsb(rooks);
        rooks &= rooks - 1;
        
        Bitboard attacks = get_rook_attacks(from, all_pieces);
        attacks &= ~friends;
        
        while (attacks) {
            Square to = lsb(attacks);
            attacks &= attacks - 1;
            moves.push_back(Move(from, to));
        }
    }
}

void ChessBoard::generate_queen_moves(std::vector<Move>& moves, Color c) const {
    const auto& pieces = (c == WHITE) ? pieces_white : pieces_black;
    Bitboard queens = pieces[QUEEN];
    Bitboard friends = (c == WHITE) ? all_white : all_black;
    
    while (queens) {
        Square from = lsb(queens);
        queens &= queens - 1;
        
        Bitboard attacks = get_queen_attacks(from, all_pieces);
        attacks &= ~friends;
        
        while (attacks) {
            Square to = lsb(attacks);
            attacks &= attacks - 1;
            moves.push_back(Move(from, to));
        }
    }
}

void ChessBoard::generate_king_moves(std::vector<Move>& moves, Color c) const {
    const auto& pieces = (c == WHITE) ? pieces_white : pieces_black;
    Bitboard king = pieces[KING];
    if (king == 0) return;
    
    Square from = lsb(king);
    Bitboard friends = (c == WHITE) ? all_white : all_black;
    Bitboard attacks = get_king_attacks(from);
    attacks &= ~friends;
    
    while (attacks) {
        Square to = lsb(attacks);
        attacks &= attacks - 1;
        moves.push_back(Move(from, to));
    }
}

void ChessBoard::generate_castling_moves(std::vector<Move>& moves, Color c) const {
    if (is_check(c)) return; // Não pode fazer roque em xeque
    
    Square king_sq = (c == WHITE) ? E1 : E8;
    const auto& pieces = (c == WHITE) ? pieces_white : pieces_black;
    
    if (!(pieces[KING] & set_bit(king_sq))) return;
    
    // Roque do lado do rei
    if (castling_rights[c][0]) {
        Square rook_sq = (c == WHITE) ? H1 : H8;
        Square f1 = (c == WHITE) ? F1 : F8;
        Square g1 = (c == WHITE) ? G1 : G8;
        
        if (pieces[ROOK] & set_bit(rook_sq)) {
            // Verificar se as casas estão vazias e não estão atacadas
            Bitboard path = set_bit(f1) | set_bit(g1);
            if ((path & all_pieces) == 0) {
                if (!is_square_attacked(f1, c == WHITE ? BLACK : WHITE) &&
                    !is_square_attacked(g1, c == WHITE ? BLACK : WHITE)) {
                    Move castle(king_sq, g1);
                    castle.is_castle = true;
                    moves.push_back(castle);
                }
            }
        }
    }
    
    // Roque do lado da dama
    if (castling_rights[c][1]) {
        Square rook_sq = (c == WHITE) ? A1 : A8;
        Square d1 = (c == WHITE) ? D1 : D8;
        Square c1 = (c == WHITE) ? C1 : C8;
        Square b1 = (c == WHITE) ? B1 : B8;
        
        if (pieces[ROOK] & set_bit(rook_sq)) {
            Bitboard path = set_bit(b1) | set_bit(c1) | set_bit(d1);
            if ((path & all_pieces) == 0) {
                if (!is_square_attacked(c1, c == WHITE ? BLACK : WHITE) &&
                    !is_square_attacked(d1, c == WHITE ? BLACK : WHITE)) {
                    Move castle(king_sq, c1);
                    castle.is_castle = true;
                    moves.push_back(castle);
                }
            }
        }
    }
}

std::vector<Move> ChessBoard::generate_legal_moves() const {
    std::vector<Move> moves;
    
    generate_pawn_moves(moves, side_to_move);
    generate_knight_moves(moves, side_to_move);
    generate_bishop_moves(moves, side_to_move);
    generate_rook_moves(moves, side_to_move);
    generate_queen_moves(moves, side_to_move);
    generate_king_moves(moves, side_to_move);
    generate_castling_moves(moves, side_to_move);
    
    // Filtrar movimentos ilegais (que deixam o rei em xeque)
    std::vector<Move> legal_moves;
    for (const auto& move : moves) {
        if (is_legal_move(move)) {
            legal_moves.push_back(move);
        }
    }
    
    return legal_moves;
}

bool ChessBoard::is_legal_move(const Move& move) const {
    // Fazer o movimento temporariamente
    ChessBoard temp = *this;
    temp.make_move_internal(move);
    
    // Verificar se o rei está em xeque após o movimento
    return !temp.is_check(side_to_move);
}

void ChessBoard::make_move_internal(const Move& move) {
    GameState state;
    state.move = move;
    state.en_passant_square = en_passant_square;
    state.castling_rights[0][0] = castling_rights[0][0];
    state.castling_rights[0][1] = castling_rights[0][1];
    state.castling_rights[1][0] = castling_rights[1][0];
    state.castling_rights[1][1] = castling_rights[1][1];
    state.halfmove_clock = halfmove_clock;
    state.captured_piece = NONE;
    state.captured_square = NO_SQUARE;
    
    auto& my_pieces = (side_to_move == WHITE) ? pieces_white : pieces_black;
    auto& enemy_pieces = (side_to_move == WHITE) ? pieces_black : pieces_white;
    
    PieceType moving_piece = get_piece(move.from);
    
    // Captura
    PieceType captured = get_piece(move.to);
    if (captured != NONE) {
        state.captured_piece = captured;
        state.captured_square = move.to;
        enemy_pieces[captured] &= ~set_bit(move.to);
    }
    
    // En passant
    if (move.is_en_passant) {
        // O peão capturado está na casa atrás do en_passant_square
        // Para brancas: en_passant_square está em rank 2, peão preto está em rank 3 (rank + 1)
        // Para pretas: en_passant_square está em rank 5, peão branco está em rank 4 (rank - 1)
        Square ep_capture = (side_to_move == WHITE) ? 
            make_square(get_file(en_passant_square), get_rank(en_passant_square) + 1) :
            make_square(get_file(en_passant_square), get_rank(en_passant_square) - 1);
        enemy_pieces[PAWN] &= ~set_bit(ep_capture);
        state.captured_piece = PAWN;
        state.captured_square = ep_capture;
    }
    
    // Mover peça
    my_pieces[moving_piece] &= ~set_bit(move.from);
    if (move.promotion != NONE) {
        my_pieces[move.promotion] |= set_bit(move.to);
    } else {
        my_pieces[moving_piece] |= set_bit(move.to);
    }
    
    // Roque
    if (move.is_castle) {
        Square king_from = move.from;
        Square king_to = move.to;
        Square rook_from, rook_to;
        
        // Determinar qual lado do roque baseado na posição de destino do rei
        // Roque do lado do rei: rei vai para G1/G8 (file 6)
        // Roque do lado da dama: rei vai para C1/C8 (file 2)
        int king_to_file = get_file(king_to);
        int king_from_file = get_file(king_from);
        
        if (king_to_file == 6) {
            // Roque do lado do rei (king-side)
            rook_from = (side_to_move == WHITE) ? H1 : H8;
            rook_to = (side_to_move == WHITE) ? F1 : F8;
        } else if (king_to_file == 2) {
            // Roque do lado da dama (queen-side)
            rook_from = (side_to_move == WHITE) ? A1 : A8;
            rook_to = (side_to_move == WHITE) ? D1 : D8;
        } else {
            // Fallback: usar comparação de files
            if (king_to_file > king_from_file) {
                // Roque do lado do rei
                rook_from = (side_to_move == WHITE) ? H1 : H8;
                rook_to = (side_to_move == WHITE) ? F1 : F8;
            } else {
                // Roque do lado da dama
                rook_from = (side_to_move == WHITE) ? A1 : A8;
                rook_to = (side_to_move == WHITE) ? D1 : D8;
            }
        }
        
        // Mover a torre: remover da posição original e adicionar na nova posição
        // Se chegamos aqui, o roque foi validado, então a torre deve estar na posição correta
        my_pieces[ROOK] &= ~set_bit(rook_from);  // Remover torre da posição original
        my_pieces[ROOK] |= set_bit(rook_to);     // Adicionar torre na nova posição
    }
    
    // Atualizar en passant
    en_passant_square = NO_SQUARE;
    if (moving_piece == PAWN) {
        int rank_diff = abs(get_rank(move.to) - get_rank(move.from));
        if (rank_diff == 2) {
            en_passant_square = make_square(get_file(move.from), 
                (side_to_move == WHITE) ? get_rank(move.from) + 1 : get_rank(move.from) - 1);
        }
    }
    
    // Atualizar direitos de roque
    if (moving_piece == KING) {
        castling_rights[side_to_move][0] = false;
        castling_rights[side_to_move][1] = false;
    }
    if (moving_piece == ROOK) {
        if (move.from == (side_to_move == WHITE ? A1 : A8)) {
            castling_rights[side_to_move][1] = false;
        } else if (move.from == (side_to_move == WHITE ? H1 : H8)) {
            castling_rights[side_to_move][0] = false;
        }
    }
    if (captured == ROOK) {
        if (move.to == (side_to_move == WHITE ? A8 : A1)) {
            castling_rights[!side_to_move][1] = false;
        } else if (move.to == (side_to_move == WHITE ? H8 : H1)) {
            castling_rights[!side_to_move][0] = false;
        }
    }
    
    // Atualizar contadores
    if (moving_piece == PAWN || captured != NONE) {
        halfmove_clock = 0;
    } else {
        halfmove_clock++;
    }
    
    if (side_to_move == BLACK) {
        fullmove_number++;
    }
    
    side_to_move = (side_to_move == WHITE) ? BLACK : WHITE;
    
    update_bitboards();
    history.push_back(state);
}

bool ChessBoard::make_move(const Move& move) {
    if (!is_legal_move(move)) {
        return false;
    }
    
    make_move_internal(move);
    return true;
}

void ChessBoard::unmake_move() {
    if (history.empty()) return;
    
    GameState state = history.back();
    history.pop_back();
    
    side_to_move = (side_to_move == WHITE) ? BLACK : WHITE;
    
    auto& my_pieces = (side_to_move == WHITE) ? pieces_white : pieces_black;
    auto& enemy_pieces = (side_to_move == WHITE) ? pieces_black : pieces_white;
    
    Move move = state.move;
    PieceType moving_piece = get_piece(move.to);
    if (move.promotion != NONE) {
        moving_piece = PAWN;
    }
    
    // Desfazer movimento
    my_pieces[moving_piece] &= ~set_bit(move.to);
    my_pieces[moving_piece] |= set_bit(move.from);
    if (move.promotion != NONE) {
        my_pieces[move.promotion] &= ~set_bit(move.to);
    }
    
    // Desfazer captura
    if (state.captured_piece != NONE) {
        enemy_pieces[state.captured_piece] |= set_bit(state.captured_square);
    }
    
    // Desfazer en passant
    if (move.is_en_passant) {
        // O peão capturado está na casa atrás do en_passant_square
        Square ep_capture = (side_to_move == WHITE) ? 
            make_square(get_file(en_passant_square), get_rank(en_passant_square) + 1) :
            make_square(get_file(en_passant_square), get_rank(en_passant_square) - 1);
        enemy_pieces[PAWN] |= set_bit(ep_capture);
    }
    
    // Desfazer roque
    if (move.is_castle) {
        Square king_from = move.from;
        Square king_to = move.to;
        Square rook_from, rook_to;
        
        // Determinar qual lado do roque baseado na posição de destino do rei
        if (get_file(king_to) == 6) {
            // Roque do lado do rei (king-side)
            rook_from = (side_to_move == WHITE) ? H1 : H8;
            rook_to = (side_to_move == WHITE) ? F1 : F8;
        } else if (get_file(king_to) == 2) {
            // Roque do lado da dama (queen-side)
            rook_from = (side_to_move == WHITE) ? A1 : A8;
            rook_to = (side_to_move == WHITE) ? D1 : D8;
        } else {
            // Fallback para compatibilidade
            if (get_file(king_to) > get_file(king_from)) {
                rook_from = (side_to_move == WHITE) ? H1 : H8;
                rook_to = (side_to_move == WHITE) ? F1 : F8;
            } else {
                rook_from = (side_to_move == WHITE) ? A1 : A8;
                rook_to = (side_to_move == WHITE) ? D1 : D8;
            }
        }
        
        // Desfazer movimento da torre
        my_pieces[ROOK] &= ~set_bit(rook_to);
        my_pieces[ROOK] |= set_bit(rook_from);
    }
    
    // Restaurar estado
    en_passant_square = state.en_passant_square;
    castling_rights[0][0] = state.castling_rights[0][0];
    castling_rights[0][1] = state.castling_rights[0][1];
    castling_rights[1][0] = state.castling_rights[1][0];
    castling_rights[1][1] = state.castling_rights[1][1];
    halfmove_clock = state.halfmove_clock;
    
    if (side_to_move == BLACK) {
        fullmove_number--;
    }
    
    update_bitboards();
}

bool ChessBoard::is_checkmate(Color c) const {
    if (!is_check(c)) return false;
    if (c != side_to_move) {
        // Se não é a vez deste lado, criar tabuleiro temporário
        ChessBoard temp = *this;
        temp.side_to_move = c;
        std::vector<Move> moves = temp.generate_legal_moves();
        return moves.empty();
    }
    std::vector<Move> moves = const_cast<ChessBoard*>(this)->generate_legal_moves();
    return moves.empty();
}

bool ChessBoard::is_stalemate(Color c) const {
    if (is_check(c)) return false;
    if (c != side_to_move) {
        // Se não é a vez deste lado, criar tabuleiro temporário
        ChessBoard temp = *this;
        temp.side_to_move = c;
        std::vector<Move> moves = temp.generate_legal_moves();
        return moves.empty();
    }
    std::vector<Move> moves = const_cast<ChessBoard*>(this)->generate_legal_moves();
    return moves.empty();
}

bool ChessBoard::is_game_over() const {
    return is_checkmate(side_to_move) || is_stalemate(side_to_move);
}

void ChessBoard::print_board() const {
    const char* piece_chars = "PNBRQK";
    
    std::cout << "\n  a b c d e f g h\n";
    for (int rank = 7; rank >= 0; rank--) {
        std::cout << (rank + 1) << " ";
        for (int file = 0; file < 8; file++) {
            Square sq = make_square(file, rank);
            PieceType pt = get_piece(sq);
            
            if (pt == NONE) {
                std::cout << ". ";
            } else {
                Color c = get_piece_color(sq);
                char piece = piece_chars[pt];
                if (c == WHITE) {
                    std::cout << static_cast<char>(toupper(piece)) << " ";
                } else {
                    std::cout << static_cast<char>(tolower(piece)) << " ";
                }
            }
        }
        std::cout << (rank + 1) << "\n";
    }
    std::cout << "  a b c d e f g h\n\n";
}

Square ChessBoard::square_from_string(const std::string& str) {
    if (str.length() != 2) return NO_SQUARE;
    int file = str[0] - 'a';
    int rank = str[1] - '1';
    if (file < 0 || file > 7 || rank < 0 || rank > 7) return NO_SQUARE;
    return make_square(file, rank);
}

std::string ChessBoard::square_to_string(Square sq) {
    if (sq == NO_SQUARE) return "";
    std::string str;
    str += 'a' + get_file(sq);
    str += '1' + get_rank(sq);
    return str;
}

std::string Move::to_string() const {
    std::string str = ChessBoard::square_to_string(from) + ChessBoard::square_to_string(to);
    if (promotion != NONE) {
        const char* prom_chars = "nbrq";
        str += prom_chars[promotion - 1];
    }
    return str;
}

Move Move::from_string(const std::string& move_str) {
    if (move_str.length() < 4 || move_str.length() > 5) return Move();
    
    Square from = ChessBoard::square_from_string(move_str.substr(0, 2));
    Square to = ChessBoard::square_from_string(move_str.substr(2, 2));
    
    if (from == NO_SQUARE || to == NO_SQUARE) return Move();
    
    PieceType promotion = NONE;
    if (move_str.length() == 5) {
        char prom = tolower(move_str[4]);
        switch (prom) {
            case 'n': promotion = KNIGHT; break;
            case 'b': promotion = BISHOP; break;
            case 'r': promotion = ROOK; break;
            case 'q': promotion = QUEEN; break;
        }
    }
    
    return Move(from, to, promotion);
}

std::string ChessBoard::to_fen() const {
    std::ostringstream fen;
    
    // Posição das peças
    for (int rank = 7; rank >= 0; rank--) {
        int empty_count = 0;
        for (int file = 0; file < 8; file++) {
            Square sq = make_square(file, rank);
            PieceType pt = get_piece(sq);
            
            if (pt == NONE) {
                empty_count++;
            } else {
                if (empty_count > 0) {
                    fen << empty_count;
                    empty_count = 0;
                }
                Color c = get_piece_color(sq);
                const char* piece_chars = "PNBRQK";
                char piece = piece_chars[pt];
                fen << (c == WHITE ? static_cast<char>(toupper(piece)) : static_cast<char>(tolower(piece)));
            }
        }
        if (empty_count > 0) fen << empty_count;
        if (rank > 0) fen << "/";
    }
    
    // Lado a mover
    fen << " " << (side_to_move == WHITE ? "w" : "b") << " ";
    
    // Direitos de roque
    bool any_castle = false;
    if (castling_rights[WHITE][0]) { fen << "K"; any_castle = true; }
    if (castling_rights[WHITE][1]) { fen << "Q"; any_castle = true; }
    if (castling_rights[BLACK][0]) { fen << "k"; any_castle = true; }
    if (castling_rights[BLACK][1]) { fen << "q"; any_castle = true; }
    if (!any_castle) fen << "-";
    
    // En passant
    fen << " ";
    if (en_passant_square != NO_SQUARE) {
        fen << square_to_string(en_passant_square);
    } else {
        fen << "-";
    }
    
    // Contadores
    fen << " " << halfmove_clock << " " << fullmove_number;
    
    return fen.str();
}

void ChessBoard::from_fen(const std::string& fen) {
    // Resetar tabuleiro
    for (int i = 0; i < 6; i++) {
        pieces_white[i] = 0;
        pieces_black[i] = 0;
    }
    
    std::istringstream iss(fen);
    std::string board_str, turn_str, castle_str, ep_str;
    int halfmove, fullmove;
    
    iss >> board_str >> turn_str >> castle_str >> ep_str >> halfmove >> fullmove;
    
    // Parse do tabuleiro
    int rank = 7, file = 0;
    for (char c : board_str) {
        if (c == '/') {
            rank--;
            file = 0;
        } else if (isdigit(c)) {
            file += c - '0';
        } else {
            Square sq = make_square(file, rank);
            Color color = isupper(c) ? WHITE : BLACK;
            c = tolower(c);
            
            PieceType pt = NONE;
            switch (c) {
                case 'p': pt = PAWN; break;
                case 'n': pt = KNIGHT; break;
                case 'b': pt = BISHOP; break;
                case 'r': pt = ROOK; break;
                case 'q': pt = QUEEN; break;
                case 'k': pt = KING; break;
            }
            
            if (color == WHITE) {
                pieces_white[pt] |= set_bit(sq);
            } else {
                pieces_black[pt] |= set_bit(sq);
            }
            file++;
        }
    }
    
    side_to_move = (turn_str == "w") ? WHITE : BLACK;
    
    castling_rights[WHITE][0] = (castle_str.find('K') != std::string::npos);
    castling_rights[WHITE][1] = (castle_str.find('Q') != std::string::npos);
    castling_rights[BLACK][0] = (castle_str.find('k') != std::string::npos);
    castling_rights[BLACK][1] = (castle_str.find('q') != std::string::npos);
    
    en_passant_square = (ep_str == "-") ? NO_SQUARE : square_from_string(ep_str);
    halfmove_clock = halfmove;
    fullmove_number = fullmove;
    
    update_bitboards();
}

