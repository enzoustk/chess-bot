#include "chess_gui.h"
#include <iostream>
#include <sstream>
#include <algorithm>

ChessGUI::ChessGUI() 
    : window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Jogo de Xadrez", sf::Style::Close),
      selected_square(NO_SQUARE),
      is_square_selected(false) {
    
    window.setFramerateLimit(60);
    
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
    status_text.setCharacterSize(16);
    status_text.setFillColor(sf::Color::White);
    status_text.setPosition(BOARD_SIZE + 10, 10);
    
    move_text.setFont(font);
    move_text.setCharacterSize(14);
    move_text.setFillColor(sf::Color::White);
    move_text.setPosition(BOARD_SIZE + 10, 200);
    
    // Carregar texturas das peças
    if (!load_piece_textures()) {
        std::cerr << "Aviso: Não foi possível carregar todas as texturas das peças.\n";
        std::cerr << "Usando símbolos Unicode como fallback.\n";
    } else {
        std::cout << "Texturas das peças carregadas com sucesso (" 
                  << piece_textures.size() << " texturas).\n";
    }
    
    update_status_text();
}

ChessGUI::~ChessGUI() {
    if (window.isOpen()) {
        window.close();
    }
}

Square ChessGUI::get_square_from_mouse(int mouse_x, int mouse_y) const {
    if (mouse_x < 0 || mouse_x >= BOARD_SIZE || mouse_y < 0 || mouse_y >= BOARD_SIZE) {
        return NO_SQUARE;
    }
    
    int file = mouse_x / SQUARE_SIZE;
    int rank = 7 - (mouse_y / SQUARE_SIZE); // Inverter porque y=0 está no topo
    
    return ChessBoard::make_square(file, rank);
}

void ChessGUI::draw_board() {
    for (int rank = 0; rank < 8; rank++) {
        for (int file = 0; file < 8; file++) {
            sf::RectangleShape square(sf::Vector2f(SQUARE_SIZE, SQUARE_SIZE));
            square.setPosition(file * SQUARE_SIZE, (7 - rank) * SQUARE_SIZE);
            
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
    
    // Tentar usar textura da imagem
    auto it = piece_textures.find({pt, c});
    if (it != piece_textures.end()) {
        const sf::Texture& texture = it->second;
        sf::Vector2u texture_size = texture.getSize();
        
        // Verificar se a textura é válida
        if (texture_size.x > 0 && texture_size.y > 0) {
            sf::Sprite piece_sprite(texture);
            
            // Escalar a imagem para caber no quadrado (deixar um pequeno padding)
            float scale = (SQUARE_SIZE - 10) / static_cast<float>(std::max(texture_size.x, texture_size.y));
            piece_sprite.setScale(scale, scale);
            
            // Centralizar a peça no quadrado
            sf::FloatRect sprite_bounds = piece_sprite.getLocalBounds();
            float scaled_width = sprite_bounds.width * scale;
            float scaled_height = sprite_bounds.height * scale;
            
            piece_sprite.setPosition(
                x + (SQUARE_SIZE - scaled_width) / 2 - sprite_bounds.left * scale,
                y + (SQUARE_SIZE - scaled_height) / 2 - sprite_bounds.top * scale
            );
            
            target.draw(piece_sprite);
            return;
        }
    }
    
    // Fallback: usar texto Unicode se a textura não foi carregada
    sf::Text piece_text;
    piece_text.setFont(font);
    piece_text.setCharacterSize(SQUARE_SIZE - 20);
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
        x + (SQUARE_SIZE - text_bounds.width) / 2 - text_bounds.left,
        y + (SQUARE_SIZE - text_bounds.height) / 2 - text_bounds.top
    );
    
    target.draw(piece_text);
}

void ChessGUI::draw_pieces() {
    for (int rank = 0; rank < 8; rank++) {
        for (int file = 0; file < 8; file++) {
            Square sq = ChessBoard::make_square(file, rank);
            PieceType pt = board.get_piece(sq);
            
            if (pt != NONE) {
                Color c = board.get_piece_color(sq);
                int x = file * SQUARE_SIZE;
                int y = (7 - rank) * SQUARE_SIZE;
                draw_piece(window, pt, c, x, y);
            }
        }
    }
}

void ChessGUI::draw_legal_moves() {
    if (!is_square_selected) return;
    
    for (const Move& move : legal_moves_for_selected) {
        int file = ChessBoard::get_file(move.to);
        int rank = ChessBoard::get_rank(move.to);
        
        sf::CircleShape indicator(8);
        indicator.setFillColor(legal_move_color);
        indicator.setPosition(
            file * SQUARE_SIZE + SQUARE_SIZE / 2 - 8,
            (7 - rank) * SQUARE_SIZE + SQUARE_SIZE / 2 - 8
        );
        
        window.draw(indicator);
    }
}

void ChessGUI::draw_highlights() {
    if (is_square_selected && selected_square != NO_SQUARE) {
        int file = ChessBoard::get_file(selected_square);
        int rank = ChessBoard::get_rank(selected_square);
        
        sf::RectangleShape highlight(sf::Vector2f(SQUARE_SIZE, SQUARE_SIZE));
        highlight.setFillColor(selected_color);
        highlight.setPosition(file * SQUARE_SIZE, (7 - rank) * SQUARE_SIZE);
        
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
                
                if (board.make_move(move)) {
                    move_made = true;
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
        } else if (event.type == sf::Event::MouseButtonPressed) {
            if (event.mouseButton.button == sf::Mouse::Left) {
                handle_mouse_click(event.mouseButton.x, event.mouseButton.y);
            }
        } else if (event.type == sf::Event::KeyPressed) {
            if (event.key.code == sf::Keyboard::Escape) {
                // Desfazer movimento
                if (board.get_fullmove_number() > 1 || board.get_side_to_move() == BLACK) {
                    board.unmake_move();
                    selected_square = NO_SQUARE;
                    is_square_selected = false;
                    legal_moves_for_selected.clear();
                    update_status_text();
                }
            } else if (event.key.code == sf::Keyboard::R) {
                // Reiniciar
                board = ChessBoard();
                selected_square = NO_SQUARE;
                is_square_selected = false;
                legal_moves_for_selected.clear();
                update_status_text();
            }
        }
    }
}

void ChessGUI::render() {
    window.clear(sf::Color(50, 50, 50));
    
    draw_board();
    draw_highlights();
    draw_legal_moves();
    draw_pieces();
    
    // Painel lateral
    sf::RectangleShape panel(sf::Vector2f(200, WINDOW_HEIGHT));
    panel.setFillColor(sf::Color(40, 40, 40));
    panel.setPosition(BOARD_SIZE, 0);
    window.draw(panel);
    
    window.draw(status_text);
    window.draw(move_text);
    
    window.display();
}

void ChessGUI::run() {
    while (window.isOpen()) {
        handle_events();
        render();
    }
}

