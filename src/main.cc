#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <cstdint>
#include <iostream>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <SFML/Graphics.hpp>

enum cellstate{empty = 0, black = 1, white = 2, ko = 3}; // .#OX
enum cellcolor{emptycolor = 0x0000000, whitecolor = 0xffffffff, blackcolor = 0x000000ff};
// Idiom: other color = color^3;
// Idiom: Corrupted Cell if (cell & 252 != 0);

typedef struct{
    uint8_t current_color;
    unsigned int black_prisoners;
    unsigned int white_prisoners;
    uint8_t board[19][19];
} Board;

bool in_bounds(uint8_t x, uint8_t y){
    return 0 <= x && x < 19 && 0 <= y && y < 19;
}

void board_clean_flags(Board *B){ // All cells cast to .#OX
    for (int i = 0; i < 19; ++i){
        for (int j = 0; j < 19; ++j){
            B->board[i][j] &= 3;
        }
    }
}

void board_print(Board *B){
    for (int i = 0; i < 19; ++i){
        for (int j = 0; j < 19; ++j){
            printf("%c%c", ".#OX*#OX"[B->board[i][j] & 3 + ((i%6 == 3 && j%6 == 3) << 2)], " \n"[j%19 == 18]);
        }
    }
    printf("Prisoners. B: %i, W: %i\nCurrent Move: %c", B->black_prisoners, B->white_prisoners, "BW"[B->current_color == white]);
}

void board_remove_group(Board *B, uint8_t x, uint8_t y){
    uint8_t *current_cell;
    for (int i = 0; i < 19; ++i){
        for (int j = 0; j < 19; ++j){
            current_cell = &B->board[i][j];
            if ((*current_cell & 4) != 0){
                B->black_prisoners += (*current_cell & 3) == white;
                B->white_prisoners += (*current_cell & 3) == black;
                *current_cell = empty;
            }
        }
    }
}

bool board_has_liberty(Board *B, uint8_t x, uint8_t y){
    uint8_t check_cell, current_cell_color = B->board[y][x] & 3; // check_cell uninitialized
    if (current_cell_color == empty) return true;
    B->board[y][x] |= 4;
    int x_off, y_off;
    for (int k = 0; k < 4; ++k){
        x_off = x + ( ((k&1)-1) & (1-(k&2))); // 1, 0, -1,  0
        y_off = y + (~((k&1)-1) & (1-(k&2))); // 0, 1,  0, -1
        if (in_bounds(x_off, y_off)){
            check_cell = B->board[y_off][x_off];
            if (check_cell == empty || 
                check_cell == current_cell_color && board_has_liberty(B, x_off, y_off)) 
                return true; // && higher precendence than || lmao
        }
    }
    return false;
}

bool board_play_move(Board *B, uint8_t x, uint8_t y){
    // Returns whether placement was successful
    //assert(in_bounds(x, y));
    if (!in_bounds(x, y)) return false;
    uint8_t *current_cell = &B->board[y][x];
    uint8_t color = B->current_color;
    if (*current_cell != empty) {
        if (*current_cell == color    ) printf("Cell of own color at %i, %i!\n", x, y);
        if (*current_cell == (color^3)) printf("Cell of other color at %i, %i!\n", x, y);
        if (*current_cell == ko       ) printf("Ko cell at %i, %i!\n", x, y);
        return false;
    }

    *current_cell = color;
    // Check if the tentatively placed stone kills a group
    bool is_kill = false;
    int x_off, y_off;
    for (int k = 0; k < 4; ++k){
        x_off = x + ( ((k&1)-1) & (1-(k&2))); // 1, 0, -1,  0
        y_off = y + (~((k&1)-1) & (1-(k&2))); // 0, 1,  0, -1
        if (in_bounds(x_off, y_off)){
            if ((B->board[y_off][x_off]&3) == (color^3) && !board_has_liberty(B, x_off, y_off)){
                is_kill = true;
                board_remove_group(B, x_off, y_off);
            } else board_clean_flags(B);
        }
    } // guaranteed clean flag
    if (!is_kill){
        if (!board_has_liberty(B, x, y)){
            printf("Suicide Move at %i, %i!\n", x, y);
            *current_cell = empty;
            board_clean_flags(B); 
            return false;
        }
        board_clean_flags(B);
    }
    printf("Played Move at %i, %i.\n", x, y);
    B->current_color ^= 3;
    return true;
}

void board_setup_game(Board *B){
    B->current_color = black;
    B->black_prisoners = B->white_prisoners = 0;
    for (int i = 0; i < 19; ++i){
        for (int j = 0; j < 19; ++j){
            B->board[i][j] = empty;
        }
    }
}

void board_stone_color_update(Board B, std::vector<sf::CircleShape> *stones){
    int color;
    for (int i = 0; i < 19; ++i){
        for (int j = 0; j < 19; ++j){
            color = B.board[i][j];
            (*stones)[j+19*i].setFillColor(sf::Color(color == white ? whitecolor : color == black ? blackcolor : emptycolor));
        }
    }
}

