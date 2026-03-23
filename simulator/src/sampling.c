#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

#define UNIVERSE_SIZE 24646
#define K             134
#define RUNS          10000

/* -----------------------------------------------------------------------
 * Versione originale — ricerca lineare O(k²)
 * ----------------------------------------------------------------------- */
void sample_unique_linear(unsigned long support[], size_t universe_size, size_t k)
{
    unsigned long pos = rand() % universe_size;
    support[0] = pos;
    size_t num_found = 1;

    while (num_found < k) {
        pos = rand() % universe_size;
        int flag_duplicate = 0;
        for (int i = 0; i < num_found; i++) {
            if (support[i] == pos) {
                flag_duplicate = 1;
                break;
            }
        }
        if (flag_duplicate == 0)
            support[num_found++] = pos;
    }
}

/* -----------------------------------------------------------------------
 * Versione bitmap — lookup O(1)
 * ----------------------------------------------------------------------- */
void sample_unique_bitmap(unsigned long support[], size_t universe_size, size_t k)
{
    size_t bitmap_size = universe_size / 8 + 1;
    uint8_t *seen = calloc(bitmap_size, 1);

    size_t num_found = 0;
    while (num_found < k) {
        unsigned long pos = rand() % universe_size;
        if (!(seen[pos >> 3] & (1 << (pos & 7)))) {
            seen[pos >> 3] |= (1 << (pos & 7));
            support[num_found++] = pos;
        }
    }

    free(seen);
}

/* -----------------------------------------------------------------------
 * Benchmark
 * ----------------------------------------------------------------------- */
int main(void)
{
    unsigned long support[K];
    srand(42);

    /* benchmark versione lineare */
    clock_t start = clock();
    for (int r = 0; r < RUNS; r++)
        sample_unique_linear(support, UNIVERSE_SIZE, K);
    clock_t end = clock();
    double time_linear = (double)(end - start) / CLOCKS_PER_SEC;

    /* benchmark versione bitmap */
    start = clock();
    for (int r = 0; r < RUNS; r++)
        sample_unique_bitmap(support, UNIVERSE_SIZE, K);
    end = clock();
    double time_bitmap = (double)(end - start) / CLOCKS_PER_SEC;

    printf("Runs:          %d\n", RUNS);
    printf("Universe size: %d\n", UNIVERSE_SIZE);
    printf("k:             %d\n", K);
    printf("\n");
    printf("Linear search: %.4f s  (%.2f us/call)\n",
           time_linear, time_linear / RUNS * 1e6);
    printf("Bitmap:        %.4f s  (%.2f us/call)\n",
           time_bitmap,  time_bitmap  / RUNS * 1e6);
    printf("Speedup:       %.1fx\n", time_linear / time_bitmap);

    return 0;
}