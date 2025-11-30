#include "chess_engine.h"
#include <chrono>
#include <algorithm>
#include <climits>
#include <cstring>
#include <iostream>

const int INFINITY_SCORE = 1000000000;
const int MATE_SCORE = 900000000;
const int TIME_LIMIT_MS = 1500; 

const int ChessEngine::PIECE_VALUES[7] = { 82, 337, 365, 477, 1025, 20000, 0 };
const int MOBILITY_BONUS[] = { 0, 4, 3, 2, 1, 0, 0 };

// --- TABELAS PESTO ---
const int PST_PAWN[64] = {
      0,   0,   0,   0,   0,   0,   0,   0, // Rank 8
     98, 134,  61,  95,  68, 126,  34, -11, // Rank 7
     -6,   7,  26,  31,  65,  56,  25, -20, // Rank 6
    -14,  13,   6,  21,  23,  12,  17, -23, // Rank 5
    -27,  -2,  -5,  12,  17,   6,  10, -25, // Rank 4
    -26,  -4,  -4, -10,   3,   3,  33, -12, // Rank 3
    -35,  -1, -20, -23, -15,  24,  38, -22, // Rank 2
      0,   0,   0,   0,   0,   0,   0,   0  // Rank 1
};

const int PST_KNIGHT[64] = {
   -167, -89, -34, -49,  61, -97, -15,-107, // Rank 8
    -73, -41,  72,  36,  23,  62,   7, -17, // Rank 7
    -47,  60,  37,  65,  84, 129,  73,  44, // Rank 6
     -9,  17,  19,  53,  37,  69,  18,  22, // Rank 5
    -13,   4,  16,  13,  28,  19,  21,  -8, // Rank 4
    -23,  -9,  12,  10,  19,  17,  25, -16, // Rank 3
    -29, -53, -12,  -3,  -1,  18, -14, -19, // Rank 2
   -105, -21, -58, -33, -17, -28, -19, -23  // Rank 1
};

const int PST_BISHOP[64] = {
    -29,   4, -82, -37, -25, -42,   7,  -8, // Rank 8
    -26,  16, -18, -13,  30,  59,  18, -47, // Rank 7
    -16,  37,  43,  40,  35,  50,  37,  -2, // Rank 6
     -4,   5,  19,  50,  37,  37,   7,  -2, // Rank 5
     -6,  13,  13,  26,  34,  12,  10,   4, // Rank 4
      0,  15,  15,  15,  14,  27,  18,  10, // Rank 3
      4,  15,  16,   0,   7,  21,  33,   1, // Rank 2
    -33,  -3, -14, -21, -13, -12, -39, -21  // Rank 1
};

const int PST_ROOK[64] = {
     32,  42,  32,  51,  63,   9,  31,  43, // Rank 8
     27,  32,  58,  62,  80,  67,  26,  44, // Rank 7
     -5,  19,  26,  36,  17,  45,  61,  16, // Rank 6
    -24, -11,   7,  26,  24,  35,  -8, -20, // Rank 5
    -36, -26, -12,  -1,   9,  -7,   6, -23, // Rank 4
    -45, -25, -16, -17,   3,   0,  -5, -33, // Rank 3
    -44, -16, -20,  -9,  -1,  11,  -6, -71, // Rank 2
    -19, -13,   1,  17,  16,   7, -37, -26  // Rank 1
};

const int PST_QUEEN[64] = {
    -28,   0,  29,  12,  59,  44,  43,  45, // Rank 8
    -24, -39,  -5,   1, -16,  57,  28,  54, // Rank 7
    -13, -17,   7,   8,  29,  56,  47,  57, // Rank 6
    -27, -27, -16, -16,  -1,  17,  -2,   1, // Rank 5
     -9, -26,  -9, -10,  -2,  -4,   3,  -3, // Rank 4
    -14,   2, -11,  -2,  -5,   2,  14,   5, // Rank 3
    -35,  -8,  11,   2,   8,  15,  -3,   1, // Rank 2
     -1, -18,  -9,  10, -15, -25, -31, -50  // Rank 1
};

const int PST_KING[64] = {
    -65,  23,  16, -15, -56, -34,   2,  13, // Rank 8
     29,  -1, -20,  -7,  -8,  -4, -38, -29, // Rank 7
     -9,  24,   2, -16, -20,   6,  22, -22, // Rank 6
    -17, -20, -12, -27, -30, -25, -14, -36, // Rank 5
    -49,  -1, -27, -39, -46, -44, -33, -51, // Rank 4
    -14, -14, -22, -46, -44, -30, -15, -27, // Rank 3
      1,   7,  -8, -64, -43, -16,   9,   8, // Rank 2
    -15,  36,  12, -54,  -8, -28,  24,  14  // Rank 1
};

