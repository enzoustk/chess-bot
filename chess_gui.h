#ifndef CHESS_GUI_H
#define CHESS_GUI_H

#ifdef USE_SFML
#include <SFML/Graphics.hpp>
#include "chess.h"
#include "chess_engine.h"
#include <vector>
#include <map>
#include <string>
#include <thread>
#include <atomic>
#include <mutex>
#include <chrono> // Necessário para time_point

enum GameStateGUI {
    MENU,
    PLAYING,
    GAME_OVER
};

class ChessGUI {
private:
    sf::RenderWindow window;
    sf::Font font;
    
    // Elementos do Jogo
    ChessBoard board;
    ChessEngine engine;
    
    // Controle de Thread (Para não travar)
    std::thread engine_thread;
    std::atomic<bool> is_thinking;
    std::atomic<bool> move_ready;
    Move calculated_move;

    // Estado
    GameStateGUI current_state;
    Color player_color; 
    bool game_started;
    bool game_ended;
    Color winner;
    bool selecting_time;  // Indica se está na fase de seleção de tempo
    
    // Tempo
    float white_time_seconds;
    float black_time_seconds;
    float initial_time_seconds;
    // [CORREÇÃO] Variável que estava faltando:
    std::chrono::steady_clock::time_point move_start_time;
    
    // Seleção e Movimento
    Square selected_square;
    bool is_square_selected;
    std::vector<Move> legal_moves_for_selected;
    Move last_move;
    bool has_last_move;
    
    // Promoção
    bool awaiting_promotion;
    Move pending_promotion_move;
    Square promotion_square;
    
    struct Theme {
        sf::Color light_square = sf::Color(240, 217, 181);
        sf::Color dark_square = sf::Color(181, 136, 99);
        sf::Color bg_color = sf::Color(22, 21, 18);
        sf::Color sidebar_color = sf::Color(38, 36, 33);
        sf::Color highlight_color = sf::Color(155, 199, 0, 100); // Lichess-like highlight
        sf::Color move_highlight_color = sf::Color(155, 199, 0, 100);
        sf::Color check_color = sf::Color(200, 50, 50, 200);
        sf::Color legal_move_color = sf::Color(0, 0, 0, 40);
        sf::Color text_color = sf::Color(240, 240, 240);
        sf::Color button_color = sf::Color(60, 60, 60);
        sf::Color button_hover_color = sf::Color(80, 80, 80);
    } theme;

    struct AnimatingPiece {
        PieceType piece;
        Color color;
        sf::Vector2f start_pos;
        sf::Vector2f end_pos;
        std::chrono::steady_clock::time_point start_time;
        float duration_seconds;
    };

    std::vector<AnimatingPiece> animations;

    // Recursos Gráficos
    std::map<std::pair<PieceType, Color>, sf::Texture> piece_textures;
    
    // Textos
    sf::Text status_text;
    sf::Text move_text;
    sf::Text white_clock_text;
    sf::Text black_clock_text;
    
    // Capturas
    std::vector<PieceType> captured_white;
    std::vector<PieceType> captured_black;
    
    // Avaliação
    int current_eval = 0;

    // Constantes
    // Constantes (agora usadas apenas como referência ou mínimos)
    static const int MIN_WINDOW_WIDTH = 800;
    static const int MIN_WINDOW_HEIGHT = 600;
    
    // --- FUNÇÕES ---
    int get_board_size() const;
    int get_square_size() const;
    int get_panel_width() const;
    float get_scale_factor() const;
    unsigned int get_font_size(int base_size) const;
    bool is_white_at_bottom() const { return player_color == WHITE; }
    
    // Desenho
    void draw_board();
    void draw_pieces();
    void draw_piece(sf::RenderTarget& target, PieceType pt, Color c, int x, int y);
    void draw_piece(sf::RenderTarget& target, PieceType pt, Color c, int x, int y, float custom_size); // Sobrecarga
    void draw_highlights();
    void draw_legal_moves();
    void draw_last_move();
    void draw_check_indicator();
    void draw_menu();
    void draw_game_over();
    void draw_promotion_menu();
    void draw_clocks_and_captured();
    void draw_thinking_indicator();
    
    // Nova Interface
    void draw_sidebar();
    void draw_player_card(float x, float y, float w, float h, Color player, float time_left, const std::vector<PieceType>& captured);
    int calculate_material_score(const std::vector<PieceType>& captured);
    
    // Lógica
    bool load_piece_textures();
    std::string get_piece_image_path(PieceType pt, Color c) const;
    std::string get_piece_symbol(PieceType pt, Color c) const;
    std::string format_time(int seconds) const;
    Square get_square_from_mouse(int x, int y) const;
    
    // Eventos
    void handle_mouse_click(int x, int y);
    void handle_menu_click(int x, int y);
    void handle_game_over_click(int x, int y);
    void handle_promotion_click(int x, int y);
    bool handle_resign_button_click(int x, int y);
    
    // Engine & Jogo
    void start_engine_thinking();
    void engine_worker(ChessBoard board_copy);
    void apply_engine_move();
    
    void update_clocks();
    void update_status_text();
    void reset_game();
    void start_game();
    void resign(Color c);
    Square find_king_square(Color c) const;

public:
    ChessGUI();
    ~ChessGUI();
    void run();
    void render();
    void handle_events();
};

#endif // USE_SFML
#endif // CHESS_GUI_H