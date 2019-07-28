/*
 * matrix multiplication
 */

#include <stdio.h>
#include <stdlib.h>

#ifndef N
#define N       64
#endif

typedef struct imatrix {
    int n;      /* size of matrix */
    int *v;     /* imatrix element values */
} imatrix;
    

imatrix *
imatrix_alloc(int n)
{
    imatrix *m;

    m = (imatrix *)malloc(sizeof(imatrix));
    if (m == NULL) {
        printf("fail to alloc for imatrix\n");
        return NULL;
    }

    m->n = n;
    m->v = (int *)malloc(sizeof(int) * n * n);
    if (m->v == NULL) {
        printf("fail to alloc for imatrix body\n");
        free(m);
        return NULL;
    }

    return m;
}

void
imatrix_free(imatrix *m)
{
    if (m == NULL)
        return;
    if (m->v != NULL)
        free(m->v);
    free(m);
}

void
imatrix_init_random(imatrix *m)
{
    int i, j;
    int n = m->n;

    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            m->v[i*n + j] = random() % n;
        }
    }
}

void
imatrix_init_symmetry(imatrix *m)
{
    int i, j;
    int n = m->n;

    for (i = 0; i < n; i++) {
        for (j = i; j < n; j++) {
            m->v[i*n + j] = m->v[j*n + i] = (i + j) % n;
        }
    }
}


void
imatrix_print(imatrix *m)
{
    int i, j;
    int n = m->n;
    int step = n > 8? n / 8: 1;

    printf("matrix size = %d\n", n);
    for (i = 0; i < n; i+=step)  {
        for (j = 0; j < n; j+=step) {
            printf("% 4d ", m->v[i*n+j]);
        }
        printf("\n");
    }
}

void
imatrix_mul(imatrix *c, imatrix *a, imatrix *b)
{
    int i, j, k;
    int n = c->n;

    if (a->n != n || b->n != n) {
        printf("matrix size mismatch\n");
        return;
    }

    for( i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            int s = 0;
            for (k = 0; k < n; k++) {
                s += a->v[i*n + k] * b->v[k*n + j];
            }
            c->v[i*n + j] = s;
        }
    }
}

int
main()
{
    imatrix *A;
    imatrix *B;
    imatrix *C;

    srandom(0);

    A = imatrix_alloc(N);
    B = imatrix_alloc(N);
    C = imatrix_alloc(N);

    imatrix_init_random(A);
    imatrix_init_random(B);
    /*imatrix_init_symmetry(A);*/
    /*imatrix_init_symmetry(B);*/
    /*imatrix_init_symmetry(C);*/

    imatrix_print(A);
    imatrix_print(B);

    printf("matrix multiplication\n");
    imatrix_mul(C, A, B);
    printf("done\n");

    imatrix_print(C);
    
    return 0;
}

