#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <cstdint>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <SFML/Graphics.hpp>

enum cellstate{empty = 0, black = 1, white = 2, ko = 3}; // .#OX
enum cellcolor{emptycolor = 0x0000000, whitecolor = 0xffffffff, blackcolor = 0x000000ff};
// Idiom: other color = color^3;
// Idiom: Corrupted Cell if (cell & 252 != 0);

typedef struct{
    unsigned int black_prisoners;
    unsigned int white_prisoners;
    uint8_t board[19][19];
} Board;

bool in_bounds(uint8_t x, uint8_t y){
    return 0 <= x && x < 19 && 0 <= y && y < 19;
}

void board_reset_prisoners(Board *B){
    B->black_prisoners = B->white_prisoners = 0;
}

void board_fill_empty(Board *B){
    for (int k = 0; k < 361; ++k){
        B->board[k/19%19][k%19] = empty;
    }
}

void board_clean_flags(Board *B){ // All cells cast to .#OX
    for (int k = 0; k < 361; ++k){
        B->board[k/19%19][k%19] &= 3;
    }
}

void board_print(Board *B){
    for (int k = 0; k < 361; ++k){
        printf("%c%c", ".#OX*#OX"[B->board[k/19%19][k%19] & 3 + ((k%6 == 0 && k%19%6 == 3) << 2)], " \n"[k%19 == 18]);
    }
    printf("Prisoners. B: %i, W: %i\n", B->black_prisoners, B->white_prisoners);
}

void board_remove_group(Board *B, uint8_t x, uint8_t y){
    uint8_t *current_cell;
    for (int k = 0; k < 361; ++k){
        current_cell = &B->board[k/19%19][k%19];
        if ((*current_cell & 4) != 0){
            B->black_prisoners += (*current_cell & 3) == white;
            B->white_prisoners += (*current_cell & 3) == black;
            *current_cell = empty;
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

bool board_play_move(Board *B, uint8_t x, uint8_t y, cellstate color){
    // Returns whether placement was successful
    assert(in_bounds(x, y));
    uint8_t *current_cell = &B->board[y][x];
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
    return true;
}

void board_play(Board *B){
    /// not done
}

void board_stone_color_update(Board B, std::vector<sf::CircleShape> *stones){
    int color;
    for (int k = 0; k < 361; ++k){
        color = B.board[k/19%19][k%19];
        (*stones)[k].setFillColor(sf::Color(color == white ? whitecolor : color == black ? blackcolor : emptycolor));
    }
}

int main(){
    Board B1;
    board_fill_empty(&B1);
    board_reset_prisoners(&B1);
    B1.board[2][3] = black;
    B1.board[5][3] = white;
    B1.board[1][3] = black;
    B1.board[2][7] = black;

    constexpr uint16_t WIDTH  = 1000;
    constexpr uint16_t HEIGHT = 800;
    constexpr uint16_t LINE_THICKNESS = 2;
    constexpr float CIRCLE_RADIUS = 5.0f;
    float STONE_SIZE = (std::max((unsigned short)100, std::min(WIDTH, HEIGHT)) - 100) / 35.f; 
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
        stones.push_back(sf::CircleShape(19, 32));
        stones.back().setPosition(
            LEFT_BOARD_BUFFER + static_cast<int>(((k%19)    * (HEIGHT -   UP_BOARD_BUFFER -  DOWN_BOARD_BUFFER))/18) - 19 + LINE_THICKNESS/2.0f,
              UP_BOARD_BUFFER + static_cast<int>(((k/19%19) * (WIDTH  - LEFT_BOARD_BUFFER - RIGHT_BOARD_BUFFER))/18) - 19 + LINE_THICKNESS/2.0f
        );
        stones.back().setFillColor(sf::Color(blackcolor));
    }

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        board_stone_color_update(B1, &stones);

        window.clear();
        window.draw(b);
        for (auto line : lines) window.draw(line);
        for (auto circle : circles) window.draw(circle);
        for (auto stone : stones) window.draw(stone);
        window.display();
    }
}
// g++ -c src/main.cc && g++ main.o -o Go -lsfml-graphics -lsfml-window -lsfml-system && ./Go