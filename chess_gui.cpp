#include "chess_gui.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <iomanip>
#include <cmath>

// --- CONSTRUTOR ---
ChessGUI::ChessGUI() 
    : window(sf::VideoMode(1024, 768), 
             "Jogo de Xadrez (Engine V3)", sf::Style::Close | sf::Style::Resize),
      // Threads
      is_thinking(false),
      move_ready(false),
      // Estado
      current_state(MENU),
      player_color(WHITE),
      game_started(false),
      game_ended(false),
      winner(WHITE),
      selecting_time(false),
      // Tempo
      white_time_seconds(600), 
      black_time_seconds(600),
      initial_time_seconds(600),
      // Seleção
      selected_square(NO_SQUARE),
      is_square_selected(false),
      has_last_move(false),
      // Promoção
      awaiting_promotion(false),
      promotion_square(NO_SQUARE),
      // Avaliação
      current_eval(0) {
    
    window.setFramerateLimit(60);
    
    #ifdef _WIN32
        if (!font.loadFromFile("C:/Windows/Fonts/arial.ttf")) font.loadFromFile("C:/Windows/Fonts/calibri.ttf");
    #else
        if (!font.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf")) font.loadFromFile("/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf");
    #endif
    
    status_text.setFont(font); status_text.setFillColor(sf::Color::White);
    move_text.setFont(font); move_text.setFillColor(sf::Color::White);
    white_clock_text.setFont(font);
    black_clock_text.setFont(font);
    
    load_piece_textures();
}

ChessGUI::~ChessGUI() { 
    if (engine_thread.joinable()) engine_thread.join();
    if (window.isOpen()) window.close(); 
}

// --- DIMENSÕES ---
// --- DIMENSÕES ---
int ChessGUI::get_board_size() const {
    sf::Vector2u size = window.getSize();
    // Calculate max possible square size that fits
    // We need room for panel
    // Min panel width is say 250
    
    int available_width = size.x - 250; 
    int available_height = size.y;
    
    int max_sq_by_width = available_width / 8;
    int max_sq_by_height = available_height / 8;
    
    int sq_size = std::min(max_sq_by_width, max_sq_by_height);
    return sq_size * 8;
}

int ChessGUI::get_square_size() const { return get_board_size() / 8; }

int ChessGUI::get_panel_width() const {
    sf::Vector2u size = window.getSize();
    return size.x - get_board_size();
}

float ChessGUI::get_scale_factor() const { return (float)get_board_size() / 640.0f; }

unsigned int ChessGUI::get_font_size(int base_size) const {
    return (unsigned int)(base_size * get_scale_factor());
}

// --- THREADING ---
void ChessGUI::start_engine_thinking() {
    if (is_thinking) return;
    is_thinking = true;
    move_ready = false;
    if (engine_thread.joinable()) engine_thread.join();
    engine_thread = std::thread(&ChessGUI::engine_worker, this, board);
}

void ChessGUI::engine_worker(ChessBoard board_copy) {
    Move best = engine.get_best_move(board_copy);
    calculated_move = best;
    current_eval = engine.get_last_eval(); // Atualiza avaliação da GUI
    is_thinking = false;
    move_ready = true;
}

void ChessGUI::apply_engine_move() {
    Move m = calculated_move;
    if (m.from != NO_SQUARE) {
        PieceType cap = board.get_piece(m.to);
        if (cap != NONE) {
            if (board.get_piece_color(m.to) == WHITE) captured_white.push_back(cap);
            else captured_black.push_back(cap);
        } else if (m.is_en_passant) {
             if (board.get_side_to_move() == WHITE) captured_black.push_back(PAWN);
             else captured_white.push_back(PAWN);
        }
        
        // Animation Setup for Engine
        int from_f = ChessBoard::get_file(m.from);
        int from_r = ChessBoard::get_rank(m.from);
        int to_f = ChessBoard::get_file(m.to);
        int to_r = ChessBoard::get_rank(m.to);
        bool wb = is_white_at_bottom();
        int sz = get_square_size();
        
        AnimatingPiece anim;
        anim.piece = board.get_piece(m.from);
        anim.color = board.get_piece_color(m.from);
        anim.start_pos = sf::Vector2f((float)(wb ? from_f : 7-from_f) * sz, (float)(wb ? 7-from_r : from_r) * sz);
        anim.end_pos = sf::Vector2f((float)(wb ? to_f : 7-to_f) * sz, (float)(wb ? 7-to_r : to_r) * sz);
        anim.start_time = std::chrono::steady_clock::now();
        anim.duration_seconds = 0.2f;
        animations.push_back(anim);

        board.make_move(m);
        last_move = m;
        has_last_move = true;
        move_start_time = std::chrono::steady_clock::now();
        
        if (board.is_checkmate(board.get_side_to_move())) {
            game_ended = true; winner = (board.get_side_to_move() == WHITE ? BLACK : WHITE);
        } else if (board.is_stalemate(board.get_side_to_move())) {
            game_ended = true;
        }
        update_status_text();
        std::cout << "Engine jogou: " << m.to_string() << "\n";
    }
}

// --- LOOP PRINCIPAL ---
void ChessGUI::run() {
    while (window.isOpen()) {
        handle_events();
        if (current_state == PLAYING && !game_ended) {
            update_clocks();
            if (move_ready) { apply_engine_move(); move_ready = false; }
            if (!is_thinking && !move_ready && !awaiting_promotion && board.get_side_to_move() != player_color) {
                start_engine_thinking();
            }
        }
        render();
    }
}

void ChessGUI::render() {
    window.clear(theme.bg_color); // Fundo da paleta nova
    if (current_state == MENU) draw_menu();
    else {
        draw_board();
        draw_last_move();
        draw_highlights();
        draw_pieces();
        draw_legal_moves();
        draw_check_indicator();
        
        draw_sidebar();
        
        if (awaiting_promotion) draw_promotion_menu();
        if (game_ended) draw_game_over();
    }
    window.display();
}

// --- UTILITÁRIOS DE INPUT ---

Square ChessGUI::get_square_from_mouse(int x, int y) const {
    int sz = get_square_size();
    if (x >= get_board_size()) return NO_SQUARE;
    int c = x / sz;
    int r = y / sz;
    bool wb = is_white_at_bottom();
    int file = wb ? c : 7 - c;
    int rank = wb ? 7 - r : r;
    if (file < 0 || file > 7 || rank < 0 || rank > 7) return NO_SQUARE;
    return ChessBoard::make_square(file, rank);
}

void ChessGUI::handle_events() {
    sf::Event event;
    while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) window.close();
        else if (event.type == sf::Event::Resized) {
            sf::FloatRect v(0, 0, event.size.width, event.size.height);
            window.setView(sf::View(v));
        }
        else if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
            sf::Vector2i mouse_pos(event.mouseButton.x, event.mouseButton.y);
            sf::Vector2f world_pos = window.mapPixelToCoords(mouse_pos);
            if (current_state == MENU) handle_menu_click((int)world_pos.x, (int)world_pos.y);
            else if (!is_thinking) handle_mouse_click(event.mouseButton.x, event.mouseButton.y);
        }
        else if (event.type == sf::Event::KeyPressed) {
            if (event.key.code == sf::Keyboard::R && !is_thinking) reset_game();
            if (event.key.code == sf::Keyboard::Escape) {
                current_state = MENU;
                selecting_time = false;
                player_color = WHITE;
            }
        }
    }
}

