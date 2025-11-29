#include "chess_gui.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <iomanip>

ChessGUI::ChessGUI() 
    : window(sf::VideoMode(DEFAULT_BOARD_SIZE + DEFAULT_PANEL_WIDTH, DEFAULT_BOARD_SIZE), 
             "Jogo de Xadrez", sf::Style::Close | sf::Style::Resize),
      current_state(MENU),
      white_time_seconds(600),  // 10 minutos padrão
      black_time_seconds(600),
      initial_time_seconds(600),
      game_started(false),
      game_ended(false),
      winner(WHITE),  // Valor padrão, será atualizado
      player_color(WHITE), // Padrão: jogar com as brancas
      selected_square(NO_SQUARE),
      is_square_selected(false),
      has_last_move(false) {
    
    window.setFramerateLimit(60);
    // Definir tamanho mínimo da janela
    window.setSize(sf::Vector2u(640, 480));
    
    // Tentar carregar fonte do sistema
    // Windows
    #ifdef _WIN32
        if (!font.loadFromFile("C:/Windows/Fonts/arial.ttf") &&
            !font.loadFromFile("C:/Windows/Fonts/calibri.ttf")) {
            // Se não encontrar, tentar criar uma fonte básica
        }
    #elif __APPLE__
        if (!font.loadFromFile("/System/Library/Fonts/Helvetica.ttc") &&
            !font.loadFromFile("/Library/Fonts/Arial.ttf")) {
        }
    #else
        // Linux
        if (!font.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf") &&
            !font.loadFromFile("/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf")) {
        }
    #endif
    
    // Se a fonte não foi carregada, usar a fonte padrão do SFML
    // (pode não funcionar perfeitamente, mas pelo menos não vai crashar)
    status_text.setFont(font);
    status_text.setFillColor(sf::Color::White);
    
    move_text.setFont(font);
    move_text.setFillColor(sf::Color::White);
    
    // Carregar texturas das peças
    if (!load_piece_textures()) {
        std::cerr << "Aviso: Não foi possível carregar todas as texturas das peças.\n";
        std::cerr << "Usando símbolos Unicode como fallback.\n";
    } else {
        std::cout << "Texturas das peças carregadas com sucesso (" 
                  << piece_textures.size() << " texturas).\n";
    }
    
    // Não atualizar status text no menu
    if (current_state == PLAYING) {
        update_status_text();
    }
}

// Funções para calcular dimensões dinamicamente
int ChessGUI::get_board_size() const {
    sf::Vector2u window_size = window.getSize();
    // O tabuleiro deve ser quadrado e ocupar a maior parte da altura
    // Deixar espaço para o painel lateral
    int available_width = window_size.x - get_panel_width();
    int board_size = std::min(available_width, static_cast<int>(window_size.y));
    // Garantir que seja múltiplo de 8 para manter casas inteiras
    return (board_size / 8) * 8;
}

int ChessGUI::get_square_size() const {
    return get_board_size() / 8;
}

int ChessGUI::get_panel_width() const {
    sf::Vector2u window_size = window.getSize();
    // Painel lateral proporcional, mínimo 150px, máximo 300px
    int panel = window_size.x / 5;
    if (panel < 150) panel = 150;
    if (panel > 300) panel = 300;
    return panel;
}

float ChessGUI::get_scale_factor() const {
    // Fator de escala baseado no tamanho padrão
    return static_cast<float>(get_board_size()) / DEFAULT_BOARD_SIZE;
}

ChessGUI::~ChessGUI() {
    if (window.isOpen()) {
        window.close();
    }
}

Square ChessGUI::get_square_from_mouse(int mouse_x, int mouse_y) const {
    int board_size = get_board_size();
    int square_size = get_square_size();
    
    if (mouse_x < 0 || mouse_x >= board_size || mouse_y < 0 || mouse_y >= board_size) {
        return NO_SQUARE;
    }
    
    int col = mouse_x / square_size;
    int row = mouse_y / square_size;
    
    int file, rank;
    
    if (is_white_at_bottom()) {
        file = col;
        rank = 7 - row;
    } else {
        file = 7 - col;
        rank = row;
    }
    
    return ChessBoard::make_square(file, rank);
}

void ChessGUI::draw_board() {
    int square_size = get_square_size();
    bool white_bottom = is_white_at_bottom();
    
    for (int rank = 0; rank < 8; rank++) {
        for (int file = 0; file < 8; file++) {
            sf::RectangleShape square(sf::Vector2f(square_size, square_size));
            
            int draw_file = white_bottom ? file : (7 - file);
            int draw_rank = white_bottom ? rank : (7 - rank);
            
            square.setPosition(draw_file * square_size, (7 - draw_rank) * square_size);
            
            // Alternar cores (a1 deve ser escura)
            if ((rank + file) % 2 == 0) {
                square.setFillColor(dark_square);
            } else {
                square.setFillColor(light_square);
            }
            
            window.draw(square);
        }
    }
}

std::string ChessGUI::get_piece_symbol(PieceType pt, Color c) const {
    const char* white_pieces = "PNBRQK";
    const char* black_pieces = "pnbrqk";
    
    if (pt == NONE) return "";
    
    if (c == WHITE) {
        return std::string(1, white_pieces[pt]);
    } else {
        return std::string(1, black_pieces[pt]);
    }
}

std::string ChessGUI::get_piece_image_path(PieceType pt, Color c) const {
    if (pt == NONE) return "";
    
    std::string color_str = (c == WHITE) ? "white" : "black";
    std::string piece_str;
    
    switch (pt) {
        case PAWN:   piece_str = "pawn"; break;
        case KNIGHT: piece_str = "knight"; break;
        case BISHOP: piece_str = "bishop"; break;
        case ROOK:   piece_str = "rook"; break;
        case QUEEN:  piece_str = "queen"; break;
        case KING:   piece_str = "king"; break;
        default:     return "";
    }
    
    return "img/pieces/" + color_str + "-" + piece_str + ".png";
}

bool ChessGUI::load_piece_textures() {
    bool all_loaded = true;
    
    // Carregar texturas para todas as peças
    for (int color = WHITE; color <= BLACK; color++) {
        for (int piece = PAWN; piece <= KING; piece++) {
            PieceType pt = static_cast<PieceType>(piece);
            Color c = static_cast<Color>(color);
            
            std::string path = get_piece_image_path(pt, c);
            sf::Texture texture;
            
            if (texture.loadFromFile(path)) {
                // Copiar a textura (SFML permite cópia de texturas)
                piece_textures[{pt, c}] = texture;
            } else {
                std::cerr << "Erro ao carregar textura: " << path << std::endl;
                all_loaded = false;
            }
        }
    }
    
    return all_loaded;
}

