#include <stdio.h>
#include <stdlib.h>

int rand_int(int n) {
    /**make sure n is positive*/
    if (n < 0) {
        n = -n;
    }
    /**make sure n is not 0*/
    if (n == 0) {
        return 0;
    }
    /**generate a random number*/
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

int rand_int_n(int n) {
    int i, r;
    int res = 0;
    for (i = 0; get_ones(res)<n; i++) {
        /**make sure that is not already set*/
        do {
            r = rand_int(32);
        } while (r & (1 << i));
        res |= 1 << r;
    }
    return res;
}

