#ifndef CHESS_GUI_H
#define CHESS_GUI_H

#include "chess.h"
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include <map>
#include <chrono>

class ChessGUI {
private:
    sf::RenderWindow window;
    ChessBoard board;
    
    // Dimensões (valores padrão, calculados dinamicamente)
    static const int DEFAULT_BOARD_SIZE = 640;
    static const int DEFAULT_PANEL_WIDTH = 200;
    
    // Funções para calcular dimensões dinamicamente
    int get_board_size() const;
    int get_square_size() const;
    int get_panel_width() const;
    float get_scale_factor() const;  // Fator de escala baseado no tamanho da janela
    
    // Cores
    sf::Color light_square = sf::Color(240, 217, 181);
    sf::Color dark_square = sf::Color(181, 136, 99);
    sf::Color highlight_color = sf::Color(255, 255, 0, 150);
    sf::Color legal_move_color = sf::Color(0, 255, 0, 100);
    sf::Color selected_color = sf::Color(255, 255, 0, 200);
    sf::Color last_move_color = sf::Color(255, 200, 0, 180);  // Cor para destacar último movimento
    sf::Color check_color = sf::Color(255, 0, 0, 150);  // Cor vermelha transparente para xeque
    
    // Estados do jogo
    enum GameState {
        MENU,
        PLAYING
    };
    GameState current_state;
    
    // Tempo dos jogadores (em segundos)
    int white_time_seconds;
    int black_time_seconds;
    std::chrono::steady_clock::time_point move_start_time;
    bool game_started;
    bool game_ended;  // Se o jogo terminou (tempo esgotado, xeque-mate, etc)
    Color winner;  // Vencedor do jogo
    
    // Peças capturadas
    std::vector<PieceType> captured_white;  // Peças brancas capturadas (por pretas)
    std::vector<PieceType> captured_black;  // Peças pretas capturadas (por brancas)
    
    // Estado da interface
    Square selected_square;
    std::vector<Move> legal_moves_for_selected;  // Armazenar movimentos completos, não apenas destinos
    bool is_square_selected;
    Move last_move;  // Último movimento feito
    bool has_last_move;  // Se há um último movimento para destacar
    
    // Fontes e textos
    sf::Font font;
    sf::Text status_text;
    sf::Text move_text;
    
    // Texturas das peças
    std::map<std::pair<PieceType, Color>, sf::Texture> piece_textures;
    bool load_piece_textures();
    
    // Funções auxiliares
    Square get_square_from_mouse(int mouse_x, int mouse_y) const;
    void draw_board();
    void draw_pieces();
    void draw_highlights();
    void draw_legal_moves();
    void draw_last_move();  // Destacar casas de origem e destino do último movimento
    void draw_check_indicator();  // Desenhar círculo vermelho quando rei está em xeque
    Square find_king_square(Color c) const;  // Encontrar posição do rei
    void update_status_text();
    void handle_mouse_click(int mouse_x, int mouse_y);
    
    // Menu e configuração
    void draw_menu();
    void handle_menu_click(int mouse_x, int mouse_y);
    void start_game();
    
    // Relógio e peças capturadas
    void update_clocks();
    void draw_clocks_and_captured();
    std::string format_time(int seconds) const;
    bool is_white_at_bottom() const;  // Verificar se brancas estão embaixo
    bool handle_resign_button_click(int mouse_x, int mouse_y);  // Retorna true se clicou em botão
    void resign(Color player);
    void draw_piece(sf::RenderTarget& target, PieceType pt, Color c, int x, int y);
    void draw_piece(sf::RenderTarget& target, PieceType pt, Color c, int x, int y, float custom_size);
    std::string get_piece_symbol(PieceType pt, Color c) const;
    std::string get_piece_image_path(PieceType pt, Color c) const;
    
public:
    ChessGUI();
    ~ChessGUI();
    
    void run();
    void handle_events();
    void render();
};

#endif // CHESS_GUI_H

