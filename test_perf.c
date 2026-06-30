#include "phi_algo_merge.h"
#include <stdio.h>
#include <time.h>

int main() {
    printf("Test: Performance Benchmark\n");
    printf("===========================\n\n");
    
    clock_t start, end;
    double cpu_time_used;
    
    // Chaotic Chain
    start = clock();
    for(int i=0; i<10000; i++) {
        uint8_t seed[32];
        uint8_t chain[96];
        phi_chaotic_chain(seed, chain, 6);
    }
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("10,000 chaotic chains: %.3f sec\n", cpu_time_used);
    
    // Hybrid Nonce
    start = clock();
    for(int i=0; i<10000; i++) {
        uint8_t nonce[32];
        phi_hybrid_nonce(nonce, 32);
    }
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("10,000 hybrid nonces: %.3f sec\n", cpu_time_used);
    printf("TPS: %.0f\n\n", 10000 / cpu_time_used);
    
    return 0;
}
