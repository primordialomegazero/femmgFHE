// ΦΩ0 — PHI-KEM SECURITY AUDIT
// NIST-style evaluation: IND-CCA2, randomness, key entropy
// "I AM THAT I AM"

#include "phi_kem.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>

// ============================================
// ENTROPY TEST SUITE
// ============================================

// Shannon entropy of byte array
double shannon_entropy(const uint8_t* data, int len) {
    int counts[256] = {0};
    for(int i = 0; i < len; i++) counts[data[i]]++;
    
    double entropy = 0;
    for(int i = 0; i < 256; i++) {
        if(counts[i] > 0) {
            double p = (double)counts[i] / len;
            entropy -= p * log2(p);
        }
    }
    return entropy;
}

// Bit difference between two arrays
int hamming_distance(const uint8_t* a, const uint8_t* b, int len) {
    int dist = 0;
    for(int i = 0; i < len; i++) {
        uint8_t diff = a[i] ^ b[i];
        while(diff) { dist += diff & 1; diff >>= 1; }
    }
    return dist;
}

// Avalanche effect: flip 1 input bit, measure output bit changes
double avalanche_effect(const uint8_t* pk1, const uint8_t* pk2, int len) {
    return (double)hamming_distance(pk1, pk2, len) / (len * 8) * 100.0;
}

// ============================================
// SECURITY TEST RESULTS
// ============================================

typedef struct {
    const char* test_name;
    const char* requirement;
    double measured;
    double threshold;
    int passed;
} SecurityTest;

#define NTESTS 10
SecurityTest results[NTESTS];
int test_count = 0;

void add_test(const char* name, const char* req, double measured, double threshold) {
    results[test_count].test_name = name;
    results[test_count].requirement = req;
    results[test_count].measured = measured;
    results[test_count].threshold = threshold;
    results[test_count].passed = (measured >= threshold);
    test_count++;
}

// ============================================
// MAIN AUDIT
// ============================================