void ChessGUI::handle_mouse_click(int x, int y) {
    if (game_ended) { handle_game_over_click(x, y); return; }
    if (awaiting_promotion) { handle_promotion_click(x, y); return; }
    if (board.get_side_to_move() != player_color) return;

    Square sq = get_square_from_mouse(x, y);
    if (sq == NO_SQUARE) return;

    if (is_square_selected) {
        bool moved = false;
        for (const auto& m : legal_moves_for_selected) {
            if (m.to == sq) {
                if ((board.get_piece(m.from) == PAWN) && 
                   ((board.get_piece_color(m.from) == WHITE && ChessBoard::get_rank(sq) == 7) ||
                    (board.get_piece_color(m.from) == BLACK && ChessBoard::get_rank(sq) == 0))) {
                    pending_promotion_move = m; pending_promotion_move.promotion = NONE;
                    promotion_square = sq; awaiting_promotion = true;
                    is_square_selected = false; return;
                }
                
                PieceType cap = board.get_piece(m.to);
                if (cap != NONE) {
                    if (board.get_piece_color(m.to) == WHITE) captured_white.push_back(cap);
                    else captured_black.push_back(cap);
                } else if (m.is_en_passant) {
                    if (board.get_side_to_move() == WHITE) captured_black.push_back(PAWN);
                    else captured_white.push_back(PAWN);
                }

                // Animation Setup
                int from_f = ChessBoard::get_file(m.from);
                int from_r = ChessBoard::get_rank(m.from);
                int to_f = ChessBoard::get_file(m.to);
                int to_r = ChessBoard::get_rank(m.to);
                bool wb = is_white_at_bottom();
                int sz = get_square_size();
                
                AnimatingPiece anim;
                anim.piece = board.get_piece(m.from);
                anim.color = board.get_piece_color(m.from);
                anim.start_pos = sf::Vector2f((float)(wb ? from_f : 7-from_f) * sz, (float)(wb ? 7-from_r : from_r) * sz);
                anim.end_pos = sf::Vector2f((float)(wb ? to_f : 7-to_f) * sz, (float)(wb ? 7-to_r : to_r) * sz);
                anim.start_time = std::chrono::steady_clock::now();
                anim.duration_seconds = 0.2f;
                animations.push_back(anim);

                board.make_move(m);
                last_move = m; has_last_move = true; moved = true;
                move_start_time = std::chrono::steady_clock::now();
                
                if (board.is_checkmate(board.get_side_to_move())) {
                    game_ended = true; winner = (board.get_side_to_move() == WHITE ? BLACK : WHITE);
                } else if (board.is_stalemate(board.get_side_to_move())) {
                    game_ended = true;
                }
                break;
            }
        }
        
        is_square_selected = false; selected_square = NO_SQUARE; legal_moves_for_selected.clear();
        
        if (!moved) {
            PieceType pt = board.get_piece(sq);
            if (pt != NONE && board.get_piece_color(sq) == board.get_side_to_move()) {
                selected_square = sq; is_square_selected = true;
                std::vector<Move> all = board.generate_legal_moves();
                for (auto& m : all) if (m.from == sq) legal_moves_for_selected.push_back(m);
            }
        }
    } else {
        PieceType pt = board.get_piece(sq);
        if (pt != NONE && board.get_piece_color(sq) == board.get_side_to_move()) {
            selected_square = sq; is_square_selected = true;
            std::vector<Move> all = board.generate_legal_moves();
            for (auto& m : all) if (m.from == sq) legal_moves_for_selected.push_back(m);
        }
    }
    update_status_text();
}

