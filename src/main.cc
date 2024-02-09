#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <SFML/Graphics.hpp>

enum cellstate{empty = 0, black = 1, white = 2, ko = 3}; // .#OX
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

void board_SFML_print(Board *B){
    /// not done
}

int main(){
    Board B1;
    board_fill_empty(&B1);
    board_reset_prisoners(&B1);
    //board_play_move(&B1, 1, 0, black);
    //board_play_move(&B1, 0, 0, white);
    //board_play_move(&B1, 0, 1, black);
    //board_print(&B1);

    sf::RenderWindow window(sf::VideoMode(200, 200), "SFML works!");

    sf::RectangleShape b(sf::Vector2f(200, 200));
    b.setFillColor(sf::Color(0xffdb8cff));
    b.setPosition(sf::Vector2f(0, 0));

    std::vector<sf::RectangleShape> lines;
    for (int k = 0; k < 20; ++k){
        lines.push_back(sf::RectangleShape(sf::Vector2f(180, 1)));
        lines.back().setPosition(sf::Vector2f(10, 10 + k * 10));
        lines.back().setFillColor(sf::Color::Black);
    }
    for (int k = 0; k < 20; ++k){
        lines.push_back(sf::RectangleShape(sf::Vector2f(1, 180)));
        lines.back().setPosition(sf::Vector2f(10 + k * 10, 10));
        lines.back().setFillColor(sf::Color::Black);
    }

    

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        window.clear();
        window.draw(b);
        for (auto line : lines) window.draw(line);
        window.display();
    }
}
// g++ src/main.cc -o Go && ./Go
// g++ -c src/main.cc && g++ main.o -o Go -lsfml-graphics -lsfml-window -lsfml-system && ./Go