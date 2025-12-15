#ifndef CHESS_ENGINE_H
#define CHESS_ENGINE_H

#include "chess.h"
#include <vector>
#include <random>
#include <chrono>

// Tipos de flags para a Transposition Table
enum TTFlag {
    TT_EXACT,
    TT_ALPHA, // Upper Bound
    TT_BETA   // Lower Bound
};

// Entrada da Tabela
struct TTEntry {
    uint64_t key;
    int score;
    int depth;
    TTFlag flag;
    Move best_move;
};

// Classe da Tabela de Transposição
class TranspositionTable {
private:
    std::vector<TTEntry> table;
    size_t size;

public:
    TranspositionTable(size_t size_mb = 64) {
        // Tamanho de cada entrada = 24 bytes aprox.
        // 64MB dá cerca de 2.6 milhões de entradas
        size = (size_mb * 1024 * 1024) / sizeof(TTEntry);
        table.resize(size);
    }

    void clear() {
        std::fill(table.begin(), table.end(), TTEntry{0, 0, 0, TT_EXACT, Move()});
    }

    // Armazenar
    void store(uint64_t key, int depth, int score, TTFlag flag, Move best_move) {
        size_t index = key % size;
        // Substituição simples: profundidade maior ou igual substitui
        if (table[index].key == 0 || depth >= table[index].depth) {
            table[index] = {key, score, depth, flag, best_move};
        }
    }

    // Recuperar
    bool probe(uint64_t key, int depth, int alpha, int beta, int& score, Move& best_move) {
        size_t index = key % size;
        const TTEntry& entry = table[index];

        if (entry.key == key) {
            best_move = entry.best_move; // Sempre útil para ordenação
            if (entry.depth >= depth) {
                if (entry.flag == TT_EXACT) {
                    score = entry.score;
                    return true;
                }
                if (entry.flag == TT_ALPHA && entry.score <= alpha) {
                    score = alpha;
                    return true;
                }
                if (entry.flag == TT_BETA && entry.score >= beta) {
                    score = beta;
                    return true;
                }
            }
        }
        return false;
    }
};

class ChessEngine {
private:
    std::mt19937 rng;

    mutable int history_moves[64][64];
    mutable Move killer_moves[20][2];
    mutable bool stop_search;
    mutable std::chrono::time_point<std::chrono::steady_clock> start_time;
    
    // [NOVO] Instância da TT
    mutable TranspositionTable tt;

    static const int PIECE_VALUES[7];

    int evaluate_material(const ChessBoard& board) const;
    void order_moves(const ChessBoard& board, std::vector<Move>& moves, int ply, const Move& tt_move) const;
    void order_moves_simple(const ChessBoard& board, std::vector<Move>& moves) const;
    int quiescence(ChessBoard& board, int alpha, int beta, int depth_left) const;
    int negamax(ChessBoard& board, int depth, int ply, int alpha, int beta) const;

    int ChessEngine::eval_pawns(const ChessBoard &board) const;

    
    // [NOVO] Armazenar última avaliação
    mutable int last_eval_score;

public:
    ChessEngine();
    
    Move get_best_move(const ChessBoard& board);
    int get_last_eval() const { return last_eval_score; } // Getter
    Move get_random_move(const ChessBoard& board);
    bool has_legal_moves(const ChessBoard& board) const;
};

#endif // CHESS_ENGINE_H