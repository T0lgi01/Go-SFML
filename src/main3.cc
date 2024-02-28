#include <cstdint>
#include <stdio.h>
#include <stdint.h>
#include <vector>
#include <iostream>

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
            printf("%c%c", ".#OX*#OX"[B->board[i][j] & 3 + ((i%6 == 3 && j%6 == 3) << 2)], " \n"[j == 18]);
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
    //printf("Played Move at %i, %i.\n", x, y);
    B->current_color ^= 3;
    return true;
}

void board_setup_game(Board *B){
    B->current_color = black;
    B->black_prisoners = B->white_prisoners = 0;
    for (int k = 0; k < 361; ++k){
        B->board[k/19%19][k%19] = empty;
    }
}

void __attribute__((noinline)) board_print1(Board *B){ // 5684 cycles on -O3
    for (int i = 0; i < 19; ++i){
        for (int j = 0; j < 19; ++j){
            printf("%c%c", ".#OX*#OX"[(B->board[i][j] & 3) + ((i%6 == 3 && j%6 == 3) << 2)], " \n"[j == 18]);
        }
    }
}

void __attribute__((noinline)) board_print2(Board *B){ // 7876 cycles on -O3
    for (int i = 0; i < 19; ++i){
        for (int j = 0; j < 19; ++j){
            std::cout << ".#OX*#OX"[(B->board[i][j] & 3) + ((i%6 == 3 && j%6 == 3) << 2)] << " \n"[j == 18];
        }
    }
}

void __attribute__((noinline)) board_print3(Board *B){ // 4112 cycles on -O3
    uint8_t which;
    for (int i = 0; i < 19; ++i){
        for (int j = 0; j < 19; ++j){
            which = B->board[i][j] & 3;
            if (which == empty && i%6 == 3 && j%6 == 3){
                printf("*");
            }
            else if (which == empty){
                printf(".");
            }
            else if (which == black){
                printf("#");
            }
            else if (which == white){
                printf("O");
            }
            else {
                printf("X");
            }
            if (j == 18){
                printf("\n");
            }
            else {
                printf(" ");
            }
        }
    }
}

void __attribute__((noinline)) board_print4(Board *B){ // 5219 cycles on -O3
    uint8_t which;
    for (int i = 0; i < 19; ++i){
        for (int j = 0; j < 19; ++j){
            which = B->board[i][j] & 3;
            if (which == empty && i%6 == 3 && j%6 == 3){
                printf("*");
            }
            else if (which == empty){
                printf(".");
            }
            else if (which == black){
                printf("#");
            }
            else if (which == white){
                printf("O");
            }
            else {
                printf("X");
            }
            printf("%c", " \n"[j == 18]);
        }
    }
}

void __attribute__((noinline)) board_print5(Board *B){ // 4125 cycles on -O3
    uint8_t which;
    for (int i = 0; i < 19; ++i){
        for (int j = 0; j < 19; ++j){
            which = B->board[i][j] & 3;
            if (which == empty){
                if (i%6 == 3 && j%6 == 3){
                    printf("*");
                } else {
                    printf(".");
                }  
            }
            else if (which == black){
                printf("#");
            }
            else if (which == white){
                printf("O");
            }
            else {
                printf("X");
            }
            if (j == 18){
                printf("\n");
            }
            else {
                printf(" ");
            }
        }
    }
}

int main(){
    Board B1; 
    board_setup_game(&B1);

    int moves[][2] = {{2, 0}, {3, 0}, {1, 0}, {2, 1}, {13, 17}, {1, 1}, {0, 0}, {0, 1}};

    for (auto move : moves) {
        board_play_move(&B1, move[0], move[1]);
    }

    board_print1(&B1);
    board_print2(&B1);
    board_print3(&B1);
    board_print4(&B1);
    board_print5(&B1);

    return 0;
} 
// g++ -O3 -g -c src/main3.cc && g++ main3.o -o Go3 && ./Go3
// rm ./callgrind*
// valgrind --tool=callgrind --dump-instr=yes --collect-jumps=yes ./Go3
// kcachegrind
// g++ -O3 -g -c src/main3.cc && g++ main3.o -o Go3 && rm ./callgrind* && valgrind --tool=callgrind --dump-instr=yes --collect-jumps=yes ./Go3 && kcachegrind