/*
 * quick sort
 */

#include <stdio.h>
#include <stdlib.h>

#ifndef N
#define N       5000
#endif

typedef struct iarray {
    int n;      /* size of array */
    int *v;     /* iarray element values */
} iarray;
    

iarray *
iarray_alloc(int n)
{
    iarray *arry;

    arry = (iarray *)malloc(sizeof(iarray));
    if (arry == NULL) {
        printf("fail to alloc for iarray\n");
        return NULL;
    }

    arry->n = n;
    arry->v = (int *)malloc(sizeof(int) * n);
    if (arry->v == NULL) {
        printf("fail to alloc for iarray body\n");
        free(arry);
        return NULL;
    }

    return arry;
}

void
iarray_free(iarray *arry)
{
    if (arry == NULL)
        return;
    if (arry->v != NULL)
        free(arry->v);
    free(arry);
}

void
iarray_init_random(iarray *arry, int seed)
{
    int i;
    int n = arry->n;
    srandom(seed);
    for (i = 0; i < n; i++) {
        arry->v[i] = random() % n;
    }
}

void
iarray_init_ascend(iarray *arry)
{
    int i;
    int n = arry->n;

    arry->v[0] = 1;
    for (i = 1; i < n; i++) {
        arry->v[i] = arry->v[i-1] + i % 4;
    }
}

void
iarray_init_descend(iarray *arry)
{
    int i;
    int n = arry->n;

    arry->v[n-1] = 1;
    for (i = n-1; i > 0; i--) {
        arry->v[i-1] = arry->v[i] + i % 4;
    }
}

void
iarray_print(iarray *arry)
{
    int i;
    int n = arry->n;
    int step = n > 16? n / 16: 1;

    printf("array size = %d\n", n);
    for (i = 0; i < n; i += step)  {
        printf("% 4d ", arry->v[i]);
    }
    printf("\n");
}

void
quick_sort(int a[], int left, int right)
{
    if (left < right) {
        int v = a[right];
        int i = left-1;
        int j = right;
        int t;

        do {
            do i++; while (a[i] < v);
            do j--; while (a[j] > v);
            t = a[i];
            a[i] = a[j];
            a[j] = t;
        } while (j > i);

        a[j] = a[i];
        a[i] = a[right];
        a[right] = t;
        quick_sort(a, left, i-1);
        quick_sort(a, i+1, right);
    }
}

int
main()
{
    iarray *arry;

    arry = iarray_alloc(N);
    iarray_init_random(arry, 0);
    iarray_print(arry);

    printf("quick sort\n");
    quick_sort(arry->v, 0, arry->n-1);
    printf("done\n");

    iarray_print(arry);
    
    return 0;
}