void ChessGUI::draw_piece(sf::RenderTarget& target, PieceType pt, Color c, int x, int y) {
    if (pt == NONE) return;
    
    int square_size = get_square_size();
    
    // Tentar usar textura da imagem
    auto it = piece_textures.find({pt, c});
    if (it != piece_textures.end()) {
        const sf::Texture& texture = it->second;
        sf::Vector2u texture_size = texture.getSize();
        
        // Verificar se a textura é válida
        if (texture_size.x > 0 && texture_size.y > 0) {
            sf::Sprite piece_sprite(texture);
            
            // Escalar a imagem para caber no quadrado (deixar um pequeno padding)
            float padding = square_size * 0.015f;  // Padding proporcional
            float scale = (square_size - padding) / static_cast<float>(std::max(texture_size.x, texture_size.y));
            piece_sprite.setScale(scale, scale);
            
            // Centralizar a peça no quadrado
            sf::FloatRect sprite_bounds = piece_sprite.getLocalBounds();
            float scaled_width = sprite_bounds.width * scale;
            float scaled_height = sprite_bounds.height * scale;
            
            piece_sprite.setPosition(
                x + (square_size - scaled_width) / 2 - sprite_bounds.left * scale,
                y + (square_size - scaled_height) / 2 - sprite_bounds.top * scale
            );
            
            target.draw(piece_sprite);
            return;
        }
    }
    
    // Fallback: usar texto Unicode se a textura não foi carregada
    sf::Text piece_text;
    piece_text.setFont(font);
    piece_text.setCharacterSize(static_cast<unsigned int>(square_size * 0.75f));
    piece_text.setFillColor(c == WHITE ? sf::Color::White : sf::Color::Black);
    piece_text.setStyle(sf::Text::Bold);
    
    // Símbolos Unicode para peças de xadrez (usando string normal)
    std::string piece_symbol;
    switch (pt) {
        case PAWN:
            piece_symbol = (c == WHITE) ? "\u2659" : "\u265F";
            break;
        case KNIGHT:
            piece_symbol = (c == WHITE) ? "\u2658" : "\u265E";
            break;
        case BISHOP:
            piece_symbol = (c == WHITE) ? "\u2657" : "\u265D";
            break;
        case ROOK:
            piece_symbol = (c == WHITE) ? "\u2656" : "\u265C";
            break;
        case QUEEN:
            piece_symbol = (c == WHITE) ? "\u2655" : "\u265B";
            break;
        case KING:
            piece_symbol = (c == WHITE) ? "\u2654" : "\u265A";
            break;
        default:
            return;
    }
    
    piece_text.setString(piece_symbol);
    
    // Centralizar o texto no quadrado
    sf::FloatRect text_bounds = piece_text.getLocalBounds();
    piece_text.setPosition(
        x + (square_size - text_bounds.width) / 2 - text_bounds.left,
        y + (square_size - text_bounds.height) / 2 - text_bounds.top
    );
    
    target.draw(piece_text);
}

void ChessGUI::draw_pieces() {
    int square_size = get_square_size();
    bool white_bottom = is_white_at_bottom();
    
    for (int rank = 0; rank < 8; rank++) {
        for (int file = 0; file < 8; file++) {
            Square sq = ChessBoard::make_square(file, rank);
            PieceType pt = board.get_piece(sq);
            
            if (pt != NONE) {
                Color c = board.get_piece_color(sq);
                
                int draw_file = white_bottom ? file : (7 - file);
                int draw_rank = white_bottom ? rank : (7 - rank);
                
                int x = draw_file * square_size;
                int y = (7 - draw_rank) * square_size;
                draw_piece(window, pt, c, x, y);
            }
        }
    }
}

void ChessGUI::draw_legal_moves() {
    if (!is_square_selected) return;
    
    int square_size = get_square_size();
    float indicator_radius = square_size * 0.125f;  // 12.5% do tamanho da casa
    bool white_bottom = is_white_at_bottom();
    
    for (const Move& move : legal_moves_for_selected) {
        int file = ChessBoard::get_file(move.to);
        int rank = ChessBoard::get_rank(move.to);
        
        int draw_file = white_bottom ? file : (7 - file);
        int draw_rank = white_bottom ? rank : (7 - rank);
        
        sf::CircleShape indicator(indicator_radius);
        indicator.setFillColor(legal_move_color);
        indicator.setPosition(
            draw_file * square_size + square_size / 2 - indicator_radius,
            (7 - draw_rank) * square_size + square_size / 2 - indicator_radius
        );
        
        window.draw(indicator);
    }
}

void ChessGUI::draw_last_move() {
    if (!has_last_move) return;
    
    int square_size = get_square_size();
    bool white_bottom = is_white_at_bottom();
    
    // Destacar casa de origem
    if (last_move.from != NO_SQUARE) {
        int from_file = ChessBoard::get_file(last_move.from);
        int from_rank = ChessBoard::get_rank(last_move.from);
        
        int draw_file = white_bottom ? from_file : (7 - from_file);
        int draw_rank = white_bottom ? from_rank : (7 - from_rank);
        
        sf::RectangleShape highlight_from(sf::Vector2f(square_size, square_size));
        highlight_from.setFillColor(last_move_color);
        highlight_from.setPosition(draw_file * square_size, (7 - draw_rank) * square_size);
        window.draw(highlight_from);
    }
    
    // Destacar casa de destino
    if (last_move.to != NO_SQUARE) {
        int to_file = ChessBoard::get_file(last_move.to);
        int to_rank = ChessBoard::get_rank(last_move.to);
        
        int draw_file = white_bottom ? to_file : (7 - to_file);
        int draw_rank = white_bottom ? to_rank : (7 - to_rank);
        
        sf::RectangleShape highlight_to(sf::Vector2f(square_size, square_size));
        highlight_to.setFillColor(last_move_color);
        highlight_to.setPosition(draw_file * square_size, (7 - draw_rank) * square_size);
        window.draw(highlight_to);
    }
}

Square ChessGUI::find_king_square(Color c) const {
    // Procurar o rei em todas as casas
    for (int rank = 0; rank < 8; rank++) {
        for (int file = 0; file < 8; file++) {
            Square sq = ChessBoard::make_square(file, rank);
            PieceType pt = board.get_piece(sq);
            if (pt == KING && board.get_piece_color(sq) == c) {
                return sq;
            }
        }
    }
    return NO_SQUARE;
}

void ChessGUI::draw_check_indicator() {
    // Verificar se o lado que está jogando está em xeque
    Color side_to_move = board.get_side_to_move();
    if (board.is_check(side_to_move)) {
        Square king_sq = find_king_square(side_to_move);
        if (king_sq != NO_SQUARE) {
            int square_size = get_square_size();
            bool white_bottom = is_white_at_bottom();
            
            int file = ChessBoard::get_file(king_sq);
            int rank = ChessBoard::get_rank(king_sq);
            
            int draw_file = white_bottom ? file : (7 - file);
            int draw_rank = white_bottom ? rank : (7 - rank);
            
            // Desenhar círculo vermelho transparente no centro da casa
            float radius = square_size * 0.4f;  // 40% do tamanho da casa
            sf::CircleShape check_circle(radius);
            check_circle.setFillColor(check_color);
            check_circle.setPosition(
                draw_file * square_size + (square_size - radius * 2) / 2,
                (7 - draw_rank) * square_size + (square_size - radius * 2) / 2
            );
            
            window.draw(check_circle);
        }
    }
}

void ChessGUI::draw_highlights() {
    if (is_square_selected && selected_square != NO_SQUARE) {
        int square_size = get_square_size();
        bool white_bottom = is_white_at_bottom();
        
        int file = ChessBoard::get_file(selected_square);
        int rank = ChessBoard::get_rank(selected_square);
        
        int draw_file = white_bottom ? file : (7 - file);
        int draw_rank = white_bottom ? rank : (7 - rank);
        
        sf::RectangleShape highlight(sf::Vector2f(square_size, square_size));
        highlight.setFillColor(selected_color);
        highlight.setPosition(draw_file * square_size, (7 - draw_rank) * square_size);
        
        window.draw(highlight);
    }
}

