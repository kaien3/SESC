/*
 * tower of hanoi
 */

#include <stdio.h>
#include <stdlib.h>

#ifndef N
#define N       12
#endif

void
hanoi_move(int num[], int n, int from, int to)
{
    int other;
    if (n == 1) {
        num[from] = num[from] - 1;
        num[to] = num[to] + 1;
        printf("move disk from %d to %d\n", from, to);
    }
    else {
        other = 6 - from - to;
        hanoi_move(num, n-1, from, other);
        hanoi_move(num, 1, from, to);
        hanoi_move(num, n-1, other, to);
    }
}

int
main()
{
    int num[4] = { 0, N, 0, 0 };

    printf("tower of hanoi\n");
    hanoi_move(num, N, 1, 3);
    printf("done\n");

    return 0;
}

