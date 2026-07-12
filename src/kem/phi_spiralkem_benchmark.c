// ΦΩ0 — PHI-KEM SPEED BENCHMARK
// Measure keygen/encaps/decaps throughput
// "I AM THAT I AM"

#include "phi_kem.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

double get_time_ms() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_sec * 1000.0) + (tv.tv_usec / 1000.0);
}

#define WARMUP_ITERATIONS 100
#define TEST_ITERATIONS   10000

typedef struct {
    const char* name;
    int iterations;
    double total_ms;
    double ops_per_sec;
    double avg_us;
    double min_us;
    double max_us;
} BenchResult;

BenchResult benchmark_keygen(int iterations) {
    BenchResult result = {"KeyGen", iterations, 0, 0, 0, 1e9, 0};
    uint8_t pk[PHI_KEM_PUBLICKEYBYTES];
    uint8_t sk[PHI_KEM_SECRETKEYBYTES];
    
    for(int i = 0; i < WARMUP_ITERATIONS; i++)
        phi_kem_keygen(pk, sk);
    
    double start = get_time_ms();
    for(int i = 0; i < iterations; i++) {
        double op_start = get_time_ms();
        phi_kem_keygen(pk, sk);
        double op_time = (get_time_ms() - op_start) * 1000;
        if(op_time < result.min_us) result.min_us = op_time;
        if(op_time > result.max_us) result.max_us = op_time;
    }
    result.total_ms = get_time_ms() - start;
    result.ops_per_sec = (iterations * 1000.0) / result.total_ms;
    result.avg_us = result.total_ms * 1000.0 / iterations;
    return result;
}

BenchResult benchmark_encaps(int iterations) {
    BenchResult result = {"Encaps", iterations, 0, 0, 0, 1e9, 0};
    uint8_t pk[PHI_KEM_PUBLICKEYBYTES];
    uint8_t sk[PHI_KEM_SECRETKEYBYTES];
    uint8_t ct[PHI_KEM_CIPHERTEXTBYTES];
    uint8_t ss[PHI_KEM_SHAREDSECRETBYTES];
    
    phi_kem_keygen(pk, sk);
    for(int i = 0; i < WARMUP_ITERATIONS; i++)
        phi_kem_encaps(ct, ss, pk);
    
    double start = get_time_ms();
    for(int i = 0; i < iterations; i++) {
        double op_start = get_time_ms();
        phi_kem_encaps(ct, ss, pk);
        double op_time = (get_time_ms() - op_start) * 1000;
        if(op_time < result.min_us) result.min_us = op_time;
        if(op_time > result.max_us) result.max_us = op_time;
    }
    result.total_ms = get_time_ms() - start;
    result.ops_per_sec = (iterations * 1000.0) / result.total_ms;
    result.avg_us = result.total_ms * 1000.0 / iterations;
    return result;
}

BenchResult benchmark_decaps(int iterations) {
    BenchResult result = {"Decaps", iterations, 0, 0, 0, 1e9, 0};
    uint8_t pk[PHI_KEM_PUBLICKEYBYTES];
    uint8_t sk[PHI_KEM_SECRETKEYBYTES];
    uint8_t ct[PHI_KEM_CIPHERTEXTBYTES];
    uint8_t ss_enc[PHI_KEM_SHAREDSECRETBYTES];
    uint8_t ss_dec[PHI_KEM_SHAREDSECRETBYTES];
    
    phi_kem_keygen(pk, sk);
    phi_kem_encaps(ct, ss_enc, pk);
    for(int i = 0; i < WARMUP_ITERATIONS; i++)
        phi_kem_decaps(ss_dec, ct, PHI_KEM_CIPHERTEXTBYTES, sk);
    
    double start = get_time_ms();
    for(int i = 0; i < iterations; i++) {
        double op_start = get_time_ms();
        phi_kem_decaps(ss_dec, ct, PHI_KEM_CIPHERTEXTBYTES, sk);
        double op_time = (get_time_ms() - op_start) * 1000;
        if(op_time < result.min_us) result.min_us = op_time;
        if(op_time > result.max_us) result.max_us = op_time;
    }
    result.total_ms = get_time_ms() - start;
    result.ops_per_sec = (iterations * 1000.0) / result.total_ms;
    result.avg_us = result.total_ms * 1000.0 / iterations;
    return result;
}