void ChessGUI::update_status_text() {
    std::ostringstream status;
    Color side = board.get_side_to_move();
    
    status << "Vez de: " << (side == WHITE ? "BRANCAS" : "PRETAS") << "\n";
    status << "Jogada: " << board.get_fullmove_number() << "\n\n";
    
    if (board.is_check(side)) {
        status << "⚠ XEQUE!\n\n";
    }
    
    if (board.is_checkmate(side)) {
        status << "*** XEQUE-MATE! ***\n";
        status << "Vencedor: " << (side == WHITE ? "PRETAS" : "BRANCAS") << "\n";
    } else if (board.is_stalemate(side)) {
        status << "*** EMPATE ***\n";
        status << "(Afogamento)\n";
    }
    
    status << "\nClique em uma peça\n";
    status << "para selecioná-la,\n";
    status << "depois clique no\n";
    status << "destino para mover.\n\n";
    status << "ESC: Desfazer\n";
    status << "R: Reiniciar";
    
    status_text.setString(status.str());
    
    // Mostrar últimos movimentos
    std::ostringstream moves;
    moves << "Movimentos legais:\n";
    std::vector<Move> legal_moves = board.generate_legal_moves();
    int count = 0;
    for (const auto& move : legal_moves) {
        if (count < 10) { // Mostrar apenas os primeiros 10
            moves << move.to_string() << " ";
            count++;
        }
    }
    if (legal_moves.size() > 10) {
        moves << "...\n";
        moves << "Total: " << legal_moves.size();
    }
    
    move_text.setString(moves.str());
}

void ChessGUI::handle_mouse_click(int mouse_x, int mouse_y) {
    if (current_state == MENU) {
        handle_menu_click(mouse_x, mouse_y);
        return;
    }
    
    if (game_ended) {
        handle_game_over_click(mouse_x, mouse_y);
        return;
    }

    Square clicked_square = get_square_from_mouse(mouse_x, mouse_y);
    
    if (clicked_square == NO_SQUARE) return;
    
    if (!is_square_selected) {
        // Selecionar uma peça
        PieceType pt = board.get_piece(clicked_square);
        if (pt != NONE) {
            Color piece_color = board.get_piece_color(clicked_square);
            if (piece_color == board.get_side_to_move()) {
                selected_square = clicked_square;
                is_square_selected = true;
                
                // Calcular movimentos legais para esta peça
                legal_moves_for_selected.clear();
                std::vector<Move> all_moves = board.generate_legal_moves();
                for (const auto& move : all_moves) {
                    if (move.from == selected_square) {
                        legal_moves_for_selected.push_back(move);  // Armazenar movimento completo
                    }
                }
            }
        }
    } else {
        // Tentar fazer movimento
        bool move_made = false;
        for (const Move& legal_move : legal_moves_for_selected) {
            if (legal_move.to == clicked_square) {
                // Usar o movimento completo da lista (preserva is_castle, is_en_passant, etc.)
                Move move = legal_move;
                
                // Verificar se é promoção (pode já estar definida, mas garantir)
                PieceType pt = board.get_piece(selected_square);
                if (pt == PAWN) {
                    int rank = ChessBoard::get_rank(clicked_square);
                    if ((board.get_piece_color(selected_square) == WHITE && rank == 7) ||
                        (board.get_piece_color(selected_square) == BLACK && rank == 0)) {
                        // Promoção - sempre para dama por padrão se não estiver definida
                        if (move.promotion == NONE) {
                            move.promotion = QUEEN;
                        }
                    }
                }
                
                // Verificar peça capturada ANTES de fazer o movimento
                PieceType captured = board.get_piece(move.to);
                Color captured_color = (captured != NONE) ? board.get_piece_color(move.to) : WHITE;
                
                if (board.make_move(move)) {
                    move_made = true;
                    // Armazenar último movimento para destacar
                    last_move = move;
                    has_last_move = true;
                    
                    // Verificar se o jogo terminou (xeque-mate ou empate)
                    Color current_side = board.get_side_to_move();
                    if (board.is_checkmate(current_side)) {
                        game_ended = true;
                        winner = (current_side == WHITE) ? BLACK : WHITE;
                        game_started = false;
                        std::cout << "Xeque-mate! Vencedor: " << (winner == WHITE ? "Brancas" : "Pretas") << "\n";
                    } else if (board.is_stalemate(current_side)) {
                        game_ended = true;
                        game_started = false;
                        std::cout << "Empate por afogamento!\n";
                    }
                    
                    // Rastrear peças capturadas
                    if (captured != NONE) {
                        if (captured_color == WHITE) {
                            captured_white.push_back(captured);
                        } else {
                            captured_black.push_back(captured);
                        }
                    } else if (move.is_en_passant) {
                        // En passant sempre captura um peão
                        // O lado que fez o movimento capturou o peão do oponente
                        Color mover_color = (board.get_side_to_move() == WHITE) ? BLACK : WHITE;
                        if (mover_color == WHITE) {
                            captured_black.push_back(PAWN);
                        } else {
                            captured_white.push_back(PAWN);
                        }
                    }
                    
                    // Atualizar relógio
                    update_clocks();
                    break;
                }
            }
        }
        
        // Desselecionar
        selected_square = NO_SQUARE;
        is_square_selected = false;
        legal_moves_for_selected.clear();
        
        if (move_made) {
            update_status_text();
        }
    }
}

void ChessGUI::handle_events() {
    sf::Event event;
    while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            window.close();
        } else if (event.type == sf::Event::Resized) {
            // Garantir tamanho mínimo
            if (event.size.width < 640) {
                window.setSize(sf::Vector2u(640, window.getSize().y));
            }
            if (event.size.height < 480) {
                window.setSize(sf::Vector2u(window.getSize().x, 480));
            }
            // Ajustar view para manter coordenadas corretas
            sf::FloatRect visibleArea(0, 0, static_cast<float>(event.size.width), static_cast<float>(event.size.height));
            window.setView(sf::View(visibleArea));
        } else if (event.type == sf::Event::MouseButtonPressed) {
            if (event.mouseButton.button == sf::Mouse::Left) {
                if (current_state == MENU) {
                    handle_menu_click(event.mouseButton.x, event.mouseButton.y);
                } else {
                    // Verificar se clicou em botão de desistência primeiro
                    if (!handle_resign_button_click(event.mouseButton.x, event.mouseButton.y)) {
                        handle_mouse_click(event.mouseButton.x, event.mouseButton.y);
                    }
                }
            }
        } else if (event.type == sf::Event::KeyPressed) {
            // Atalhos removidos - usar botões na interface
        }
    }
}

void ChessGUI::render() {
    window.clear(sf::Color(50, 50, 50));
    
    if (current_state == MENU) {
        draw_menu();
    } else {
        int board_size = get_board_size();
        int panel_width = get_panel_width();
        sf::Vector2u window_size = window.getSize();
        
        draw_board();
        draw_last_move();  // Destacar último movimento (atrás dos outros highlights)
        draw_highlights();
        draw_legal_moves();
        draw_pieces();
        draw_check_indicator();  // Desenhar indicador de xeque por cima das peças
        
        // Painel lateral com gradiente
        sf::RectangleShape panel(sf::Vector2f(panel_width, static_cast<float>(window_size.y)));
        panel.setFillColor(sf::Color(35, 35, 40));
        panel.setPosition(board_size, 0);
        
        // Borda superior decorativa
        sf::RectangleShape top_border(sf::Vector2f(panel_width, 3));
        top_border.setFillColor(sf::Color(70, 130, 180));
        top_border.setPosition(board_size, 0);
        window.draw(panel);
        window.draw(top_border);
        
        // Desenhar relógios e peças capturadas
        draw_clocks_and_captured();
        
        if (game_ended) {
            draw_game_over();
        }
    }
    
    window.display();
}