int main(){
    Board B1;
    board_setup_game(&B1);

    constexpr uint16_t WIDTH  = 1000;
    constexpr uint16_t HEIGHT = 800;
    constexpr uint16_t LINE_THICKNESS = 2;
    constexpr float CIRCLE_RADIUS = 5.0f;
    float STONE_SIZE = (std::max((unsigned short)100, std::min(WIDTH, HEIGHT)) - 100) / 36.f; 
    constexpr uint16_t  LEFT_BOARD_BUFFER = 144;
    constexpr uint16_t RIGHT_BOARD_BUFFER = 144;
    constexpr uint16_t    UP_BOARD_BUFFER = 44;
    constexpr uint16_t  DOWN_BOARD_BUFFER = 44;

    sf::RenderWindow window(sf::VideoMode(WIDTH, HEIGHT), "SFML works!");

    sf::RectangleShape b(sf::Vector2f(WIDTH, HEIGHT));
    b.setFillColor(sf::Color(0xffdb8cff));
    b.setPosition(sf::Vector2f(0, 0));

    std::vector<sf::RectangleShape> lines;
    for (int k = 0; k < 19; ++k){
        lines.push_back(sf::RectangleShape(sf::Vector2f(WIDTH - LEFT_BOARD_BUFFER - RIGHT_BOARD_BUFFER + LINE_THICKNESS, LINE_THICKNESS)));
        lines.back().setPosition(LEFT_BOARD_BUFFER, static_cast<int>(UP_BOARD_BUFFER + (k * (HEIGHT - UP_BOARD_BUFFER - DOWN_BOARD_BUFFER))/18));
        lines.back().setFillColor(sf::Color::Black);
    }
    for (int k = 0; k < 19; ++k){
        lines.push_back(sf::RectangleShape(sf::Vector2f(LINE_THICKNESS, HEIGHT - UP_BOARD_BUFFER - DOWN_BOARD_BUFFER + LINE_THICKNESS)));
        lines.back().setPosition(static_cast<int>(LEFT_BOARD_BUFFER + (k * (WIDTH - LEFT_BOARD_BUFFER - RIGHT_BOARD_BUFFER))/18), UP_BOARD_BUFFER);
        lines.back().setFillColor(sf::Color::Black);
    }
    std::vector<sf::CircleShape> circles;
    for (int k = 0; k < 9; ++k){
        circles.push_back(sf::CircleShape(CIRCLE_RADIUS, 32));
        circles.back().setPosition( 
            LEFT_BOARD_BUFFER + static_cast<int>(((3 + 6*(k/3)) * (HEIGHT -   UP_BOARD_BUFFER -  DOWN_BOARD_BUFFER))/18) - CIRCLE_RADIUS + LINE_THICKNESS/2.0f,
              UP_BOARD_BUFFER + static_cast<int>(((3 + 6*(k%3)) * (WIDTH  - LEFT_BOARD_BUFFER - RIGHT_BOARD_BUFFER))/18) - CIRCLE_RADIUS + LINE_THICKNESS/2.0f);
        circles.back().setFillColor(sf::Color::Black);
    }

    std::vector<sf::CircleShape> stones;
    for (int k = 0; k < 361; ++k){
        stones.push_back(sf::CircleShape(STONE_SIZE, 32));
        stones.back().setPosition(
            LEFT_BOARD_BUFFER + static_cast<int>(((k%19)    * (HEIGHT -   UP_BOARD_BUFFER -  DOWN_BOARD_BUFFER))/18) - STONE_SIZE + LINE_THICKNESS/2.0f,
              UP_BOARD_BUFFER + static_cast<int>(((k/19%19) * (WIDTH  - LEFT_BOARD_BUFFER - RIGHT_BOARD_BUFFER))/18) - STONE_SIZE + LINE_THICKNESS/2.0f
        );
        stones.back().setFillColor(sf::Color(blackcolor));
    }

    board_stone_color_update(B1, &stones);

    window.clear();
    window.draw(b);
    for (auto line : lines) window.draw(line);
    for (auto circle : circles) window.draw(circle);
    for (auto stone : stones) window.draw(stone);
    window.display();



    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
            if (event.type == sf::Event::MouseButtonPressed){
                sf::Vector2i position = sf::Mouse::getPosition(window);
                int x = (position.x - LEFT_BOARD_BUFFER + STONE_SIZE) / (2*STONE_SIZE);
                int y = (position.y -   UP_BOARD_BUFFER + STONE_SIZE) / (2*STONE_SIZE);
                printf("px: %i, py: %i, x: %i, y: %i\n", position.x, position.x, x, y);
                board_play_move(&B1, x, y);

                board_stone_color_update(B1, &stones);

                window.clear();
                window.draw(b);
                for (auto line : lines) window.draw(line);
                for (auto circle : circles) window.draw(circle);
                for (auto stone : stones) window.draw(stone);
                window.display();
            }
        }

        
    }
}
// g++ -c src/main.cc && g++ main.o -o Go -lsfml-graphics -lsfml-window -lsfml-system && ./Go