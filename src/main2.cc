#include <cstdint>
#include <random>

typedef struct{
    uint8_t board[19][19];
} Board;

void board_setup_game(Board *B){
    uint8_t i = 0;
    uint8_t j = 0;
    for (; i < 19; ++i){
        for (j = 0; j < 19; ++j){
            B->board[i][j] = std::rand() % 256;
        }
    }
}

void __attribute__((noinline)) board_clean_flags1(Board *B){
    for (int k = 0; k < 361; ++k){
        B->board[k/19%19][k%19] &= 3;
    }
}

void __attribute__((noinline)) board_clean_flags2(Board *B){
    int j = 0;
    for (int i = 0; i < 19; ++i){
        for (j = 0; j < 19; ++j){
            B->board[i][j] &= 3;
        }
    }
}

void __attribute__((noinline)) board_clean_flags3(Board *B){
    for (int i = 0; i < 19; ++i){
        for (int j = 0; j < 19; ++j){
            B->board[i][j] &= 3;
        }
    }
}

void __attribute__((noinline)) board_clean_flags4(Board *B){
    uint8_t i = 0;
    uint8_t j = 0;
    for (; i < 19; ++i){
        for (j = 0; j < 19; ++j){
            B->board[i][j] &= 3;
        }
    }
}

void __attribute__((noinline)) board_clean_flags5(Board *B){
    int i = 0;
    int j = 0;
    for (; i < 19; ++i){
        for (j = 0; j < 19; ++j){
            B->board[i][j] &= 3;
        }
    }
}

void board_print(Board *B){
    uint8_t i = 0;
    uint8_t j = 0;
    for (; i < 19; ++i){
        for (j = 0; j < 19; ++j){
            printf("%i", B->board[i][j]);
        }
    }
}

int main(){
    Board B1; 
    board_setup_game(&B1);

    board_clean_flags1(&B1); //board_setup_game(&B1); board_print(&B1);
    board_clean_flags2(&B1); //board_setup_game(&B1); board_print(&B1);
    board_clean_flags3(&B1); //board_setup_game(&B1); board_print(&B1);
    board_clean_flags4(&B1); //board_setup_game(&B1); board_print(&B1);
    board_clean_flags5(&B1); //board_setup_game(&B1); board_print(&B1);
    
    /*
    for (int i = 0; i < 10000; ++i){
        board_clean_flags1(&B1); board_print(&B1); board_setup_game(&B1);
        board_clean_flags2(&B1); board_print(&B1); board_setup_game(&B1);
        board_clean_flags3(&B1); board_print(&B1); board_setup_game(&B1);
        board_clean_flags4(&B1); board_print(&B1); board_setup_game(&B1);
        board_clean_flags5(&B1); board_print(&B1); board_setup_game(&B1);
    }*/

    printf("\n");

    return 0;
}
// g++ -O3 -g -c src/main2.cc && g++ main2.o -o Go_test
// rm ./callgrind*
// valgrind --tool=callgrind --dump-instr=yes --collect-jumps=yes ./Go_test
// kcachegrind