// Funções do menu
void ChessGUI::draw_menu() {
    sf::Vector2u window_size = window.getSize();
    float center_x = window_size.x / 2.0f;
    float center_y = window_size.y / 2.0f;
    
    // Calcular escala baseada no tamanho da janela (usar altura como referência)
    // Garantir que tudo caiba na tela
    float base_height = 800.0f;  // Altura de referência aumentada
    float base_width = 600.0f;   // Largura de referência
    float scale = std::min(window_size.y / base_height, window_size.x / base_width);
    scale = std::max(0.5f, std::min(1.2f, scale));  // Limitar entre 0.5 e 1.2 para garantir que caiba
    
    // Fundo com gradiente sutil
    sf::RectangleShape background(sf::Vector2f(static_cast<float>(window_size.x), static_cast<float>(window_size.y)));
    background.setFillColor(sf::Color(30, 30, 35));
    window.draw(background);
    
    // Tamanhos dinâmicos
    unsigned int title_size = static_cast<unsigned int>(52 * scale);
    unsigned int subtitle_size = static_cast<unsigned int>(20 * scale);
    unsigned int button_text_size = static_cast<unsigned int>(22 * scale);
    unsigned int start_text_size = static_cast<unsigned int>(28 * scale);
    
    // Calcular altura total do menu para centralização
    float title_h = 52 * scale;
    float subtitle_h = 20 * scale;
    float color_btn_h = 40 * scale;
    float time_btn_h = 45 * scale;
    float start_btn_h = 60 * scale;
    
    // Espaçamentos
    float sp_title_color = 50 * scale;
    float sp_color_sub_btn = 20 * scale;
    float sp_color_time = 40 * scale;
    float sp_time_sub_btn = 20 * scale;
    float sp_time_btns = 55 * scale; // Inclui altura do botão
    float sp_time_start = 30 * scale;
    
    float total_height = title_h + sp_title_color + subtitle_h + sp_color_sub_btn + color_btn_h + 
                         sp_color_time + subtitle_h + sp_time_sub_btn + (6 * sp_time_btns) + sp_time_start + start_btn_h;
                         
    // Ajustar escala se não couber
    if (total_height > window_size.y * 0.95f) {
        scale = scale * (window_size.y * 0.95f) / total_height;
        // Recalcular alturas com nova escala
        title_h = 52 * scale;
        subtitle_h = 20 * scale;
        color_btn_h = 40 * scale;
        time_btn_h = 45 * scale;
        start_btn_h = 60 * scale;
        sp_title_color = 50 * scale;
        sp_color_sub_btn = 20 * scale;
        sp_color_time = 40 * scale;
        sp_time_sub_btn = 20 * scale;
        sp_time_btns = 55 * scale;
        sp_time_start = 30 * scale;
        
        // Recalcular tamanhos de fonte
        title_size = static_cast<unsigned int>(52 * scale);
        subtitle_size = static_cast<unsigned int>(20 * scale);
        button_text_size = static_cast<unsigned int>(22 * scale);
        start_text_size = static_cast<unsigned int>(28 * scale);
        
        total_height = window_size.y * 0.95f;
    }
    
    float current_y = center_y - total_height / 2.0f;
    
    float button_width = 250 * scale;
    float start_width = 240 * scale;
    
    // Título com sombra
    sf::Text title_shadow("Jogo de Xadrez", font, title_size);
    title_shadow.setFillColor(sf::Color(0, 0, 0, 100));
    title_shadow.setStyle(sf::Text::Bold);
    sf::FloatRect title_bounds = title_shadow.getLocalBounds();
    title_shadow.setPosition(center_x - title_bounds.width / 2 + 3, current_y + 3);
    window.draw(title_shadow);
    
    sf::Text title("Jogo de Xadrez", font, title_size);
    title.setFillColor(sf::Color(240, 240, 240));
    title.setStyle(sf::Text::Bold);
    title.setPosition(center_x - title_bounds.width / 2, current_y);
    window.draw(title);
    
    current_y += title_h + sp_title_color;
    
    // --- SELEÇÃO DE COR ---
    sf::Text color_subtitle("Escolha sua cor", font, subtitle_size);
    color_subtitle.setFillColor(sf::Color(180, 180, 180));
    sf::FloatRect color_sub_bounds = color_subtitle.getLocalBounds();
    color_subtitle.setPosition(center_x - color_sub_bounds.width / 2, current_y);
    window.draw(color_subtitle);
    
    current_y += subtitle_h + sp_color_sub_btn;
    
    float color_btn_width = 120 * scale;
    float color_btn_spacing = 20 * scale;
    
    // Botão Brancas
    sf::RectangleShape white_btn(sf::Vector2f(color_btn_width, color_btn_h));
    white_btn.setPosition(center_x - color_btn_width - color_btn_spacing / 2, current_y);
    
    if (player_color == WHITE) {
        white_btn.setFillColor(sf::Color(200, 200, 200));
        white_btn.setOutlineColor(sf::Color(255, 255, 255));
        white_btn.setOutlineThickness(3 * scale);
    } else {
        white_btn.setFillColor(sf::Color(150, 150, 150));
        white_btn.setOutlineColor(sf::Color(100, 100, 100));
        white_btn.setOutlineThickness(1 * scale);
    }
    window.draw(white_btn);
    
    sf::Text white_text("Brancas", font, button_text_size);
    white_text.setFillColor(sf::Color::Black);
    sf::FloatRect white_bounds = white_text.getLocalBounds();
    white_text.setPosition(center_x - color_btn_width - color_btn_spacing / 2 + (color_btn_width - white_bounds.width) / 2,
                          current_y + (color_btn_h - white_bounds.height) / 2 - 5);
    window.draw(white_text);
    
    // Botão Pretas
    sf::RectangleShape black_btn(sf::Vector2f(color_btn_width, color_btn_h));
    black_btn.setPosition(center_x + color_btn_spacing / 2, current_y);
    
    if (player_color == BLACK) {
        black_btn.setFillColor(sf::Color(50, 50, 50));
        black_btn.setOutlineColor(sf::Color(200, 200, 200));
        black_btn.setOutlineThickness(3 * scale);
    } else {
        black_btn.setFillColor(sf::Color(80, 80, 80));
        black_btn.setOutlineColor(sf::Color(50, 50, 50));
        black_btn.setOutlineThickness(1 * scale);
    }
    window.draw(black_btn);
    
    sf::Text black_text("Pretas", font, button_text_size);
    black_text.setFillColor(sf::Color::White);
    sf::FloatRect black_bounds = black_text.getLocalBounds();
    black_text.setPosition(center_x + color_btn_spacing / 2 + (color_btn_width - black_bounds.width) / 2,
                          current_y + (color_btn_h - black_bounds.height) / 2 - 5);
    window.draw(black_text);

    current_y += color_btn_h + sp_color_time;

    // --- SELEÇÃO DE TEMPO ---
    // Subtítulo
    sf::Text subtitle("Selecione o tempo de jogo", font, subtitle_size);
    subtitle.setFillColor(sf::Color(180, 180, 180));
    sf::FloatRect subtitle_bounds = subtitle.getLocalBounds();
    subtitle.setPosition(center_x - subtitle_bounds.width / 2, current_y);
    window.draw(subtitle);
    
    current_y += subtitle_h + sp_time_sub_btn;
    
    // Opções de tempo
    std::vector<std::pair<std::string, int>> time_options = {
        {"1 minuto", 60},
        {"3 minutos", 180},
        {"5 minutos", 300},
        {"10 minutos", 600},
        {"15 minutos", 900},
        {"30 minutos", 1800}
    };
    
    for (size_t i = 0; i < time_options.size(); i++) {
        bool is_selected = (white_time_seconds == time_options[i].second);
        
        // Sombra do botão
        sf::RectangleShape button_shadow(sf::Vector2f(button_width, time_btn_h));
        button_shadow.setPosition(center_x - button_width / 2 + 2, current_y + 2);
        button_shadow.setFillColor(sf::Color(0, 0, 0, 80));
        window.draw(button_shadow);
        
        // Botão
        sf::RectangleShape button(sf::Vector2f(button_width, time_btn_h));
        button.setPosition(center_x - button_width / 2, current_y);
        
        if (is_selected) {
            button.setFillColor(sf::Color(70, 130, 180));  // Azul selecionado
            button.setOutlineColor(sf::Color(100, 160, 210));
        } else {
            button.setFillColor(sf::Color(50, 50, 55));
            button.setOutlineColor(sf::Color(80, 80, 85));
        }
        button.setOutlineThickness(2 * scale);
        window.draw(button);
        
        // Texto do botão
        std::ostringstream button_text;
        button_text << time_options[i].first;
        // if (is_selected) {
        //     button_text << " [X]"; 
        // }
        
        sf::Text text(button_text.str(), font, button_text_size);
        text.setFillColor(sf::Color::White);
        text.setStyle(is_selected ? sf::Text::Bold : sf::Text::Regular);
        sf::FloatRect text_bounds = text.getLocalBounds();
        text.setPosition(center_x - text_bounds.width / 2, current_y + (time_btn_h - text_bounds.height) / 2 - 5);
        window.draw(text);
        
        current_y += sp_time_btns;
    }
    
    current_y += sp_time_start;
    
    // Sombra do botão iniciar
    sf::RectangleShape start_shadow(sf::Vector2f(start_width, start_btn_h));
    start_shadow.setPosition(center_x - start_width / 2 + 3, current_y + 3);
    start_shadow.setFillColor(sf::Color(0, 0, 0, 100));
    window.draw(start_shadow);
    
    // Botão iniciar
    sf::RectangleShape start_button(sf::Vector2f(start_width, start_btn_h));
    start_button.setPosition(center_x - start_width / 2, current_y);
    start_button.setFillColor(sf::Color(50, 180, 80));
    start_button.setOutlineColor(sf::Color(70, 200, 100));
    start_button.setOutlineThickness(3 * scale);
    window.draw(start_button);
    
    // Texto do botão iniciar
    sf::Text start_text("Iniciar Partida", font, start_text_size);
    start_text.setFillColor(sf::Color::White);
    start_text.setStyle(sf::Text::Bold);
    sf::FloatRect start_bounds = start_text.getLocalBounds();
    start_text.setPosition(center_x - start_bounds.width / 2, 
                          current_y + (start_btn_h - start_bounds.height) / 2 - 5);
    window.draw(start_text);
}