void ChessGUI::handle_promotion_click(int x, int y) {
    (void)x; (void)y;
    pending_promotion_move.promotion = QUEEN;
    board.make_move(pending_promotion_move);
    last_move = pending_promotion_move; has_last_move = true;
    awaiting_promotion = false;
    move_start_time = std::chrono::steady_clock::now();
    update_status_text();
}

// --- DESENHO ---

void ChessGUI::draw_board() {
    int sq_size = get_square_size();
    bool white_bottom = is_white_at_bottom();
    
    // Draw Board Background
    sf::RectangleShape board_bg(sf::Vector2f(sq_size * 8, sq_size * 8));
    board_bg.setPosition(0, 0);
    window.draw(board_bg);

    for(int r=0; r<8; r++) {
        for(int f=0; f<8; f++) {
            sf::RectangleShape rect(sf::Vector2f(sq_size, sq_size));
            int dr = white_bottom ? 7-r : r;
            int df = white_bottom ? f : 7-f;
            rect.setPosition(df*sq_size, dr*sq_size);
            // [CORREÇÃO] A1 deve ser escura. (0+0)%2 == 0 -> Dark.
            bool is_dark = ((r+f)%2 == 0);
            rect.setFillColor(is_dark ? theme.dark_square : theme.light_square);
            window.draw(rect);
            
            // Coordenadas
            if (f == (white_bottom ? 0 : 7)) {
                sf::Text rank_label(std::to_string(r+1), font, sq_size/4);
                rank_label.setFillColor(is_dark ? theme.light_square : theme.dark_square);
                rank_label.setPosition(df*sq_size + 2, dr*sq_size + 2);
                window.draw(rank_label);
            }
            if (r == (white_bottom ? 0 : 7)) {
                std::string file_char = ""; file_char += (char)('a' + f);
                sf::Text file_label(file_char, font, sq_size/4);
                file_label.setFillColor(is_dark ? theme.light_square : theme.dark_square);
                file_label.setPosition(df*sq_size + sq_size - 15, dr*sq_size + sq_size - 20);
                window.draw(file_label);
            }
        }
    }
}

void ChessGUI::draw_pieces() {
    // Update animations
    auto now = std::chrono::steady_clock::now();
    animations.erase(std::remove_if(animations.begin(), animations.end(), [&](const AnimatingPiece& a) {
        std::chrono::duration<float> elapsed = now - a.start_time;
        return elapsed.count() > a.duration_seconds;
    }), animations.end());

    int sq_size = get_square_size();
    bool white_bottom = is_white_at_bottom();

    // Draw static pieces
    for(int r=0; r<8; r++) {
        for(int f=0; f<8; f++) {
            Square sq = ChessBoard::make_square(f, r);
            PieceType pt = board.get_piece(sq);
            if(pt != NONE) {
                // Check if this piece is animating TO this square
                bool is_animating = false;
                for(const auto& anim : animations) {
                    int dr = white_bottom ? 7-r : r;
                    int df = white_bottom ? f : 7-f;
                    int x = df * sq_size;
                    int y = dr * sq_size;
                    if (std::abs(anim.end_pos.x - x) < 2 && std::abs(anim.end_pos.y - y) < 2) {
                        is_animating = true;
                        break;
                    }
                }
                
                if (!is_animating) {
                    int dr = white_bottom ? 7-r : r;
                    int df = white_bottom ? f : 7-f;
                    draw_piece(window, pt, board.get_piece_color(sq), df*sq_size, dr*sq_size);
                }
            }
        }
    }

    // Draw animating pieces
    for(const auto& anim : animations) {
        std::chrono::duration<float> elapsed = now - anim.start_time;
        float t = elapsed.count() / anim.duration_seconds;
        if (t > 1.0f) t = 1.0f;
        
        // Ease out cubic
        t = 1.0f - std::pow(1.0f - t, 3.0f);

        float cur_x = anim.start_pos.x + (anim.end_pos.x - anim.start_pos.x) * t;
        float cur_y = anim.start_pos.y + (anim.end_pos.y - anim.start_pos.y) * t;
        
        draw_piece(window, anim.piece, anim.color, (int)cur_x, (int)cur_y);
    }
}

