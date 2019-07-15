/*
 * prime number (sieve of Eratosthenes)
 */

#include <stdio.h>
#include <stdlib.h>

#ifndef N
#define N       10000
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
iarray_clear(iarray *arry, int init_val)
{
    int i;
    int n = arry->n;

    for (i = 0; i < n; i++) {
        arry->v[i] = init_val;
    }
}

void
iarray_print(iarray *arry)
{
    int i;
    int n = arry->n;
    int step = n > 8? n / 8: 1;

    printf("array size = %d\n", n);
    for (i = 0; i < n; i += step)  {
        printf("% 4d ", arry->v[i]);
    }
    printf("\n");
}

void
prime_print(iarray *primes)
{
    int i;
    int count = 0;
    int last_prime = 0;
    for (i = 0; i < primes->n; i++) {
        if (primes->v[i]) {
            printf("%d ", i);
            count++;
            last_prime = i;
        }
    }

    printf("\n");
    printf("%d prime numbers found, last prime = %d\n",
           count, last_prime);
}

void
sieve(iarray *primes)
{
    int i;
    int n = primes->n;

    primes->v[0] = primes->v[1] = 0;    /* 0 and 1 are not prime nummber */

    for (i = 0; i < n; i++) {
        if (primes->v[i]) {
            int prime = i;
            int k;
            for (k = i + prime; k < n; k += prime) {
                primes->v[k] = 0;
            }
        }
    }
}

int
main()
{
    iarray *primes;

    primes = iarray_alloc(N);
    iarray_clear(primes, 1);

    printf("sieve of Eratosthenes\n");
    sieve(primes);
    printf("done\n");

    prime_print(primes);

/*    iarray_print(arry);*/
    
    return 0;
}