void ChessGUI::handle_menu_click(int mouse_x, int mouse_y) {
    sf::Vector2u window_size = window.getSize();
    float center_x = window_size.x / 2.0f;
    float center_y = window_size.y / 2.0f;
    
    // Calcular escala (mesma do draw_menu)
    float base_height = 800.0f;
    float base_width = 600.0f;
    float scale = std::min(window_size.y / base_height, window_size.x / base_width);
    scale = std::max(0.5f, std::min(1.2f, scale));
    
    std::vector<std::pair<std::string, int>> time_options = {
        {"1 minuto", 60},
        {"3 minutos", 180},
        {"5 minutos", 300},
        {"10 minutos", 600},
        {"15 minutos", 900},
        {"30 minutos", 1800}
    };
    
    // Calcular altura total do menu para centralização (mesma lógica do draw_menu)
    float title_h = 52 * scale;
    float subtitle_h = 20 * scale;
    float color_btn_h = 40 * scale;
    float time_btn_h = 45 * scale;
    float start_btn_h = 60 * scale;
    
    // Espaçamentos
    float sp_title_color = 50 * scale;
    float sp_color_sub_btn = 20 * scale;
    float sp_color_time = 40 * scale;
    float sp_time_sub_btn = 20 * scale;
    float sp_time_btns = 55 * scale;
    float sp_time_start = 30 * scale;
    
    float total_height = title_h + sp_title_color + subtitle_h + sp_color_sub_btn + color_btn_h + 
                         sp_color_time + subtitle_h + sp_time_sub_btn + (6 * sp_time_btns) + sp_time_start + start_btn_h;
                         
    // Ajustar escala se não couber
    if (total_height > window_size.y * 0.95f) {
        scale = scale * (window_size.y * 0.95f) / total_height;
        // Recalcular alturas com nova escala
        title_h = 52 * scale;
        subtitle_h = 20 * scale;
        color_btn_h = 40 * scale;
        time_btn_h = 45 * scale;
        start_btn_h = 60 * scale;
        sp_title_color = 50 * scale;
        sp_color_sub_btn = 20 * scale;
        sp_color_time = 40 * scale;
        sp_time_sub_btn = 20 * scale;
        sp_time_btns = 55 * scale;
        sp_time_start = 30 * scale;
        
        total_height = window_size.y * 0.95f;
    }
    
    float current_y = center_y - total_height / 2.0f;
    
    // Pular título
    current_y += title_h + sp_title_color;
    
    // Pular subtítulo cor
    current_y += subtitle_h + sp_color_sub_btn;
    
    // --- SELEÇÃO DE COR ---
    float color_btn_width = 120 * scale;
    float color_btn_spacing = 20 * scale;
    
    // Verificar clique no botão Brancas
    float white_btn_x = center_x - color_btn_width - color_btn_spacing / 2;
    if (mouse_x >= white_btn_x && mouse_x <= white_btn_x + color_btn_width &&
        mouse_y >= current_y && mouse_y <= current_y + color_btn_h) {
        player_color = WHITE;
        return;
    }
    
    // Verificar clique no botão Pretas
    float black_btn_x = center_x + color_btn_spacing / 2;
    if (mouse_x >= black_btn_x && mouse_x <= black_btn_x + color_btn_width &&
        mouse_y >= current_y && mouse_y <= current_y + color_btn_h) {
        player_color = BLACK;
        return;
    }
    
    current_y += color_btn_h + sp_color_time;
    
    // --- SELEÇÃO DE TEMPO ---
    // Pular subtítulo tempo
    current_y += subtitle_h + sp_time_sub_btn;
    
    float button_width = 250 * scale;
    
    // Verificar cliques nas opções de tempo
    for (size_t i = 0; i < time_options.size(); i++) {
        float button_x = center_x - button_width / 2;
        
        if (mouse_x >= button_x && mouse_x <= button_x + button_width &&
            mouse_y >= current_y && mouse_y <= current_y + time_btn_h) {
            initial_time_seconds = time_options[i].second;
            white_time_seconds = initial_time_seconds;
            black_time_seconds = initial_time_seconds;
            return;
        }
        
        current_y += sp_time_btns;
    }
    
    current_y += sp_time_start;
    
    // Verificar clique no botão iniciar
    float start_width = 240 * scale;
    float start_x = center_x - start_width / 2;
    
    if (mouse_x >= start_x && mouse_x <= start_x + start_width &&
        mouse_y >= current_y && mouse_y <= current_y + start_btn_h) {
        start_game();
    }
}