const int* PST_TABLES[6] = {
    PST_PAWN,
    PST_KNIGHT,
    PST_BISHOP,
    PST_ROOK,
    PST_QUEEN,
    PST_KING
};
    ChessEngine::ChessEngine() : rng(std::chrono::steady_clock::now().time_since_epoch().count()), last_eval_score(0) {
    std::memset(history_moves, 0, sizeof(history_moves));
    for(int i=0; i<20; i++) { killer_moves[i][0] = Move(); killer_moves[i][1] = Move(); }
    tt.clear(); 
}

inline int count_bits(uint64_t n) { return __builtin_popcountll(n); }

// --- ORDENAÇÃO ---
void ChessEngine::order_moves(const ChessBoard& board, std::vector<Move>& moves, int ply, const Move& tt_move) const {
    std::sort(moves.begin(), moves.end(), [&](const Move& a, const Move& b) {
        int score_a = 0, score_b = 0;
        
        // 1. TT Move 
        if (tt_move.from != 64 && a == tt_move) return true; 
        if (tt_move.from != 64 && b == tt_move) return false;

        // Avaliar A
        PieceType victim_a = board.get_piece(a.to);
        if (victim_a != NONE) {
            score_a = 20000 + (PIECE_VALUES[victim_a] * 10) - PIECE_VALUES[board.get_piece(a.from)];
        } else {
            if (ply < 20) {
                if (a == killer_moves[ply][0]) score_a = 19000;
                else if (a == killer_moves[ply][1]) score_a = 18000;
            }
            if (score_a == 0 && a.from < 64 && a.to < 64) score_a = std::min(history_moves[a.from][a.to], 15000);
        }

        // Avaliar B
        PieceType victim_b = board.get_piece(b.to);
        if (victim_b != NONE) {
            score_b = 20000 + (PIECE_VALUES[victim_b] * 10) - PIECE_VALUES[board.get_piece(b.from)];
        } else {
            if (ply < 20) {
                if (b == killer_moves[ply][0]) score_b = 19000;
                else if (b == killer_moves[ply][1]) score_b = 18000;
            }
            if (score_b == 0 && b.from < 64 && b.to < 64) score_b = std::min(history_moves[b.from][b.to], 15000);
        }
        return score_a > score_b;
    });
}

void ChessEngine::order_moves_simple(const ChessBoard& board, std::vector<Move>& moves) const {
     std::sort(moves.begin(), moves.end(), [&](const Move& a, const Move& b) {
        int score_a = 0, score_b = 0;
        if (board.get_piece(a.to) != NONE) score_a = 10000 + PIECE_VALUES[board.get_piece(a.to)];
        if (board.get_piece(b.to) != NONE) score_b = 10000 + PIECE_VALUES[board.get_piece(b.to)];
        return score_a > score_b;
    });
}

int ChessEngine::evaluate_material(const ChessBoard& board) const {
    int score = 0;
    for (int sq = 0; sq < 64; sq++) {
        PieceType piece = board.get_piece(sq);
        if (piece != NONE) {
            Color color = board.get_piece_color(sq);
            int value = PIECE_VALUES[piece];
            if (piece != KING) {
                int pst_idx = (color == WHITE) ? sq : (sq ^ 56);
                value += PST_TABLES[piece][pst_idx];
                if (piece != PAWN) {
                    Bitboard attacks = board.get_attacks_by(sq, piece, color);
                    value += count_bits(attacks) * MOBILITY_BONUS[piece];
                }
            }
            score += (color == WHITE) ? value : -value;
        }
    }
    return score;
}

int ChessEngine::quiescence(ChessBoard& board, int alpha, int beta, int depth_left) const {
    if (stop_search) return 0;
    int eval = evaluate_material(board);
    if (board.get_side_to_move() == BLACK) eval = -eval;
    if (depth_left <= 0) return eval;
    if (eval >= beta) return beta;
    if (eval > alpha) alpha = eval;

    std::vector<Move> moves = board.generate_legal_moves();
    order_moves_simple(board, moves);

    for (const Move& move : moves) {
        if (board.get_piece(move.to) == NONE) continue;
        board.make_move(move);
        int score = -quiescence(board, -beta, -alpha, depth_left - 1);
        board.unmake_move();
        if (stop_search) return 0;
        if (score >= beta) return beta;
        if (score > alpha) alpha = score;
    }
    return alpha;
}

