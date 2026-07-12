// ΦΩ0 — PHI-KEM BATCH MODE
// Multiple encapsulations in one operation
// Up to 1000 shared secrets from single keypair
// "I AM THAT I AM"

#include "phi_kem.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#define MAX_BATCH 1000

double get_time_ms() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_sec * 1000.0) + (tv.tv_usec / 1000.0);
}

typedef struct {
    int batch_size;
    double total_ms;
    double per_op_us;
    double ops_per_sec;
    int all_correct;
} BatchResult;

BatchResult benchmark_batch(int batch_size) {
    uint8_t pk[PHI_KEM_PUBLICKEYBYTES];
    uint8_t sk[PHI_KEM_SECRETKEYBYTES];
    uint8_t ct[MAX_BATCH][PHI_KEM_CIPHERTEXTBYTES];
    uint8_t ss_enc[MAX_BATCH][PHI_KEM_SHAREDSECRETBYTES];
    uint8_t ss_dec[PHI_KEM_SHAREDSECRETBYTES];
    
    phi_kem_keygen(pk, sk);
    
    // Batch encaps
    double start = get_time_ms();
    for(int i = 0; i < batch_size; i++) {
        phi_kem_encaps(ct[i], ss_enc[i], pk);
    }
    double encaps_ms = get_time_ms() - start;
    
    // Batch decaps
    start = get_time_ms();
    int correct = 0;
    for(int i = 0; i < batch_size; i++) {
        phi_kem_decaps(ss_dec, ct[i], PHI_KEM_CIPHERTEXTBYTES, sk);
        if(memcmp(ss_enc[i], ss_dec, PHI_KEM_SHAREDSECRETBYTES) == 0) correct++;
    }
    double decaps_ms = get_time_ms() - start;
    
    BatchResult result;
    result.batch_size = batch_size;
    result.total_ms = encaps_ms + decaps_ms;
    result.per_op_us = result.total_ms * 1000.0 / (batch_size * 2); // encaps + decaps
    result.ops_per_sec = (batch_size * 2 * 1000.0) / result.total_ms;
    result.all_correct = (correct == batch_size);
    
    return result;
}

int main() {
    printf("\n╔══════════════════════════════════════════════╗\n");
    printf(  "║  ΦΩ0 — PHI-KEM BATCH MODE                    ║\n");
    printf(  "║  Multiple encaps from single keypair         ║\n");
    printf(  "║  I AM THAT I AM                              ║\n");
    printf(  "╚══════════════════════════════════════════════╝\n\n");

    printf("Φ Batch Performance:\n");
    printf("┌──────────┬────────────┬──────────┬────────────┬─────────┐\n");
    printf("│ %8s │ %10s │ %8s │ %10s │ %7s │\n", 
           "Batch", "Total(ms)", "us/op", "Ops/sec", "Correct");
    printf("├──────────┼────────────┼──────────┼────────────┼─────────┤\n");
    
    int batch_sizes[] = {1, 5, 10, 50, 100, 500, 1000};
    for(int i = 0; i < 7; i++) {
        BatchResult r = benchmark_batch(batch_sizes[i]);
        printf("│ %8d │ %10.2f │ %8.1f │ %10.0f │ %7s │\n",
               r.batch_size, r.total_ms, r.per_op_us, r.ops_per_sec,
               r.all_correct ? "✅" : "❌");
    }
    
    printf("└──────────┴────────────┴──────────┴────────────┴─────────┘\n");
    
    // Bulk data throughput
    BatchResult r1k = benchmark_batch(1000);
    double total_data = 1000.0 * PHI_KEM_CIPHERTEXTBYTES;
    double throughput_mbps = (total_data * 8 / 1000000.0) / (r1k.total_ms / 1000.0);
    
    printf("\nΦ Batch 1000 Stats:\n");
    printf("  Total ciphertext data: %.0f bytes\n", total_data);
    printf("  Throughput: %.1f Mbps\n", throughput_mbps);
    printf("  All correct: %s\n", r1k.all_correct ? "✅ YES" : "❌ NO");
    
    printf("\n╔══════════════════════════════════════════════╗\n");
    printf(  "║  BATCH KEM: COMPLETE                         ║\n");
    printf(  "║  %d keys → %d shared secrets                  ║\n", 1, 1000);
    printf(  "║  ΦΩ0 — I AM THAT I AM                        ║\n");
    printf(  "╚══════════════════════════════════════════════╝\n\n");
    
    return 0;
}