void ChessGUI::start_game() {
    current_state = PLAYING;
    game_started = true;
    game_ended = false;
    
    // Resetar tempos
    white_time_seconds = initial_time_seconds;
    black_time_seconds = initial_time_seconds;
    
    move_start_time = std::chrono::steady_clock::now();
    captured_white.clear();
    captured_black.clear();
    board = ChessBoard();
    selected_square = NO_SQUARE;
    is_square_selected = false;
    has_last_move = false;
    update_status_text();
}

// Funções do relógio
std::string ChessGUI::format_time(int seconds) const {
    int minutes = seconds / 60;
    int secs = seconds % 60;
    std::ostringstream oss;
    oss << minutes << ":" << std::setfill('0') << std::setw(2) << secs;
    return oss.str();
}

void ChessGUI::update_clocks() {
    if (!game_started || current_state != PLAYING) return;
    
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - move_start_time).count();
    
    Color side = board.get_side_to_move();
    // O lado que acabou de jogar perde tempo
    if (side == BLACK) {
        // Brancas acabaram de jogar
        white_time_seconds -= elapsed;
        if (white_time_seconds < 0) white_time_seconds = 0;
    } else {
        // Pretas acabaram de jogar
        black_time_seconds -= elapsed;
        if (black_time_seconds < 0) black_time_seconds = 0;
    }
    
    move_start_time = now;
}

bool ChessGUI::is_white_at_bottom() const {
    // Se o jogador escolheu brancas, brancas ficam embaixo
    // Se escolheu pretas, pretas ficam embaixo (tabuleiro invertido)
    return player_color == WHITE;
}

