#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "../../src/crypto/phi_qr_kem.h"

int main() {
    printf("\n");
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║  A. TRIPLE RASHOMON KEM — COMPLETE UNIT TEST                 ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n\n");
    
    int passed = 0, failed = 0;
    
    // TEST 1: Basic roundtrip
    printf("--- TEST 1: Basic KEM Roundtrip (100 iterations) ---\n");
    {
        int ok = 0;
        for (int i = 0; i < 100; i++) {
            uint8_t pk[32], sk[32], ct[32], ss1[16], ss2[16];
            qr_kem_keygen(pk, sk);
            qr_kem_encaps(ct, ss1, pk);
            int ret = qr_kem_decaps(ss2, ct, sk);
            if (ret == 0 && memcmp(ss1, ss2, 16) == 0) ok++;
        }
        printf("  Result: %d/100\n", ok);
        if (ok == 100) { printf("  [PASS]\n\n"); passed++; }
        else { printf("  [FAIL]\n\n"); failed++; }
    }
    
    // TEST 2: Wrong key rejection
    printf("--- TEST 2: Wrong Key Rejection ---\n");
    {
        uint8_t pk[32], sk[32], sk2[32], ct[32], ss1[16], ss2[16];
        qr_kem_keygen(pk, sk);
        qr_kem_encaps(ct, ss1, pk);
        qr_kem_keygen(pk, sk2);  // Different key
        int ret = qr_kem_decaps(ss2, ct, sk2);
        if (ret == -1) { printf("  [PASS] Wrong key rejected\n\n"); passed++; }
        else { printf("  [FAIL] Wrong key accepted\n\n"); failed++; }
    }
    
    // TEST 3: Tamper detection
    printf("--- TEST 3: Ciphertext Tampering (50 iterations) ---\n");
    {
        int detected = 0;
        for (int i = 0; i < 50; i++) {
            uint8_t pk[32], sk[32], ct[32], ss1[16], ss2[16];
            qr_kem_keygen(pk, sk);
            qr_kem_encaps(ct, ss1, pk);
            ct[20 + (i % 12)] ^= 0x01;  // Tamper binding
            if (qr_kem_decaps(ss2, ct, sk) == -1) detected++;
        }
        printf("  Detected: %d/50\n", detected);
        if (detected == 50) { printf("  [PASS]\n\n"); passed++; }
        else { printf("  [FAIL]\n\n"); failed++; }
    }
    
    // TEST 4: Key uniqueness
    printf("--- TEST 4: Key Pair Uniqueness (50 iterations) ---\n");
    {
        int unique = 0;
        for (int i = 0; i < 50; i++) {
            uint8_t pk1[32], sk1[32], pk2[32], sk2[32];
            qr_kem_keygen(pk1, sk1);
            qr_kem_keygen(pk2, sk2);
            if (memcmp(pk1, pk2, 32) != 0) unique++;
        }
        printf("  Unique: %d/50\n", unique);
        if (unique >= 49) { printf("  [PASS]\n\n"); passed++; }
        else { printf("  [FAIL]\n\n"); failed++; }
    }
    
    // TEST 5: SS uniqueness
    printf("--- TEST 5: Shared Secret Uniqueness (50 iterations) ---\n");
    {
        uint8_t pk[32], sk[32], ct[32], ss[16], prev[16] = {0};
        int unique = 0;
        for (int i = 0; i < 50; i++) {
            qr_kem_keygen(pk, sk);
            qr_kem_encaps(ct, ss, pk);
            if (memcmp(ss, prev, 16) != 0) unique++;
            memcpy(prev, ss, 16);
        }
        printf("  Unique: %d/50\n", unique);
        if (unique >= 49) { printf("  [PASS]\n\n"); passed++; }
        else { printf("  [FAIL]\n\n"); failed++; }
    }
    
    // TEST 6: Fractal compression
    printf("--- TEST 6: Fractal Compression (50 iterations) ---\n");
    {
        int ok = 0;
        for (int i = 0; i < 50; i++) {
            uint8_t f64[64], ct[32], ss1[16], ss2[16];
            qr_kem_fractal_keygen(f64);
            qr_kem_fractal_encaps(f64, ct, ss1);
            int ret = qr_kem_fractal_decaps(f64, ct, ss2);
            if (ret == 0 && memcmp(ss1, ss2, 16) == 0) ok++;
        }
        printf("  Result: %d/50\n", ok);
        if (ok == 50) { printf("  [PASS]\n\n"); passed++; }
        else { printf("  [FAIL]\n\n"); failed++; }
    }
    
    // TEST 7: Binding sensitivity
    printf("--- TEST 7: Binding Sensitivity ---\n");
    {
        uint8_t pk[32], sk[32], ct1[32], ss1[16], ct2[32], ss2[16];
        qr_kem_keygen(pk, sk);
        qr_kem_encaps(ct1, ss1, pk);
        qr_kem_encaps(ct2, ss2, pk);
        int diff = (memcmp(ct1+16, ct2+16, 16) != 0);
        printf("  Different SS → different binding: %s\n", diff ? "YES" : "NO");
        if (diff) { printf("  [PASS]\n\n"); passed++; }
        else { printf("  [FAIL]\n\n"); failed++; }
    }
    
    // TEST 8: Deterministic hash
    printf("--- TEST 8: Hash Determinism ---\n");
    {
        uint8_t h1[32], h2[32];
        phi_hash32((const uint8_t*)"test_12345", 10, h1);
        phi_hash32((const uint8_t*)"test_12345", 10, h2);
        if (memcmp(h1, h2, 32) == 0) { printf("  [PASS]\n\n"); passed++; }
        else { printf("  [FAIL]\n\n"); failed++; }
    }
    
    // SUMMARY
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║  A. TRIPLE RASHOMON KEM — RESULTS                            ║\n");
    printf("╠══════════════════════════════════════════════════════════════╣\n");
    printf("║  Passed: %d/8  |  Failed: %d/8                                ║\n", passed, failed);
    if (failed == 0) printf("║  [PASS] ALL TESTS PASSED — READY FOR B                       ║\n");
    else printf("║  [FAIL] %d test(s) failed                                      ║\n", failed);
    printf("╚══════════════════════════════════════════════════════════════╝\n\n");
    
    return (failed == 0) ? 0 : 1;
}