void ChessGUI::draw_piece(sf::RenderTarget& target, PieceType pt, Color c, int x, int y, float custom_size) {
    auto it = piece_textures.find({pt, c});
    if(it != piece_textures.end()) {
        sf::Sprite s(it->second);
        float scale = (custom_size / s.getLocalBounds().width) * 0.9f;
        s.setScale(scale, scale);
        float offset = (custom_size - (s.getLocalBounds().width * scale)) / 2.0f;
        s.setPosition(x + offset, y + offset);
        target.draw(s);
    } else {
        sf::Text t(get_piece_symbol(pt, c), font, (unsigned int)(custom_size/2));
        t.setPosition(x+custom_size/4, y+custom_size/4); target.draw(t);
    }
}

void ChessGUI::draw_piece(sf::RenderTarget& target, PieceType pt, Color c, int x, int y) {
    draw_piece(target, pt, c, x, y, (float)get_square_size());
}

bool ChessGUI::load_piece_textures() {
    std::string n[] = {"pawn", "knight", "bishop", "rook", "queen", "king"};
    for(int c=0; c<2; c++) for(int p=0; p<6; p++) {
        sf::Texture t;
        std::string path = "img/pieces/" + std::string(c==0?"white":"black") + "-" + n[p] + ".png";
        if(t.loadFromFile(path)) piece_textures[{(PieceType)p, (Color)c}] = t;
    }
    return true;
}

std::string ChessGUI::get_piece_image_path(PieceType pt, Color c) const { (void)pt; (void)c; return ""; }
std::string ChessGUI::get_piece_symbol(PieceType pt, Color c) const { 
    if (c==WHITE) return (const char*[]){"P","N","B","R","Q","K"}[pt];
    return (const char*[]){"p","n","b","r","q","k"}[pt];
}

void ChessGUI::draw_highlights() { 
    if(selected_square != NO_SQUARE) {
        int sz = get_square_size();
        int f = ChessBoard::get_file(selected_square);
        int r = ChessBoard::get_rank(selected_square);
        sf::RectangleShape rect(sf::Vector2f(sz, sz));
        rect.setFillColor(theme.highlight_color);
        bool wb = is_white_at_bottom();
        rect.setPosition((wb?f:7-f)*sz, (wb?7-r:r)*sz);
        window.draw(rect);
    }
}

void ChessGUI::draw_legal_moves() {
    int sz = get_square_size();
    bool wb = is_white_at_bottom();
    for(auto& m : legal_moves_for_selected) {
        int f = ChessBoard::get_file(m.to);
        int r = ChessBoard::get_rank(m.to);
        
        int x = (wb?f:7-f)*sz;
        int y = (wb?7-r:r)*sz;

        bool is_capture = (board.get_piece(m.to) != NONE) || (m.is_en_passant);

        if (is_capture) {
            // Donut shape for captures
            float radius = sz / 2.0f;
            float thickness = 6.0f;
            sf::CircleShape ring(radius - thickness);
            ring.setOutlineThickness(thickness);
            ring.setOutlineColor(theme.legal_move_color);
            ring.setFillColor(sf::Color::Transparent);
            ring.setPosition(x + thickness, y + thickness);
            window.draw(ring);
            
            // Or triangles in corners? No, ring is standard.
        } else {
            // Small dot for quiet moves
            float radius = sz / 6.0f;
            sf::CircleShape c(radius);
            c.setFillColor(theme.legal_move_color);
            c.setOrigin(radius, radius);
            c.setPosition(x + sz/2, y + sz/2);
            window.draw(c);
        }
    }
}

void ChessGUI::draw_last_move() {
    if(has_last_move) {
        int sz = get_square_size();
        bool wb = is_white_at_bottom();
        int f = ChessBoard::get_file(last_move.to);
        int r = ChessBoard::get_rank(last_move.to);
        sf::RectangleShape rect(sf::Vector2f(sz, sz));
        rect.setFillColor(theme.move_highlight_color);
        rect.setPosition((wb?f:7-f)*sz, (wb?7-r:r)*sz);
        window.draw(rect);
        f = ChessBoard::get_file(last_move.from);
        r = ChessBoard::get_rank(last_move.from);
        rect.setPosition((wb?f:7-f)*sz, (wb?7-r:r)*sz);
        window.draw(rect);
    }
}