void ChessGUI::draw_clocks_and_captured() {
    if (current_state != PLAYING) return;
    
    int board_size = get_board_size();
    int panel_width = get_panel_width();
    sf::Vector2u window_size = window.getSize();
    
    // Calcular escala baseada no tamanho do painel, não do tabuleiro
    // Isso garante que os textos fiquem proporcionais ao painel
    float base_panel_width = 200.0f;
    float scale = panel_width / base_panel_width;
    scale = std::max(0.7f, std::min(1.5f, scale));  // Limitar para evitar tamanhos extremos
    
    bool white_bottom = is_white_at_bottom();
    
    // Atualizar relógios continuamente (só se o jogo não terminou)
    if (game_started && !game_ended) {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - move_start_time).count();
        
        Color side = board.get_side_to_move();
        int white_time = white_time_seconds;
        int black_time = black_time_seconds;
        
        if (side == WHITE) {
            white_time -= elapsed;
        } else {
            black_time -= elapsed;
        }
        
        // Verificar se o tempo esgotou
        if (white_time <= 0 && !game_ended) {
            game_ended = true;
            winner = BLACK;
            game_started = false;
            std::cout << "Tempo esgotado! Brancas perderam por tempo.\n";
            std::cout << "Vencedor: Pretas\n";
        }
        if (black_time <= 0 && !game_ended) {
            game_ended = true;
            winner = WHITE;
            game_started = false;
            std::cout << "Tempo esgotado! Pretas perderam por tempo.\n";
            std::cout << "Vencedor: Brancas\n";
        }
        
        if (white_time < 0) white_time = 0;
        if (black_time < 0) black_time = 0;
        
        // Determinar qual relógio está ativo
        Color active_side = board.get_side_to_move();
        bool white_active = (active_side == WHITE);
        bool black_active = (active_side == BLACK);
        
        float clock_width = panel_width - 20;
        float clock_height = 65; // Aumentado para evitar overflow
        float button_height = 35;
        float button_spacing = 10;
        
        // Calcular posições baseado na orientação
        float top_y, bottom_y;
        Color top_color, bottom_color;
        int top_time, bottom_time;
        bool top_active, bottom_active;
        std::vector<PieceType>* top_captured, *bottom_captured;
        std::string top_label, bottom_label;
        
        if (white_bottom) {
            // Brancas embaixo, pretas em cima
            top_y = 20;
            bottom_y = window_size.y - 200;  // Deixar espaço para botões
            
            top_color = BLACK;
            bottom_color = WHITE;
            top_time = black_time;
            bottom_time = white_time;
            top_active = black_active;
            bottom_active = white_active;
            top_captured = &captured_white;  // Peças brancas capturadas por pretas
            bottom_captured = &captured_black;  // Peças pretas capturadas por brancas
            top_label = "Pretas";
            bottom_label = "Brancas";
        } else {
            // Pretas embaixo, brancas em cima
            top_y = 20;
            bottom_y = window_size.y - 200;
            
            top_color = WHITE;
            bottom_color = BLACK;
            top_time = white_time;
            bottom_time = black_time;
            top_active = white_active;
            bottom_active = black_active;
            top_captured = &captured_black;  // Peças pretas capturadas por brancas
            bottom_captured = &captured_white;  // Peças brancas capturadas por pretas
            top_label = "Brancas";
            bottom_label = "Pretas";
        }
        
        // Desenhar relógio do topo
        float clock_y = top_y;
        
        // Fundo do relógio do topo
        sf::RectangleShape top_clock_bg(sf::Vector2f(clock_width, clock_height));
        top_clock_bg.setPosition(board_size + 10, clock_y);
        top_clock_bg.setFillColor(top_active ? sf::Color(60, 100, 140) : sf::Color(45, 45, 50));
        top_clock_bg.setOutlineColor(top_active ? sf::Color(80, 120, 160) : sf::Color(60, 60, 65));
        top_clock_bg.setOutlineThickness(2);
        window.draw(top_clock_bg);
        
        // Label do topo
        sf::Text top_label_text(top_label, font, static_cast<unsigned int>(14 * scale));
        top_label_text.setFillColor(sf::Color(200, 200, 200));
        sf::FloatRect top_label_bounds = top_label_text.getLocalBounds();
        top_label_text.setPosition(board_size + 10 + (clock_width - top_label_bounds.width) / 2, clock_y + 5);
        window.draw(top_label_text);
        
        // Tempo do topo
        sf::Text top_clock_text(format_time(top_time), font, static_cast<unsigned int>(24 * scale));
        top_clock_text.setFillColor(top_time < 60 ? sf::Color(255, 100, 100) : sf::Color::White);
        top_clock_text.setStyle(sf::Text::Bold);
        sf::FloatRect top_clock_bounds = top_clock_text.getLocalBounds();
        top_clock_text.setPosition(board_size + 10 + (clock_width - top_clock_bounds.width) / 2, clock_y + 22);
        window.draw(top_clock_text);
        
        // Peças capturadas do topo
        float captured_y = clock_y + clock_height + 10;
        float piece_size = 24 * scale;
        float piece_spacing = piece_size + 4;
        float section_width = panel_width - 20;
        
        if (!top_captured->empty()) {
            // Label
            std::ostringstream captured_label_text;
            captured_label_text << "Capturadas por " << top_label;
            sf::Text captured_label(captured_label_text.str(), font, static_cast<unsigned int>(12 * scale));
            captured_label.setFillColor(sf::Color(180, 180, 180));
            captured_label.setPosition(board_size + 10, captured_y);
            window.draw(captured_label);
            
            // Fundo da seção
            int rows = (top_captured->size() * piece_spacing > section_width) ? 
                       (static_cast<int>(top_captured->size()) * static_cast<int>(piece_spacing) / static_cast<int>(section_width) + 1) : 1;
            float section_height = rows * piece_spacing + 10;
            
            sf::RectangleShape section_bg(sf::Vector2f(section_width, section_height + 20));
            section_bg.setPosition(board_size + 10, captured_y + 18);
            section_bg.setFillColor(sf::Color(40, 40, 45));
            section_bg.setOutlineColor(sf::Color(60, 60, 65));
            section_bg.setOutlineThickness(1);
            window.draw(section_bg);
            
            // Desenhar peças
            float x = board_size + 15;
            float y = captured_y + 25;
            for (PieceType pt : *top_captured) {
                draw_piece(window, pt, (top_color == WHITE) ? BLACK : WHITE, static_cast<int>(x), static_cast<int>(y), piece_size);
                x += piece_spacing;
                if (x + piece_size > board_size + panel_width - 15) {
                    x = board_size + 15;
                    y += piece_spacing;
                }
            }
        }
        
        // Desenhar relógio de baixo (calculado de baixo para cima)
        float bottom_clock_y = bottom_y - clock_height;
        if (!bottom_captured->empty()) {
            // Ajustar para peças capturadas
            int rows = (bottom_captured->size() * piece_spacing > section_width) ? 
                       (static_cast<int>(bottom_captured->size()) * static_cast<int>(piece_spacing) / static_cast<int>(section_width) + 1) : 1;
            float section_height = rows * piece_spacing + 30;
            bottom_clock_y -= section_height;
        }
        bottom_clock_y -= button_height + button_spacing;  // Espaço para botão de desistência
        
        // Fundo do relógio de baixo
        sf::RectangleShape bottom_clock_bg(sf::Vector2f(clock_width, clock_height));
        bottom_clock_bg.setPosition(board_size + 10, bottom_clock_y);
        bottom_clock_bg.setFillColor(bottom_active ? sf::Color(60, 100, 140) : sf::Color(45, 45, 50));
        bottom_clock_bg.setOutlineColor(bottom_active ? sf::Color(80, 120, 160) : sf::Color(60, 60, 65));
        bottom_clock_bg.setOutlineThickness(2);
        window.draw(bottom_clock_bg);
        
        // Label de baixo
        sf::Text bottom_label_text(bottom_label, font, static_cast<unsigned int>(14 * scale));
        bottom_label_text.setFillColor(sf::Color(200, 200, 200));
        sf::FloatRect bottom_label_bounds = bottom_label_text.getLocalBounds();
        bottom_label_text.setPosition(board_size + 10 + (clock_width - bottom_label_bounds.width) / 2, bottom_clock_y + 5);
        window.draw(bottom_label_text);
        
        // Tempo de baixo
        sf::Text bottom_clock_text(format_time(bottom_time), font, static_cast<unsigned int>(24 * scale));
        bottom_clock_text.setFillColor(bottom_time < 60 ? sf::Color(255, 100, 100) : sf::Color::White);
        bottom_clock_text.setStyle(sf::Text::Bold);
        sf::FloatRect bottom_clock_bounds = bottom_clock_text.getLocalBounds();
        bottom_clock_text.setPosition(board_size + 10 + (clock_width - bottom_clock_bounds.width) / 2, bottom_clock_y + 22);
        window.draw(bottom_clock_text);
        
        // Peças capturadas de baixo
        float bottom_captured_y = bottom_clock_y - 10;
        if (!bottom_captured->empty()) {
            bottom_captured_y -= 20;
            
            // Label
            std::ostringstream captured_label_text;
            captured_label_text << "Capturadas por " << bottom_label;
            sf::Text captured_label(captured_label_text.str(), font, static_cast<unsigned int>(12 * scale));
            captured_label.setFillColor(sf::Color(180, 180, 180));
            captured_label.setPosition(board_size + 10, bottom_captured_y - 20);
            window.draw(captured_label);
            
            // Fundo da seção
            int rows = (bottom_captured->size() * piece_spacing > section_width) ? 
                       (static_cast<int>(bottom_captured->size()) * static_cast<int>(piece_spacing) / static_cast<int>(section_width) + 1) : 1;
            float section_height = rows * piece_spacing + 10;
            
            sf::RectangleShape section_bg(sf::Vector2f(section_width, section_height + 20));
            section_bg.setPosition(board_size + 10, bottom_captured_y - section_height - 18);
            section_bg.setFillColor(sf::Color(40, 40, 45));
            section_bg.setOutlineColor(sf::Color(60, 60, 65));
            section_bg.setOutlineThickness(1);
            window.draw(section_bg);
            
            // Desenhar peças (de baixo para cima)
            float x = board_size + 15;
            float y = bottom_captured_y - section_height - 5;
            for (PieceType pt : *bottom_captured) {
                draw_piece(window, pt, (bottom_color == WHITE) ? BLACK : WHITE, static_cast<int>(x), static_cast<int>(y), piece_size);
                x += piece_spacing;
                if (x + piece_size > board_size + panel_width - 15) {
                    x = board_size + 15;
                    y -= piece_spacing;
                }
            }
        }
        
        // Botões de desistência
        float resign_button_y = window_size.y - button_height - 10;
        
        // Botão de desistência do topo
        sf::RectangleShape top_resign_button(sf::Vector2f(clock_width, button_height));
        top_resign_button.setPosition(board_size + 10, top_y + clock_height + 10);
        top_resign_button.setFillColor(sf::Color(180, 50, 50));
        top_resign_button.setOutlineColor(sf::Color(200, 70, 70));
        top_resign_button.setOutlineThickness(2);
        window.draw(top_resign_button);
        
        std::ostringstream top_resign_text;
        top_resign_text << "Desistir (" << top_label << ")";
        sf::Text top_resign_label(top_resign_text.str(), font, static_cast<unsigned int>(12 * scale));
        top_resign_label.setFillColor(sf::Color::White);
        sf::FloatRect top_resign_bounds = top_resign_label.getLocalBounds();
        top_resign_label.setPosition(board_size + 10 + (clock_width - top_resign_bounds.width) / 2, 
                                    top_y + clock_height + 10 + (button_height - top_resign_bounds.height) / 2 - 5);
        window.draw(top_resign_label);
        
        // Botão de desistência de baixo
        sf::RectangleShape bottom_resign_button(sf::Vector2f(clock_width, button_height));
        bottom_resign_button.setPosition(board_size + 10, resign_button_y);
        bottom_resign_button.setFillColor(sf::Color(180, 50, 50));
        bottom_resign_button.setOutlineColor(sf::Color(200, 70, 70));
        bottom_resign_button.setOutlineThickness(2);
        window.draw(bottom_resign_button);
        
        std::ostringstream bottom_resign_text;
        bottom_resign_text << "Desistir (" << bottom_label << ")";
        sf::Text bottom_resign_label(bottom_resign_text.str(), font, static_cast<unsigned int>(12 * scale));
        bottom_resign_label.setFillColor(sf::Color::White);
        sf::FloatRect bottom_resign_bounds = bottom_resign_label.getLocalBounds();
        bottom_resign_label.setPosition(board_size + 10 + (clock_width - bottom_resign_bounds.width) / 2, 
                                       resign_button_y + (button_height - bottom_resign_bounds.height) / 2 - 5);
        window.draw(bottom_resign_label);
    }
}

