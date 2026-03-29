/*
 * Prime Race to 1 Trillion — Segmented Sieve in C
 *
 * Tracks primes ≡ 1 (mod 4) vs ≡ 3 (mod 4) up to N.
 * Looks for the Bays-Hudson crossover at ~609 billion.
 *
 * Compile: cl /O2 prime_race.c /Fe:prime_race.exe
 *     or:  gcc -O3 -o prime_race prime_race.c -lm
 *
 * Usage:  prime_race 1000000000000    (1 trillion)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <stdint.h>

#define SEG_SIZE (1 << 20)  /* 1M per segment */
#define MAX_SMALL_PRIMES 200000

static int small_primes[MAX_SMALL_PRIMES];
static int n_small_primes = 0;

/* Sieve small primes up to limit */
void sieve_small(int limit) {
    char *is_p = (char*)calloc(limit + 1, 1);
    memset(is_p, 1, limit + 1);
    is_p[0] = is_p[1] = 0;
    for (int i = 2; (long long)i * i <= limit; i++) {
        if (is_p[i]) {
            for (int j = i * i; j <= limit; j += i)
                is_p[j] = 0;
        }
    }
    n_small_primes = 0;
    for (int i = 2; i <= limit; i++) {
        if (is_p[i])
            small_primes[n_small_primes++] = i;
    }
    free(is_p);
}

int main(int argc, char **argv) {
    long long N = 100000000000LL; /* default 100 billion */
    if (argc > 1) N = atoll(argv[1]);

    printf("Prime Race to %lld (%0.1f billion)\n", N, N / 1e9);
    printf("================================================================\n\n");

    int sqrt_N = (int)sqrt((double)N) + 2;
    printf("Sieving small primes up to %d...\n", sqrt_N);
    sieve_small(sqrt_N);
    printf("  Found %d small primes\n\n", n_small_primes);

    long long c1 = 0, c3 = 0;
    int leader_inert = -1; /* -1 = unknown */
    long long lead_changes = 0;
    long long max_gap_inert = 0, max_gap_split = 0;

    /* Process small primes first */
    for (int i = 0; i < n_small_primes; i++) {
        int p = small_primes[i];
        if (p == 2) continue;
        if (p % 4 == 1) c1++;
        else c3++;

        long long gap = c3 - c1;
        int now_inert = gap > 0 ? 1 : 0;
        if (gap != 0) {
            if (leader_inert >= 0 && now_inert != leader_inert) {
                lead_changes++;
            }
            leader_inert = now_inert;
        }
        if (gap > max_gap_inert) max_gap_inert = gap;
        if (-gap > max_gap_split) max_gap_split = -gap;
    }

    printf("After small primes: split=%lld, inert=%lld, gap=%+lld, changes=%lld\n\n",
           c1, c3, c3 - c1, lead_changes);

    /* Segmented sieve */
    char *seg = (char*)malloc(SEG_SIZE);
    clock_t t_start = clock();
    long long last_report = 0;
    long long report_interval = 10000000000LL; /* 10 billion */

    long long low = (long long)sqrt_N + 1;
    if (low % 2 == 0) low++;

    /* Pre-compute start positions for each small prime */
    long long *starts = (long long*)malloc(n_small_primes * sizeof(long long));

    printf("Segmented sieve running...\n");
    fflush(stdout);

    while (low <= N) {
        long long high = low + (long long)SEG_SIZE * 2;
        if (high > N + 1) high = N + 1;

        int seg_len = (int)((high - low + 1) / 2);
        memset(seg, 1, seg_len);

        /* Sieve this segment */
        for (int i = 1; i < n_small_primes; i++) { /* skip p=2 */
            long long p = small_primes[i];
            long long start = ((low + p - 1) / p) * p;
            if (start % 2 == 0) start += p;
            if (start < p * p) start = p * p;

            long long idx = (start - low) / 2;
            while (idx < seg_len) {
                seg[idx] = 0;
                idx += p;
            }
        }

        /* Count primes */
        for (int i = 0; i < seg_len; i++) {
            if (seg[i]) {
                long long p = low + (long long)i * 2;
                if (p > N) break;

                if (p % 4 == 1) c1++;
                else c3++;

                long long gap = c3 - c1;
                int now_inert = gap > 0 ? 1 : 0;
                if (gap != 0) {
                    if (leader_inert >= 0 && now_inert != leader_inert) {
                        lead_changes++;
                        leader_inert = now_inert;

                        /* Print lead changes near Bays-Hudson region */
                        if (p > 500000000000LL) {
                            printf("  *** LEAD CHANGE #%lld at p=%lld: "
                                   "split=%lld inert=%lld gap=%+lld %s ***\n",
                                   lead_changes, p, c1, c3, gap,
                                   gap > 0 ? "INERT LEADS" : "SPLIT LEADS");
                            fflush(stdout);
                        }
                    }
                    leader_inert = now_inert;
                }

                if (gap > max_gap_inert) max_gap_inert = gap;
                if (-gap > max_gap_split) max_gap_split = -gap;
            }
        }

        long long processed = high - 1;
        if (processed > N) processed = N;

        /* Progress report */
        if (processed - last_report >= report_interval || processed >= N) {
            double elapsed = (double)(clock() - t_start) / CLOCKS_PER_SEC;
            double rate = processed / elapsed;
            double eta = (N - processed) / rate;
            long long gap = c3 - c1;
            long long total = c1 + c3;
            printf("  %7.1fB / %0.0fB  split=%-13lld inert=%-13lld gap=%+8lld %-5s "
                   "changes=%-6lld %0.0fs  ETA %0.0fs\n",
                   processed / 1e9, N / 1e9, c1, c3, gap,
                   gap > 0 ? "INERT" : "SPLIT",
                   lead_changes, elapsed, eta);
            fflush(stdout);
            last_report = processed;
        }

        low = high;
        if (low % 2 == 0) low++;
    }

    double elapsed_total = (double)(clock() - t_start) / CLOCKS_PER_SEC;

    printf("\n================================================================\n");
    printf("PRIME RACE COMPLETE: N = %lld\n", N);
    printf("================================================================\n\n");
    printf("Total odd primes:    %lld\n", c1 + c3);
    printf("  Split (1 mod 4):   %lld  (%.6f%%)\n", c1, 100.0 * c1 / (c1 + c3));
    printf("  Inert (3 mod 4):   %lld  (%.6f%%)\n", c3, 100.0 * c3 / (c1 + c3));
    printf("  Final gap:         %+lld\n", c3 - c1);
    printf("  Leader at end:     %s\n", c3 > c1 ? "INERT" : "SPLIT");
    printf("\n");
    printf("Lead changes:        %lld\n", lead_changes);
    printf("Max inert advantage: +%lld\n", max_gap_inert);
    printf("Max split advantage: +%lld\n", max_gap_split);
    printf("\n");
    printf("Time: %.1f seconds (%.1f minutes)\n", elapsed_total, elapsed_total / 60.0);

    free(seg);
    free(starts);
    return 0;
}
