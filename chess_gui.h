#ifndef CHESS_GUI_H
#define CHESS_GUI_H

#include "chess.h"
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include <map>

class ChessGUI {
private:
    sf::RenderWindow window;
    ChessBoard board;
    
    // Dimensões
    static const int BOARD_SIZE = 640;
    static const int SQUARE_SIZE = BOARD_SIZE / 8;
    static const int WINDOW_WIDTH = BOARD_SIZE + 200; // Espaço para informações
    static const int WINDOW_HEIGHT = BOARD_SIZE;
    
    // Cores
    sf::Color light_square = sf::Color(240, 217, 181);
    sf::Color dark_square = sf::Color(181, 136, 99);
    sf::Color highlight_color = sf::Color(255, 255, 0, 150);
    sf::Color legal_move_color = sf::Color(0, 255, 0, 100);
    sf::Color selected_color = sf::Color(255, 255, 0, 200);
    
    // Estado da interface
    Square selected_square;
    std::vector<Move> legal_moves_for_selected;  // Armazenar movimentos completos, não apenas destinos
    bool is_square_selected;
    
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
    void update_status_text();
    void handle_mouse_click(int mouse_x, int mouse_y);
    void draw_piece(sf::RenderTarget& target, PieceType pt, Color c, int x, int y);
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

