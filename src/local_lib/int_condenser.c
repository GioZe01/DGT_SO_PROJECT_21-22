#include <stdio.h>
#include <stdlib.h>
/**
 * @brief Check if there is a 1 int the given position
 * @param n the number to check
 * @param posistion the position to check
 * @return 1 if there is a 1 in the given position, 0 otherwise
 */
int check_bit(int n, int position){
    return (n >> position) & 1;
}

int rand_int(int n) {
    if (n < 0) {
        n = -n;
    }
    if (n == 0) {
        return 0;
    }
    return rand() % n;
}

int rand_int_range(int min, int max) {
    return rand() % (max - min + 1) + min;
}

int get_ones(int n) {
    int i, res = 0;
    for (i = 0; i < 32; i++) {
        if (n & (1 << i)) {
            res++;
        }
    }
    return res;
}

int rand_int_n(int n, int max) {
    int i, r;
    int res = 0;
    for (i = 0; get_ones(res)<n; i++) {
        do {
            r = rand_int(max);
        } while (r & (1 << i));
        res |= 1 << r;
    }
    return res;
}

int rand_int_n_exclude(int n, int exclude, int max){
    int i, r;
    int res = 0;
    for (i = 0; get_ones(res)<n; i++) {
        do {
            r = rand_int(max);
        } while (r & (1 << i) || r == exclude);
        res |= 1 << r;
    }
    return res;
}
int rand_int_n_pos(int n) {
    int position = rand_int(32);
    while (check_bit(n, position)) {
        position = rand_int(32);
    }
    return position+1;
}
int set_one(int n, int pos){
    return n | (1 << pos);
}