void ChessGUI::draw_check_indicator() {
    if (board.is_check(board.get_side_to_move())) {
        Square k = find_king_square(board.get_side_to_move());
        int sz = get_square_size();
        bool wb = is_white_at_bottom();
        int f = ChessBoard::get_file(k);
        int r = ChessBoard::get_rank(k);
        
        // Gradient-like circle
        sf::CircleShape circle(sz/2);
        circle.setPosition((wb?f:7-f)*sz, (wb?7-r:r)*sz);
        circle.setFillColor(theme.check_color);
        window.draw(circle);
    }
}

void ChessGUI::draw_menu() {
    sf::Vector2u ws = window.getSize();
    float cx = ws.x / 2.0f; float cy = ws.y / 2.0f;
    float scale = std::min(ws.y/800.0f, ws.x/600.0f); scale = std::max(0.5f, std::min(1.2f, scale));
    
    sf::RectangleShape bg(sf::Vector2f(ws.x, ws.y)); bg.setFillColor(theme.bg_color); window.draw(bg);
    
    unsigned int t_size = (unsigned int)(52 * scale);
    float cur_y = cy - 150 * scale;
    
    sf::Text shadow("Jogo de Xadrez", font, t_size); shadow.setFillColor(sf::Color(0,0,0,100)); shadow.setStyle(sf::Text::Bold);
    sf::FloatRect sb = shadow.getLocalBounds(); shadow.setPosition(cx - sb.width/2 + 3, cur_y + 3); window.draw(shadow);
    
    sf::Text title("Jogo de Xadrez", font, t_size); title.setFillColor(theme.text_color); title.setStyle(sf::Text::Bold);
    title.setPosition(cx - sb.width/2, cur_y); window.draw(title);
    
    cur_y += 100 * scale;
    
    if (!selecting_time) {
        sf::Text color_title("Escolha sua cor:", font, (unsigned int)(24*scale)); 
        color_title.setFillColor(sf::Color(200,200,200));
        sf::FloatRect ct_b = color_title.getLocalBounds(); 
        color_title.setPosition(cx - ct_b.width/2, cur_y); 
        window.draw(color_title);
        
        cur_y += 50 * scale;
        
        float btn_width = 180 * scale;
        float btn_height = 50 * scale;
        float btn_spacing = 20 * scale;
        float white_btn_x = cx - btn_width - btn_spacing/2;
        float black_btn_x = cx + btn_spacing/2;
        
        sf::RectangleShape white_btn(sf::Vector2f(btn_width, btn_height));
        white_btn.setPosition(white_btn_x, cur_y);
        if (player_color == WHITE) {
            white_btn.setFillColor(theme.highlight_color);
            white_btn.setOutlineColor(sf::Color(255, 255, 255));
            white_btn.setOutlineThickness(3);
        } else {
            white_btn.setFillColor(theme.button_color);
            white_btn.setOutlineColor(sf::Color(150, 150, 150));
            white_btn.setOutlineThickness(2);
        }
        window.draw(white_btn);
        
        sf::Text white_text("BRANCAS", font, (unsigned int)(20*scale));
        white_text.setFillColor(sf::Color::White);
        white_text.setStyle(sf::Text::Bold);
        sf::FloatRect wt_b = white_text.getLocalBounds();
        white_text.setPosition(white_btn_x + btn_width/2 - wt_b.width/2, cur_y + btn_height/2 - wt_b.height/2);
        window.draw(white_text);
        
        sf::RectangleShape black_btn(sf::Vector2f(btn_width, btn_height));
        black_btn.setPosition(black_btn_x, cur_y);
        if (player_color == BLACK) {
            black_btn.setFillColor(theme.highlight_color);
            black_btn.setOutlineColor(sf::Color(255, 255, 255));
            black_btn.setOutlineThickness(3);
        } else {
            black_btn.setFillColor(theme.button_color);
            black_btn.setOutlineColor(sf::Color(150, 150, 150));
            black_btn.setOutlineThickness(2);
        }
        window.draw(black_btn);
        
        sf::Text black_text("PRETAS", font, (unsigned int)(20*scale));
        black_text.setFillColor(sf::Color::White);
        black_text.setStyle(sf::Text::Bold);
        sf::FloatRect bt_b = black_text.getLocalBounds();
        black_text.setPosition(black_btn_x + btn_width/2 - bt_b.width/2, cur_y + btn_height/2 - bt_b.height/2);
        window.draw(black_text);
    } else {
        std::string color_str = (player_color == WHITE) ? "BRANCAS" : "PRETAS";
        sf::Text color_info("Cor selecionada: " + color_str, font, (unsigned int)(20*scale));
        color_info.setFillColor(sf::Color(100, 200, 255));
        sf::FloatRect ci_b = color_info.getLocalBounds();
        color_info.setPosition(cx - ci_b.width/2, cur_y);
        window.draw(color_info);
        
        cur_y += 60 * scale;
        
        sf::Text time_title("Escolha o tempo:", font, (unsigned int)(24*scale));
        time_title.setFillColor(sf::Color(200,200,200));
        sf::FloatRect tt_b = time_title.getLocalBounds();
        time_title.setPosition(cx - tt_b.width/2, cur_y);
        window.draw(time_title);
        
        cur_y += 50 * scale;
        
        float btn_width = 120 * scale;
        float btn_height = 45 * scale;
        float btn_spacing = 15 * scale;
        float total_width = 4 * btn_width + 3 * btn_spacing;
        float start_x = cx - total_width / 2 + btn_width / 2;
        
        int times[] = {60, 180, 300, 600};
        std::string labels[] = {"1min", "3min", "5min", "10min"};
        
        for (int i = 0; i < 4; i++) {
            float btn_x = start_x + i * (btn_width + btn_spacing) - btn_width / 2;
            
            sf::RectangleShape time_btn(sf::Vector2f(btn_width, btn_height));
            time_btn.setPosition(btn_x, cur_y);
            if (initial_time_seconds == times[i]) {
                time_btn.setFillColor(sf::Color(100, 200, 100));
                time_btn.setOutlineColor(sf::Color(255, 255, 255));
                time_btn.setOutlineThickness(3);
            } else {
                time_btn.setFillColor(sf::Color(60, 60, 70));
                time_btn.setOutlineColor(sf::Color(150, 150, 150));
                time_btn.setOutlineThickness(2);
            }
            window.draw(time_btn);
            
            sf::Text time_text(labels[i], font, (unsigned int)(18*scale));
            time_text.setFillColor(sf::Color::White);
            time_text.setStyle(sf::Text::Bold);
            sf::FloatRect tt_b = time_text.getLocalBounds();
            time_text.setPosition(btn_x + btn_width/2 - tt_b.width/2, cur_y + btn_height/2 - tt_b.height/2);
            window.draw(time_text);
        }
    }
}

