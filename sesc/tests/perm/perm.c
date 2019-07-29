/*
 * permutation generation (Heap's algorithm)
 */

#include <stdio.h>
#include <stdlib.h>

#ifndef N
#define N       6
#endif

void
generate(int n, char table[])
{
    if (n == 1) {
        int i;
        for (i = 0; i < N; i++)
            printf("%c ", table[i]);
        printf("\n");
    }
    else {
        int i;
        for (i = 0; i < n; i++) {
            generate(n-1, table);
            if ((n & 1) == 0) {
                /* n is even */
                char c;
                c = table[i];
                table[i] = table[n-1];
                table[n-1] = c;
            }
            else {
                /* n is odd */
                char c;
                c = table[0];
                table[0] = table[n-1];
                table[n-1] = c;
            }
        }
    }
}

int
main()
{
    int i;
    char table[N];

    if (N > 26) {
        printf("length is too long\n");
        return 1;
    }

    for (i = 0; i < N; i++) {
        table[i] = 'A' + i;
    }

    generate(N, table);

    return 0;
}