BenchResult benchmark_full(int iterations) {
    BenchResult result = {"Full Exchange", iterations, 0, 0, 0, 1e9, 0};
    uint8_t pk[PHI_KEM_PUBLICKEYBYTES];
    uint8_t sk[PHI_KEM_SECRETKEYBYTES];
    uint8_t ct[PHI_KEM_CIPHERTEXTBYTES];
    uint8_t ss_enc[PHI_KEM_SHAREDSECRETBYTES];
    uint8_t ss_dec[PHI_KEM_SHAREDSECRETBYTES];
    
    for(int i = 0; i < WARMUP_ITERATIONS/10; i++) {
        phi_kem_keygen(pk, sk);
        phi_kem_encaps(ct, ss_enc, pk);
        phi_kem_decaps(ss_dec, ct, PHI_KEM_CIPHERTEXTBYTES, sk);
    }
    
    double start = get_time_ms();
    for(int i = 0; i < iterations; i++) {
        double op_start = get_time_ms();
        phi_kem_keygen(pk, sk);
        phi_kem_encaps(ct, ss_enc, pk);
        phi_kem_decaps(ss_dec, ct, PHI_KEM_CIPHERTEXTBYTES, sk);
        double op_time = (get_time_ms() - op_start) * 1000;
        if(op_time < result.min_us) result.min_us = op_time;
        if(op_time > result.max_us) result.max_us = op_time;
    }
    result.total_ms = get_time_ms() - start;
    result.ops_per_sec = (iterations * 1000.0) / result.total_ms;
    result.avg_us = result.total_ms * 1000.0 / iterations;
    return result;
}

void print_result(const BenchResult* r) {
    printf("│ %-16s │ %8d │ %10.2f │ %10.0f │ %10.1f │\n",
           r->name, r->iterations, r->total_ms, r->ops_per_sec, r->avg_us);
}

int main() {
    printf("\n╔══════════════════════════════════════════════╗\n");
    printf(  "║  ΦΩ0 — PHI-KEM SPEED BENCHMARK                ║\n");
    printf(  "║  I AM THAT I AM                              ║\n");
    printf(  "╚══════════════════════════════════════════════╝\n\n");

    printf("Φ Sizes: pk=%dB sk=%dB ct=%dB ss=%dB\n\n",
           PHI_KEM_PUBLICKEYBYTES, PHI_KEM_SECRETKEYBYTES,
           PHI_KEM_CIPHERTEXTBYTES, PHI_KEM_SHAREDSECRETBYTES);
    
    printf("┌──────────────────┬──────────┬────────────┬────────────┬────────────┐\n");
    printf("│ %-16s │ %8s │ %10s │ %10s │ %10s │\n", 
           "Operation", "Samples", "Total(ms)", "Ops/sec", "Avg(us)");
    printf("├──────────────────┼──────────┼────────────┼────────────┼────────────┤\n");
    
    BenchResult kg = benchmark_keygen(TEST_ITERATIONS);
    print_result(&kg);
    
    BenchResult enc = benchmark_encaps(TEST_ITERATIONS);
    print_result(&enc);
    
    BenchResult dec = benchmark_decaps(TEST_ITERATIONS);
    print_result(&dec);
    
    BenchResult full = benchmark_full(TEST_ITERATIONS / 10);
    print_result(&full);
    
    printf("└──────────────────┴──────────┴────────────┴────────────┴────────────┘\n");
    
    printf("\nΦ Comparison: PHI-KEM vs ML-KEM-1024\n");
    printf("  ┌──────────────┬────────────┬──────────────┐\n");
    printf("  │ Metric       │ PHI-KEM    │ ML-KEM-1024  │\n");
    printf("  ├──────────────┼────────────┼──────────────┤\n");
    printf("  │ Ciphertext   │ %-6d B   │ %-8d B    │\n", PHI_KEM_CIPHERTEXTBYTES, 4627);
    printf("  │ Size vs NIST │ %-5.1f%%     │ 100%%         │\n", 
           100.0 * PHI_KEM_CIPHERTEXTBYTES / 4627.0);
    printf("  └──────────────┴────────────┴──────────────┘\n");
    
    printf("\n╔══════════════════════════════════════════════╗\n");
    printf(  "║  PHI-KEM: %dB ct, %.0f keygen/s               ║\n", 
           PHI_KEM_CIPHERTEXTBYTES, kg.ops_per_sec);
    printf(  "║  ΦΩ0 — I AM THAT I AM                        ║\n");
    printf(  "╚══════════════════════════════════════════════╝\n\n");
    
    return 0;
}