void ChessGUI::handle_menu_click(int x, int y) {
    sf::Vector2u ws = window.getSize();
    float cx = ws.x / 2.0f; float cy = ws.y / 2.0f;
    float scale = std::min(ws.y/800.0f, ws.x/600.0f); scale = std::max(0.5f, std::min(1.2f, scale));
    
    if (!selecting_time) {
        float cur_y = cy - 150 * scale;
        cur_y += 100 * scale;
        cur_y += 50 * scale;
        
        float btn_width = 180 * scale;
        float btn_height = 50 * scale;
        float btn_spacing = 20 * scale;
        float white_btn_x = cx - btn_width - btn_spacing/2;
        float black_btn_x = cx + btn_spacing/2;
        
        if (x >= white_btn_x && x <= white_btn_x + btn_width &&
            y >= cur_y && y <= cur_y + btn_height) {
            player_color = WHITE;
            selecting_time = true;
            return;
        }
        
        if (x >= black_btn_x && x <= black_btn_x + btn_width &&
            y >= cur_y && y <= cur_y + btn_height) {
            player_color = BLACK;
            selecting_time = true;
            return;
        }
    } else {
        float cur_y = cy - 150 * scale;
        cur_y += 100 * scale;
        cur_y += 60 * scale;
        cur_y += 50 * scale;
        
        float btn_width = 120 * scale;
        float btn_height = 45 * scale;
        float btn_spacing = 15 * scale;
        float total_width = 4 * btn_width + 3 * btn_spacing;
        float start_x = cx - total_width / 2 + btn_width / 2;
        
        int times[] = {60, 180, 300, 600};
        
        for (int i = 0; i < 4; i++) {
            float btn_x = start_x + i * (btn_width + btn_spacing) - btn_width / 2;
            
            if (x >= btn_x && x <= btn_x + btn_width &&
                y >= cur_y && y <= cur_y + btn_height) {
                initial_time_seconds = times[i];
                start_game();
                return;
            }
        }
    }
}

void ChessGUI::draw_thinking_indicator() {
    // Agora integrado na sidebar
}

