#include <stdio.h>
#include <stdint.h>
#include <assert.h>

enum cellstate{empty = 0, black = 1, white = 2, ko = 3}; // .#OX C
// Idiom: other color = color^3;
// Idiom: Corrupted Cell if (cell & 252 != 0);

typedef struct{
    uint8_t black_prisoners;
    uint8_t white_prisoners;
    uint8_t board[19][19];
} Board;

uint8_t in_bounds(uint8_t x, uint8_t y){
    return 0 <= x && x < 19 && 0 <= y && y < 19;
}

void board_default_fill(Board *B){
    for (int k = 0; k < 361; ++k){
        B->board[(k/19)%19][k%19] = empty;
    }
}

void board_reset_prisoners(Board *B){
    B->black_prisoners = 0;
    B->white_prisoners = 0;
}

void board_clean_flags(Board *B){ // All cells cast to .#OX
    for (int k = 0; k < 361; ++k){
        B->board[(k/19)%19][k%19] &= 3;
    }
}

void board_print(Board *B){
    int which_char;
    for (int k = 0; k < 361; ++k){
        which_char = B->board[(k/19)%19][k%19];
        which_char += (which_char == empty && k%6 == 0 && k%19%6 == 3) << 2;
        fputc(".#OC*"[which_char], stdout);
        fputc(" \n"[k%19 == 18], stdout);
    }
    printf("Prisoners. B: %i, W: %i\n", B->black_prisoners, B->white_prisoners);
}

void board_write(Board *B, uint8_t x, uint8_t y, enum cellstate color){
    assert(in_bounds(x, y));
    B->board[y][x] = color;
}

void board_remove_group(Board *B, uint8_t x, uint8_t y){
    for (int k = 0; k < 361; k++){
        if ((B->board[(k/19)%19][k%19] & 4) != 0){
            if ((B->board[(k/19)%19][k%19] & 3) == black){
                B->white_prisoners += 1;
            }
            if ((B->board[(k/19)%19][k%19] & 3) == white){
                B->black_prisoners += 1;
            }
            B->board[(k/19)%19][k%19] = empty;
        }
    }
}

uint8_t board_has_liberty(Board *B, uint8_t x, uint8_t y){
    uint8_t current_cell_color = B->board[y][x] & 3;
    uint8_t check_cell;
    if (current_cell_color == empty) return 1;
    B->board[y][x] |= 4;
    int x_off;
    int y_off;
    for (int k = 0; k < 4; k++){
        x_off = x + ( ((k&1)-1) & (1-(k&2)));
        y_off = y + (~((k&1)-1) & (1-(k&2)));
        if (in_bounds(x_off, y_off)){
            check_cell = B->board[y_off][x_off];
            if (check_cell == empty || 
                check_cell == current_cell_color && board_has_liberty(B, x_off, y_off)) 
                return 1; // && higher precendence than ||
        }
    }
    return 0;
}

uint8_t board_play_move(Board *B, uint8_t x, uint8_t y, cellstate color){
    // Return 1 means illegal move, Return 0 means placed stone
    // Assumes Cleaned Flags
    assert(in_bounds(x, y));
    if (B->board[y][x] != empty) {
        if (B->board[y][x] == color)
            printf("Cell of own color at %i, %i!\n", x, y);
        if (B->board[y][x] == (color^3))
            printf("Cell of other color at %i, %i!\n", x, y);
        if (B->board[y][x] == ko)
            printf("Ko cell at %i, %i!\n", x, y);
        return 1;
    }

    B->board[y][x] = color;
    // Check if the tentatively placed stone kills a group
    uint8_t is_kill;
    int x_off, y_off;
    for (int k = 0; k < 4; k++){
        x_off = x + ( ((k&1)-1) & (1-(k&2))); // 1, 0, -1,  0
        y_off = y + (~((k&1)-1) & (1-(k&2))); // 0, 1,  0, -1
        if (in_bounds(x_off, y_off)){
            if ((B->board[y_off][x_off]&3) == (color^3) && !board_has_liberty(B, x_off, y_off)){
                is_kill = 1;
                board_remove_group(B, x_off, y_off);
            } else board_clean_flags(B);
        }
    }
    if (!is_kill && !board_has_liberty(B, x, y)){
        printf("Suicide Move at %i, %i!\n", x, y);
        B->board[y][x] = empty;
        board_clean_flags(B);
        return 1;
    }
    board_clean_flags(B);
    printf("Played Move at %i, %i.\n", x, y);
    return 0;
}

void board_play(Board *B){
    uint8_t color = black;
    /// not done
}

int main(){
    Board B1;
    board_default_fill(&B1);
    board_reset_prisoners(&B1);
    board_play_move(&B1, 1, 0, black);
    board_play_move(&B1, 0, 0, white);
    board_play_move(&B1, 0, 1, black);
    board_print(&B1);
    
}
// g++ src/main.cc -o Go && ./Go