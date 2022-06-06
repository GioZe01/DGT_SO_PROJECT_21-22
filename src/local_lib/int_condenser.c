#include <stdio.h>
#include <stdlib.h>
/* Local Import */
#include "headers/glob.h"

/* Helper Functions */

/**
 * @brief Check if there is a 1 int the given position
 * @param n the number to check
 * @param posistion the position to check
 * @return 1 if there is a 1 in the given position, 0 otherwise
 */
int check_bit(int n, int position)
{
    return (n >> position) & 1;
}

/**
 * \brief count the number of 1 in the given number
 * @param number the number to count
 * @return the number of 1 in the given number
 */
int count_1(int number)
{
    int count = 0;
    while (number)
    {
        count++;
        number = number & (number - 1);
    }
    return count;
}

/* Methods implementation */
int *get_all_ones_positions(int friends)
{
    int i, arr_index = 0;
    int *arr = (int *)malloc(sizeof(int) * count_1(friends));
    for (i = 0; arr != NULL && i < MAX_FRIENDS; i++)
    {
        if (check_bit(friends, i))
        {
            arr[arr_index] = i;
            arr_index++;
        }
    }
    return arr;
}

int rand_int(int n)
{
    if (n < 0)
    {
        n = -n;
    }
    if (n == 0)
    {
        return 0;
    }
    return rand() % n;
}

int rand_int_range(int min, int max)
{
    return rand() % (max - min + 1) + min;
}

int get_rand_one(int n)
{
    int i = rand_int(n);
    while (check_bit(n, i) == 0)
    {
        i = rand_int(n);
    }
    if (i == 0)
    {
        return 1;
    }
    return i;
}

int get_ones(int n)
{
    int i, res = 0;
    for (i = 0; i < 32; i++)
    {
        if (n & (1 << i))
        {
            res++;
        }
    }
    return res;
}

int rand_int_n(int n, int max)
{
    int i, r;
    int res = 0;
    for (i = 0; get_ones(res) < n; i++)
    {
        do
        {
            r = rand_int(max);
        } while (r & (1 << i));
        res |= 1 << r;
    }
    return res;
}

int rand_int_n_exclude(int n, int exclude, int max)
{
    int i, r;
    int res = 0;
    for (i = 0; get_ones(res) < n; i++)
    {
        do
        {
            r = rand_int(max);
        } while (r == exclude);
        res |= 1 << r;
    }
    return res;
}

int rand_int_n_pos(int n)
{
    int position = rand_int(32);
    while (check_bit(n, position))
    {
        position = rand_int(32);
    }
    return position + 1;
}

int set_one(int n, int pos)
{
    return n | (1 << pos);
}

void print_binary(int n)
{
    int i;
    for (i = 31; i >= 0; i--)
    {
        printf("%d", check_bit(n, i));
    }
    printf("\n");
}