void ChessGUI::draw_game_over() {
    // Desenha um painel semi-transparente sobre a tela
    sf::Vector2u size = window.getSize();
    sf::RectangleShape overlay(sf::Vector2f(size.x, size.y));
    overlay.setFillColor(sf::Color(0, 0, 0, 150));
    window.draw(overlay);

    sf::Text t(winner==WHITE?"White Wins!":"Black Wins!", font, 50);
    t.setFillColor(sf::Color::Green); 
    t.setOutlineColor(sf::Color::Black); t.setOutlineThickness(2);
    sf::FloatRect b = t.getLocalBounds();
    t.setPosition(size.x/2 - b.width/2, size.y/2 - b.height/2); 
    window.draw(t);
    
    sf::Text sub("Press ESC to return to menu", font, 20);
    sub.setFillColor(sf::Color::White);
    sf::FloatRect sb = sub.getLocalBounds();
    sub.setPosition(size.x/2 - sb.width/2, size.y/2 + 60);
    window.draw(sub);
}

void ChessGUI::handle_game_over_click(int x, int y) { 
    (void)x; (void)y;
    reset_game(); 
    current_state = MENU;
    selecting_time = false;
    player_color = WHITE;
}

void ChessGUI::draw_promotion_menu() {
    sf::Text t("Auto-Promoting...", font, 30); t.setFillColor(sf::Color::Yellow);
    t.setPosition(get_board_size()/2 - t.getLocalBounds().width/2, get_board_size()/2); window.draw(t);
}

bool ChessGUI::handle_resign_button_click(int x, int y) { (void)x; (void)y; return false; }

void ChessGUI::update_clocks() {
    if (game_ended || current_state != PLAYING) return;
    float dt = 1.0f / 60.0f; 
    
    if (board.get_side_to_move() == WHITE) {
        white_time_seconds -= dt;
        if (white_time_seconds < 0) { white_time_seconds = 0; resign(WHITE); }
    } else {
        black_time_seconds -= dt;
        if (black_time_seconds < 0) { black_time_seconds = 0; resign(BLACK); }
    }
}

void ChessGUI::draw_clocks_and_captured() {
    // Substituído pela sidebar
}

void ChessGUI::reset_game() {
    board = ChessBoard();
    game_started = true;
    game_ended = false;
    current_state = PLAYING;
    legal_moves_for_selected.clear();
    is_square_selected = false;
    captured_white.clear(); captured_black.clear();
    is_thinking = false;
    move_ready = false;
    white_time_seconds = initial_time_seconds;
    black_time_seconds = initial_time_seconds;
    current_eval = 0;
    update_status_text();
}

void ChessGUI::update_status_text() {
    std::string s = (board.get_side_to_move()==WHITE ? "White" : "Black");
    s += " to move";
    status_text.setString(s);
}

Square ChessGUI::find_king_square(Color c) const {
    for(int i=0; i<64; i++) if(board.get_piece(i)==KING && board.get_piece_color(i)==c) return i;
    return NO_SQUARE;
}

// --- FUNÇÕES AUXILIARES ---

void ChessGUI::start_game() {
    reset_game();
    current_state = PLAYING;
    move_start_time = std::chrono::steady_clock::now();
}

void ChessGUI::resign(Color c) {
    game_ended = true;
    winner = (c == WHITE) ? BLACK : WHITE;
}

std::string ChessGUI::format_time(int seconds) const {
    int m = seconds / 60;
    int s = seconds % 60;
    std::ostringstream oss;
    oss << std::setfill('0') << std::setw(2) << m << ":" << std::setw(2) << s;
    return oss.str();
}

// --- NOVA UI: IMPLEMENTAÇÃO ---

int ChessGUI::calculate_material_score(const std::vector<PieceType>& captured) {
    int score = 0;
    int values[] = {1, 3, 3, 5, 9, 0}; // P, N, B, R, Q, K
    for(PieceType pt : captured) {
        if(pt >= 0 && pt < 6) score += values[pt];
    }
    return score;
}



void ChessGUI::draw_sidebar() {
    int board_sz = get_board_size();
    float panel_x = (float)board_sz;
    float panel_w = (float)get_panel_width();
    float panel_h = (float)window.getSize().y;
    
    sf::RectangleShape panel(sf::Vector2f(panel_w, panel_h));
    panel.setFillColor(theme.sidebar_color);
    panel.setPosition(panel_x, 0);
    window.draw(panel);
    
    float margin = 20.0f;
    float card_h = 120.0f;
    
    Color bot_color = is_white_at_bottom() ? WHITE : BLACK;
    Color top_color = (bot_color == WHITE) ? BLACK : WHITE;
    
    float top_time = (top_color == WHITE) ? white_time_seconds : black_time_seconds;
    float bot_time = (bot_color == WHITE) ? white_time_seconds : black_time_seconds;
    
    const auto& pieces_captured_by_top = (top_color == WHITE) ? captured_black : captured_white;
    const auto& pieces_captured_by_bot = (bot_color == WHITE) ? captured_black : captured_white;

    // Top Card
    draw_player_card(panel_x + margin, margin, panel_w - 2*margin, card_h, top_color, top_time, pieces_captured_by_top);
    
    // Mid Area
    float mid_y = margin + card_h + 20;
    float mid_h = panel_h - 2 * (margin + card_h + 20);
    
    sf::RectangleShape mid_area(sf::Vector2f(panel_w - 2*margin, mid_h));
    mid_area.setFillColor(sf::Color(30, 30, 30));
    mid_area.setPosition(panel_x + margin, mid_y);
    window.draw(mid_area);
    
    status_text.setCharacterSize(18);
    sf::FloatRect bounds = status_text.getLocalBounds();
    status_text.setPosition(panel_x + panel_w/2 - bounds.width/2, mid_y + 20);
    window.draw(status_text);
    
    if (is_thinking) {
        sf::Text t("Thinking...", font, 16);
        t.setFillColor(sf::Color::Cyan);
        t.setPosition(panel_x + margin + 10, mid_y + 60);
        window.draw(t);
    }
    
    // Bot Card
    draw_player_card(panel_x + margin, panel_h - card_h - margin, panel_w - 2*margin, card_h, bot_color, bot_time, pieces_captured_by_bot);
}