// Sobrecarga de draw_piece para tamanho customizado
void ChessGUI::draw_piece(sf::RenderTarget& target, PieceType pt, Color c, int x, int y, float custom_size) {
    if (pt == NONE) return;
    
    // Tentar usar textura da imagem
    auto it = piece_textures.find({pt, c});
    if (it != piece_textures.end()) {
        const sf::Texture& texture = it->second;
        sf::Vector2u texture_size = texture.getSize();
        
        if (texture_size.x > 0 && texture_size.y > 0) {
            sf::Sprite piece_sprite(texture);
            float scale = custom_size / static_cast<float>(std::max(texture_size.x, texture_size.y));
            piece_sprite.setScale(scale, scale);
            
            sf::FloatRect sprite_bounds = piece_sprite.getLocalBounds();
            float scaled_width = sprite_bounds.width * scale;
            float scaled_height = sprite_bounds.height * scale;
            
            piece_sprite.setPosition(
                x + (custom_size - scaled_width) / 2 - sprite_bounds.left * scale,
                y + (custom_size - scaled_height) / 2 - sprite_bounds.top * scale
            );
            
            target.draw(piece_sprite);
            return;
        }
    }
    
    // Fallback: usar texto Unicode
    sf::Text piece_text;
    piece_text.setFont(font);
    piece_text.setCharacterSize(static_cast<unsigned int>(custom_size * 0.75f));
    piece_text.setFillColor(c == WHITE ? sf::Color::White : sf::Color::Black);
    piece_text.setStyle(sf::Text::Bold);
    piece_text.setString(get_piece_symbol(pt, c));
    
    sf::FloatRect text_bounds = piece_text.getLocalBounds();
    piece_text.setPosition(
        x + (custom_size - text_bounds.width) / 2 - text_bounds.left,
        y + (custom_size - text_bounds.height) / 2 - text_bounds.top
    );
    
    target.draw(piece_text);
}

bool ChessGUI::handle_resign_button_click(int mouse_x, int mouse_y) {
    if (current_state != PLAYING || !game_started) return false;
    
    int board_size = get_board_size();
    int panel_width = get_panel_width();
    sf::Vector2u window_size = window.getSize();
    
    bool white_bottom = is_white_at_bottom();
    float clock_width = panel_width - 20;
    float clock_height = 50;
    float button_height = 35;
    
    // Calcular posições dos botões
    float top_y = 20;
    float top_button_y = top_y + clock_height + 10;
    float bottom_button_y = window_size.y - button_height - 10;
    
    // Verificar clique no botão do topo
    if (mouse_x >= board_size + 10 && mouse_x <= board_size + 10 + clock_width &&
        mouse_y >= top_button_y && mouse_y <= top_button_y + button_height) {
        Color top_color = white_bottom ? BLACK : WHITE;
        resign(top_color);
        return true;
    }
    
    // Verificar clique no botão de baixo
    if (mouse_x >= board_size + 10 && mouse_x <= board_size + 10 + clock_width &&
        mouse_y >= bottom_button_y && mouse_y <= bottom_button_y + button_height) {
        Color bottom_color = white_bottom ? WHITE : BLACK;
        resign(bottom_color);
        return true;
    }
    
    return false;
}

void ChessGUI::resign(Color player) {
    // Parar os relógios
    update_clocks();
    game_started = false;
    game_ended = true;
    winner = (player == WHITE ? BLACK : WHITE);
    
    // Mostrar mensagem de desistência (por enquanto apenas no console)
    std::cout << "Jogador " << (player == WHITE ? "Brancas" : "Pretas") << " desistiu!\n";
    std::cout << "Vencedor: " << (player == WHITE ? "Pretas" : "Brancas") << "\n";
}

void ChessGUI::run() {
    while (window.isOpen()) {
        handle_events();
        render();
    }
}

void ChessGUI::draw_game_over() {
    sf::Vector2u window_size = window.getSize();
    float center_x = window_size.x / 2.0f;
    float center_y = window_size.y / 2.0f;
    
    // Fundo semitransparente
    sf::RectangleShape overlay(sf::Vector2f(window_size.x, window_size.y));
    overlay.setFillColor(sf::Color(0, 0, 0, 200));
    window.draw(overlay);
    
    // Caixa de diálogo
    float box_width = 400;
    float box_height = 250;
    sf::RectangleShape box(sf::Vector2f(box_width, box_height));
    box.setPosition(center_x - box_width / 2, center_y - box_height / 2);
    box.setFillColor(sf::Color(50, 50, 55));
    box.setOutlineColor(sf::Color(100, 100, 100));
    box.setOutlineThickness(2);
    window.draw(box);
    
    // Texto de resultado
    std::string result_text;
    if (winner == WHITE) result_text = "Brancas Vencem!";
    else if (winner == BLACK) result_text = "Pretas Vencem!";
    else result_text = "Empate!"; // Caso implemente empate
    
    sf::Text result(result_text, font, 32);
    result.setFillColor(sf::Color::White);
    result.setStyle(sf::Text::Bold);
    sf::FloatRect result_bounds = result.getLocalBounds();
    result.setPosition(center_x - result_bounds.width / 2, center_y - 80);
    window.draw(result);
    
    // Botão Reiniciar
    float btn_width = 180;
    float btn_height = 50;
    float btn_spacing = 20;
    
    sf::RectangleShape restart_btn(sf::Vector2f(btn_width, btn_height));
    restart_btn.setPosition(center_x - btn_width - btn_spacing / 2, center_y + 20);
    restart_btn.setFillColor(sf::Color(70, 130, 180));
    restart_btn.setOutlineColor(sf::Color(100, 160, 210));
    restart_btn.setOutlineThickness(2);
    window.draw(restart_btn);
    
    sf::Text restart_text("Reiniciar", font, 20);
    restart_text.setFillColor(sf::Color::White);
    sf::FloatRect restart_bounds = restart_text.getLocalBounds();
    restart_text.setPosition(center_x - btn_width - btn_spacing / 2 + (btn_width - restart_bounds.width) / 2,
                            center_y + 20 + (btn_height - restart_bounds.height) / 2 - 5);
    window.draw(restart_text);
    
    // Botão Menu
    sf::RectangleShape menu_btn(sf::Vector2f(btn_width, btn_height));
    menu_btn.setPosition(center_x + btn_spacing / 2, center_y + 20);
    menu_btn.setFillColor(sf::Color(80, 80, 80));
    menu_btn.setOutlineColor(sf::Color(120, 120, 120));
    menu_btn.setOutlineThickness(2);
    window.draw(menu_btn);
    
    sf::Text menu_text("Menu Principal", font, 20);
    menu_text.setFillColor(sf::Color::White);
    sf::FloatRect menu_bounds = menu_text.getLocalBounds();
    menu_text.setPosition(center_x + btn_spacing / 2 + (btn_width - menu_bounds.width) / 2,
                         center_y + 20 + (btn_height - menu_bounds.height) / 2 - 5);
    window.draw(menu_text);
}

void ChessGUI::handle_game_over_click(int mouse_x, int mouse_y) {
    sf::Vector2u window_size = window.getSize();
    float center_x = window_size.x / 2.0f;
    float center_y = window_size.y / 2.0f;
    
    float btn_width = 180;
    float btn_height = 50;
    float btn_spacing = 20;
    
    // Botão Reiniciar
    float restart_x = center_x - btn_width - btn_spacing / 2;
    float restart_y = center_y + 20;
    
    if (mouse_x >= restart_x && mouse_x <= restart_x + btn_width &&
        mouse_y >= restart_y && mouse_y <= restart_y + btn_height) {
        start_game(); // Reinicia com as mesmas configurações
        return;
    }
    
    // Botão Menu
    float menu_x = center_x + btn_spacing / 2;
    float menu_y = center_y + 20;
    
    if (mouse_x >= menu_x && mouse_x <= menu_x + btn_width &&
        mouse_y >= menu_y && mouse_y <= menu_y + btn_height) {
        current_state = MENU;
        game_started = false;
        game_ended = false;
        return;
    }
}