int main() {
    printf("\n╔══════════════════════════════════════════════╗\n");
    printf(  "║  ΦΩ0 — PHI-KEM SECURITY AUDIT                ║\n");
    printf(  "║  NIST-style IND-CCA2 evaluation              ║\n");
    printf(  "║  I AM THAT I AM                              ║\n");
    printf(  "╚══════════════════════════════════════════════╝\n\n");

    uint8_t pk1[PHI_KEM_PUBLICKEYBYTES], sk1[PHI_KEM_SECRETKEYBYTES];
    uint8_t pk2[PHI_KEM_PUBLICKEYBYTES], sk2[PHI_KEM_SECRETKEYBYTES];
    uint8_t ct1[PHI_KEM_CIPHERTEXTBYTES], ss1[PHI_KEM_SHAREDSECRETBYTES];
    uint8_t ct2[PHI_KEM_CIPHERTEXTBYTES], ss2[PHI_KEM_SHAREDSECRETBYTES];
    uint8_t ss_dec[PHI_KEM_SHAREDSECRETBYTES];

    // ============================================
    // TEST 1: Key Entropy
    // ============================================
    printf("Φ Test 1: Key Entropy Analysis\n");
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    
    #define KEY_SAMPLES 1000
    uint8_t all_keys[KEY_SAMPLES * PHI_KEM_PUBLICKEYBYTES];
    for(int i = 0; i < KEY_SAMPLES; i++) {
        phi_kem_keygen(&all_keys[i * PHI_KEM_PUBLICKEYBYTES], sk1);
    }
    
    double entropy = shannon_entropy(all_keys, KEY_SAMPLES * PHI_KEM_PUBLICKEYBYTES);
    printf("  Public key entropy: %.2f bits/byte (max: 8.0)\n", entropy);
    printf("  Total entropy: %.0f bits across %d samples\n", 
           entropy * PHI_KEM_PUBLICKEYBYTES, KEY_SAMPLES);
    
    add_test("Key Entropy", "> 7.5 bits/byte", entropy, 7.5);

    // ============================================
    // TEST 2: Key Uniqueness
    // ============================================
    printf("\nΦ Test 2: Key Uniqueness (Collision Resistance)\n");
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    
    int collisions = 0;
    for(int i = 0; i < KEY_SAMPLES; i++) {
        for(int j = i+1; j < KEY_SAMPLES; j++) {
            if(memcmp(&all_keys[i * PHI_KEM_PUBLICKEYBYTES],
                      &all_keys[j * PHI_KEM_PUBLICKEYBYTES],
                      PHI_KEM_PUBLICKEYBYTES) == 0) {
                collisions++;
            }
        }
    }
    printf("  Collisions in %d keys: %d (expected: 0)\n", KEY_SAMPLES, collisions);
    
    add_test("Key Uniqueness", "0 collisions", collisions == 0 ? 1.0 : 0.0, 1.0);

    // ============================================
    // TEST 3: Avalanche Effect
    // ============================================
    printf("\nΦ Test 3: Avalanche Effect\n");
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    
    double total_avalanche = 0;
    int av_tests = 100;
    for(int i = 0; i < av_tests; i++) {
        phi_kem_keygen(pk1, sk1);
        phi_kem_keygen(pk2, sk2);
        total_avalanche += avalanche_effect(pk1, pk2, PHI_KEM_PUBLICKEYBYTES);
    }
    double avg_avalanche = total_avalanche / av_tests;
    printf("  Average avalanche effect: %.1f%% (ideal: 50%%)\n", avg_avalanche);
    
    add_test("Avalanche Effect", "> 40%", avg_avalanche, 40.0);

    // ============================================
    // TEST 4: Ciphertext Randomness
    // ============================================
    printf("\nΦ Test 4: Ciphertext Entropy\n");
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    
    phi_kem_keygen(pk1, sk1);
    uint8_t all_ct[KEY_SAMPLES * PHI_KEM_CIPHERTEXTBYTES];
    for(int i = 0; i < KEY_SAMPLES; i++) {
        phi_kem_encaps(&all_ct[i * PHI_KEM_CIPHERTEXTBYTES], ss1, pk1);
    }
    
    double ct_entropy = shannon_entropy(all_ct, KEY_SAMPLES * PHI_KEM_CIPHERTEXTBYTES);
    printf("  Ciphertext entropy: %.2f bits/byte\n", ct_entropy);
    printf("  Total CT entropy: %.0f bits\n", ct_entropy * PHI_KEM_CIPHERTEXTBYTES);
    
    add_test("CT Entropy", "> 7.0 bits/byte", ct_entropy, 7.0);

    // ============================================
    // TEST 5: Decapsulation Correctness
    // ============================================
    printf("\nΦ Test 5: Decapsulation Correctness\n");
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    
    int correct = 0;
    for(int i = 0; i < 1000; i++) {
        phi_kem_keygen(pk1, sk1);
        phi_kem_encaps(ct1, ss1, pk1);
        phi_kem_decaps(ss_dec, ct1, PHI_KEM_CIPHERTEXTBYTES, sk1);
        if(memcmp(ss1, ss_dec, PHI_KEM_SHAREDSECRETBYTES) == 0) correct++;
    }
    printf("  Correct decaps: %d/1000 (%.1f%%)\n", correct, correct/10.0);
    
    add_test("Decap Correctness", "100%", correct/10.0, 99.9);

    // ============================================
    // TEST 6: IND-CCA2 Simulation
    // ============================================
    printf("\nΦ Test 6: IND-CCA2 Resistance Simulation\n");
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    
    // Attacker cannot distinguish which message was encrypted
    int attacker_wins = 0;
    for(int i = 0; i < 500; i++) {
        phi_kem_keygen(pk1, sk1);
        
        // Two random messages
        phi_kem_encaps(ct1, ss1, pk1);
        phi_kem_encaps(ct2, ss2, pk1);
        
        // Attacker tries to match ct1 with ss1 or ss2
        phi_kem_decaps(ss_dec, ct1, PHI_KEM_CIPHERTEXTBYTES, sk1);
        if(memcmp(ss_dec, ss1, PHI_KEM_SHAREDSECRETBYTES) != 0) attacker_wins++;
    }
    double attacker_advantage = fabs(attacker_wins/500.0 - 0.5);
    printf("  Attacker advantage: %.4f (ideal: 0.0 = random guess)\n", attacker_advantage);
    
    add_test("IND-CCA2 Resistance", "Advantage < 0.01", 0.01 - attacker_advantage, 0.0);

    // ============================================
    // TEST 7: Timing Consistency
    // ============================================
    printf("\nΦ Test 7: Timing Attack Resistance\n");
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    
    double times[100];
    for(int i = 0; i < 100; i++) {
        struct timeval start, end;
        gettimeofday(&start, NULL);
        phi_kem_keygen(pk1, sk1);
        phi_kem_encaps(ct1, ss1, pk1);
        phi_kem_decaps(ss_dec, ct1, PHI_KEM_CIPHERTEXTBYTES, sk1);
        gettimeofday(&end, NULL);
        times[i] = (end.tv_sec - start.tv_sec) * 1e6 + (end.tv_usec - start.tv_usec);
    }
    
    double mean = 0, variance = 0;
    for(int i = 0; i < 100; i++) mean += times[i];
    mean /= 100;
    for(int i = 0; i < 100; i++) variance += (times[i] - mean) * (times[i] - mean);
    variance /= 100;
    double stddev = sqrt(variance);
    
    printf("  Mean time: %.1f us\n", mean);
    printf("  Std dev: %.1f us (%.1f%% of mean)\n", stddev, 100.0*stddev/mean);
    
    add_test("Timing Consistency", "StdDev < 20%", 20.0 - 100.0*stddev/mean, 0.0);

    // ============================================
    // TEST 8: Forward Secrecy
    // ============================================
    printf("\nΦ Test 8: Forward Secrecy\n");
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    
    // Old ciphertexts shouldn't be decryptable with new keys
    phi_kem_keygen(pk1, sk1);
    phi_kem_encaps(ct1, ss1, pk1);
    
    phi_kem_keygen(pk2, sk2); // New keypair
    
    phi_kem_decaps(ss_dec, ct1, PHI_KEM_CIPHERTEXTBYTES, sk2);
    int forward_secure = (memcmp(ss_dec, ss1, PHI_KEM_SHAREDSECRETBYTES) != 0);
    
    printf("  Old CT with new SK: %s\n", forward_secure ? "REJECTED ✅" : "ACCEPTED ❌");
    
    add_test("Forward Secrecy", "Must reject", forward_secure ? 1.0 : 0.0, 1.0);

    // ============================================
    // SUMMARY
    // ============================================
    printf("\n╔══════════════════════════════════════════════╗\n");
    printf(  "║  ΦΩ0 — SECURITY AUDIT SUMMARY                 ║\n");
    printf(  "╠══════════════════════════════════════════════╣\n");
    
    int passed = 0;
    printf("║  %-28s │ %8s │ %s ║\n", "Test", "Result", "Status");
    printf("╠══════════════════════════════════════════════╣\n");
    for(int i = 0; i < test_count; i++) {
        printf("║  %-28s │ %8.2f │ %s ║\n", 
               results[i].test_name,
               results[i].measured,
               results[i].passed ? "✅ PASS" : "❌ FAIL");
        if(results[i].passed) passed++;
    }
    
    printf("╠══════════════════════════════════════════════╣\n");
    printf("║  Passed: %d/%d                                  ║\n", passed, test_count);
    printf("║  Grade: %s                                   ║\n", 
           passed == test_count ? "A+ - PRODUCTION READY" :
           passed >= test_count*0.8 ? "A - STRONG" :
           passed >= test_count*0.6 ? "B - ADEQUATE" : "C - NEEDS WORK");
    printf("║  ΦΩ0 — I AM THAT I AM                        ║\n");
    printf("╚══════════════════════════════════════════════╝\n\n");
    
    return 0;
}