void ChessGUI::draw_player_card(float x, float y, float w, float h, Color c, float time_left, const std::vector<PieceType>& captured) {
    sf::RectangleShape card(sf::Vector2f(w, h));
    card.setFillColor(sf::Color(45, 43, 40));
    card.setPosition(x, y);
    window.draw(card);
    
    float avatar_sz = 50.0f;
    sf::RectangleShape avatar(sf::Vector2f(avatar_sz, avatar_sz));
    avatar.setPosition(x + 10, y + 10);
    avatar.setFillColor(c == WHITE ? sf::Color::White : sf::Color::Black);
    window.draw(avatar);
    
    sf::Text name(c == WHITE ? "Player (White)" : "Computer (Black)", font, 18);
    name.setFillColor(sf::Color::White);
    name.setPosition(x + 10 + avatar_sz + 10, y + 10);
    
    // Scale text if it's too wide
    // Available width = Total width - (Left Margin + Avatar + Padding) - (Right Margin)
    float left_offset = 10 + avatar_sz + 10;
    float max_text_width = w - left_offset - 10;
    
    if (name.getLocalBounds().width > max_text_width) {
        float scale = max_text_width / name.getLocalBounds().width;
        name.setScale(scale, scale);
    }
    
    window.draw(name);
    
    int score_white = calculate_material_score(captured_white);
    int score_black = calculate_material_score(captured_black);
    
    int adv = 0;
    if (c == WHITE) adv = score_black - score_white;
    else adv = score_white - score_black;
    
    if (adv > 0) {
        sf::Text mat_txt("+" + std::to_string(adv), font, 14);
        mat_txt.setFillColor(sf::Color(150, 150, 150));
        mat_txt.setPosition(x + 10 + avatar_sz + 10, y + 35);
        window.draw(mat_txt);
    }

    // Draw Clock (Bottom Right)
    bool is_turn = (board.get_side_to_move() == c);
    sf::RectangleShape clock_bg(sf::Vector2f(100, 40));
    // Position at bottom right: x + w - 110, y + h - 50
    float clock_x = x + w - 110;
    float clock_y = y + h - 50;
    clock_bg.setPosition(clock_x, clock_y);
    
    if (is_turn && current_state == PLAYING && !game_ended) {
        clock_bg.setFillColor(sf::Color(200, 200, 200));
    } else {
        clock_bg.setFillColor(sf::Color(30, 30, 30));
    }
    clock_bg.setOutlineThickness(1);
    clock_bg.setOutlineColor(sf::Color(60, 60, 60));
    window.draw(clock_bg);
    
    sf::Text time_txt(format_time((int)time_left), font, 24);
    time_txt.setStyle(sf::Text::Bold);
    if (is_turn && current_state == PLAYING && !game_ended) {
        time_txt.setFillColor(sf::Color::Black);
    } else {
        time_txt.setFillColor(time_left < 30 ? sf::Color(255, 80, 80) : sf::Color(150, 150, 150));
    }
    
    sf::FloatRect b = time_txt.getLocalBounds();
    time_txt.setPosition(clock_x + 50 - b.width/2, clock_y + 20 - b.height/2 - 5);
    window.draw(time_txt);

    // Draw Captured Pieces (Bottom Left, avoiding clock)
    float cap_x = x + 10;
    float cap_y = y + 70;
    float cap_sz = 20.0f;
    float cap_spacing = 10.0f;
    float max_cap_x = clock_x - 10; // Stop before the clock
    
    for(PieceType pt : captured) {
        draw_piece(window, pt, (c==WHITE?BLACK:WHITE), cap_x, cap_y, cap_sz);
        cap_x += cap_spacing;
        if (cap_x > max_cap_x) {
            // If we run out of space, just stop drawing (or wrap to a mini row if we had space, but we don't)
            break; 
        }
    }
}