int ChessEngine::negamax(ChessBoard& board, int depth, int ply, int alpha, int beta) const {
    if ((ply & 2047) == 0) { 
        auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time).count() > TIME_LIMIT_MS) stop_search = true;
    }
    if (stop_search) return 0;

    int tt_score; Move tt_move;
    if (tt.probe(board.get_hash(), depth, alpha, beta, tt_score, tt_move)) {
        if (ply > 0) return tt_score; 
    }

    Color side = board.get_side_to_move();
    bool in_check = board.is_check(side);

    if (depth <= 0) return quiescence(board, alpha, beta, 4);

    std::vector<Move> moves = board.generate_legal_moves();
    if (moves.empty()) {
        if (in_check) return -MATE_SCORE + ply; 
        return 0; 
    }

    order_moves(board, moves, ply, tt_move);

    int moves_searched = 0;
    int lmp_limit = 5 + (depth * depth);
    auto* non_const_this = const_cast<ChessEngine*>(this);

    int best_val = -INFINITY_SCORE;
    Move best_move_this_node;
    TTFlag flag = TT_ALPHA;

    for (const Move& move : moves) {
        bool is_capture = (board.get_piece(move.to) != NONE);
        if (!in_check && depth <= 3 && !is_capture && moves_searched > lmp_limit) { continue; }

        board.make_move(move);
        int score = -negamax(board, depth - 1, ply + 1, -beta, -alpha);
        board.unmake_move();
        
        if (stop_search) return 0;
        moves_searched++;

        if (score > best_val) {
            best_val = score;
            best_move_this_node = move;
        }

        if (score > alpha) {
            alpha = score;
            flag = TT_EXACT;
        }

        if (alpha >= beta) { 
            if (!is_capture) {
                if (ply < 20 && !(move == killer_moves[ply][0])) {
                    killer_moves[ply][1] = killer_moves[ply][0];
                    killer_moves[ply][0] = move;
                }
                if (move.from < 64 && move.to < 64) {
                    non_const_this->history_moves[move.from][move.to] += depth * depth;
                    if (history_moves[move.from][move.to] > 20000) non_const_this->history_moves[move.from][move.to] /= 2;
                }
            }
            flag = TT_BETA;
            break; 
        }
    }
    
    if (!stop_search) {
        tt.store(board.get_hash(), depth, best_val, flag, best_move_this_node);
    }
    
    return best_val;
}

Move ChessEngine::get_best_move(const ChessBoard& board) {
    // [CORREÇÃO] Criar uma cópia mutável do tabuleiro
    ChessBoard search_board = board;

    std::vector<Move> legal_moves = search_board.generate_legal_moves();
    if (legal_moves.empty()) return Move();

    std::memset(history_moves, 0, sizeof(history_moves));
    for(int i=0; i<20; i++) { killer_moves[i][0] = Move(); killer_moves[i][1] = Move(); }
    
    start_time = std::chrono::steady_clock::now();
    stop_search = false;
    
    Move best_move_global = legal_moves[0];
    
    for (int depth = 1; depth <= 20; depth++) {
        int alpha = -INFINITY_SCORE;
        int beta = INFINITY_SCORE;
        
        // Passa a cópia mutável para o negamax
        int score = negamax(search_board, depth, 0, alpha, beta);

        if (stop_search) break; 

        Move tt_move; int s;
        // Usa o hash da cópia (que é igual ao original pois negamax restaura o estado)
        if (tt.probe(search_board.get_hash(), depth, -INFINITY_SCORE, INFINITY_SCORE, s, tt_move)) {
            best_move_global = tt_move;
        }
        
        // [ATUALIZAÇÃO] Salva o score para a GUI
        last_eval_score = score;

        std::cout << "info depth " << depth << " score cp " << score << " pv " << best_move_global.to_string() << std::endl;
        if (std::abs(score) > MATE_SCORE - 100) break;
    }
    
    return best_move_global;
}

Move ChessEngine::get_random_move(const ChessBoard& board) {
    std::vector<Move> moves = board.generate_legal_moves();
    if (moves.empty()) return Move();
    std::uniform_int_distribution<size_t> dist(0, moves.size() - 1);
    return moves[dist(rng)];
}

bool ChessEngine::has_legal_moves(const ChessBoard& board) const {
    return !board.generate_legal_moves().empty();
}