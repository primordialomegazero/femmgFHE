#include "phi_algo_merge.h"
#include <stdio.h>
#include <math.h>

int main() {
    printf("Test: Statistical Distribution\n");
    printf("==============================\n\n");
    
    // Generate 1000 nonces, check distribution
    int counts[256] = {0};
    int total = 0;
    
    for(int n=0; n<100; n++) {
        uint8_t nonce[32];
        phi_hybrid_nonce(nonce, 32);
        for(int i=0; i<32; i++) {
            counts[nonce[i]]++;
            total++;
        }
    }
    
    double expected = total / 256.0;
    double chi_square = 0.0;
    int min_count = total, max_count = 0;
    
    for(int i=0; i<256; i++) {
        if(counts[i] < min_count) min_count = counts[i];
        if(counts[i] > max_count) max_count = counts[i];
        double diff = counts[i] - expected;
        chi_square += (diff * diff) / expected;
    }
    
    double avg = (double)total / 256.0;
    printf("Total bytes: %d\n", total);
    printf("Expected per byte: %.2f\n", avg);
    printf("Min count: %d\n", min_count);
    printf("Max count: %d\n", max_count);
    printf("Chi-square: %.2f\n", chi_square);
    printf("Distribution: %s\n\n", chi_square < 300 ? "✅" : "❌");
    
    return 0;
}
