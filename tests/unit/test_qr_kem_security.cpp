#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "../../src/crypto/phi_qr_kem.h"

// ═══════════════════════════════════════════════════════════════
// PHI-QR-KEM — SECURITY AUDIT & UNIT TEST
// ═══════════════════════════════════════════════════════════════

int main() {
    printf("\n");
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║  PHI-QR-KEM — SECURITY AUDIT & UNIT TESTS                    ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n\n");
    
    int passed = 0, failed = 0;
    
    // ═══════════════════════════════════════════════════════════
    // TEST 1: Basic KEM — keygen, encaps, decaps
    // ═══════════════════════════════════════════════════════════
    printf("--- TEST 1: Basic KEM Roundtrip ---\n");
    {
        int ok = 0;
        for (int i = 0; i < 50; i++) {
            uint8_t pk[32], sk[32], ct[32], ss_enc[16], ss_dec[16];
            qr_kem_keygen(pk, sk);
            qr_kem_encaps(ct, ss_enc, pk);
            int ret = qr_kem_decaps(ss_dec, ct, sk);
            if (ret == 0 && memcmp(ss_enc, ss_dec, 16) == 0) ok++;
        }
        printf("  Result: %d/50 passed\n", ok);
        if (ok == 50) { printf("  [PASS]\n\n"); passed++; }
        else { printf("  [FAIL]\n\n"); failed++; }
    }
    
    // ═══════════════════════════════════════════════════════════
    // TEST 2: Wrong key cannot decaps
    // ═══════════════════════════════════════════════════════════
    printf("--- TEST 2: Wrong Key Rejection ---\n");
    {
        uint8_t pk[32], sk[32], sk_wrong[32], ct[32], ss_enc[16], ss_dec[16];
        qr_kem_keygen(pk, sk);
        qr_kem_encaps(ct, ss_enc, pk);
        
        // Generate different key
        uint8_t pk2[32];
        qr_kem_keygen(pk2, sk_wrong);
        
        int ret = qr_kem_decaps(ss_dec, ct, sk_wrong);
        if (ret == -1) { printf("  [PASS] Wrong key rejected\n\n"); passed++; }
        else { printf("  [FAIL] Wrong key accepted!\n\n"); failed++; }
    }
    
    // ═══════════════════════════════════════════════════════════
    // TEST 3: Tampered ciphertext rejected
    // ═══════════════════════════════════════════════════════════
    printf("--- TEST 3: Ciphertext Tampering Detection ---\n");
    {
        int detected = 0;
        for (int i = 0; i < 50; i++) {
            uint8_t pk[32], sk[32], ct[32], ss[16];
            qr_kem_keygen(pk, sk);
            qr_kem_encaps(ct, ss, pk);
            
            // Tamper with a random byte in the binding
            ct[16 + (i % 16)] ^= 0x01;
            
            uint8_t ss_dec[16];
            int ret = qr_kem_decaps(ss_dec, ct, sk);
            if (ret == -1) detected++;
        }
        printf("  Result: %d/50 tampered detected\n", detected);
        if (detected == 50) { printf("  [PASS]\n\n"); passed++; }
        else { printf("  [FAIL]\n\n"); failed++; }
    }
    
    // ═══════════════════════════════════════════════════════════
    // TEST 4: Shared secret randomness (no repeats)
    // ═══════════════════════════════════════════════════════════
    printf("--- TEST 4: Shared Secret Uniqueness ---\n");
    {
        uint8_t pk[32], sk[32], ct[32], ss[16];
        uint8_t prev[16] = {0};
        int unique = 0;
        
        for (int i = 0; i < 50; i++) {
            qr_kem_keygen(pk, sk);
            qr_kem_encaps(ct, ss, pk);
            if (memcmp(ss, prev, 16) != 0) unique++;
            memcpy(prev, ss, 16);
        }
        printf("  Unique SS: %d/50\n", unique);
        if (unique >= 49) { printf("  [PASS]\n\n"); passed++; }
        else { printf("  [FAIL]\n\n"); failed++; }
    }
    
    // ═══════════════════════════════════════════════════════════
    // TEST 5: Key uniqueness
    // ═══════════════════════════════════════════════════════════
    printf("--- TEST 5: Key Pair Uniqueness ---\n");
    {
        uint8_t pk1[32], sk1[32], pk2[32], sk2[32];
        int unique = 0;
        
        for (int i = 0; i < 50; i++) {
            qr_kem_keygen(pk1, sk1);
            qr_kem_keygen(pk2, sk2);
            if (memcmp(pk1, pk2, 32) != 0 && memcmp(sk1, sk2, 32) != 0) unique++;
        }
        printf("  Unique keypairs: %d/50\n", unique);
        if (unique >= 49) { printf("  [PASS]\n\n"); passed++; }
        else { printf("  [FAIL]\n\n"); failed++; }
    }
    
    // ═══════════════════════════════════════════════════════════
    // TEST 6: Binding strength (different SS = different binding)
    // ═══════════════════════════════════════════════════════════
    printf("--- TEST 6: Binding Sensitivity ---\n");
    {
        uint8_t pk[32], sk[32];
        qr_kem_keygen(pk, sk);
        
        uint8_t ct1[32], ss1[16], ct2[32], ss2[16];
        qr_kem_encaps(ct1, ss1, pk);
        qr_kem_encaps(ct2, ss2, pk);
        
        // Binding should be different for different SS
        int bind_diff = (memcmp(ct1+16, ct2+16, 16) != 0);
        printf("  Different SS → different binding: %s\n", bind_diff ? "YES" : "NO");
        if (bind_diff) { printf("  [PASS]\n\n"); passed++; }
        else { printf("  [WARN] Same binding for different SS (collision possible)\n\n"); failed++; }
    }
    
    // ═══════════════════════════════════════════════════════════
    // TEST 7: Fractal compression roundtrip
    // ═══════════════════════════════════════════════════════════
    printf("--- TEST 7: Fractal Compression Roundtrip ---\n");
    {
        int ok = 0;
        for (int i = 0; i < 50; i++) {
            uint8_t fractal[64], ct[32], ss_enc[16], ss_dec[16];
            qr_kem_fractal_keygen(fractal);
            qr_kem_fractal_encaps(fractal, ct, ss_enc);
            int ret = qr_kem_fractal_decaps(fractal, ct, ss_dec);
            if (ret == 0 && memcmp(ss_enc, ss_dec, 16) == 0) ok++;
        }
        printf("  Result: %d/50 passed\n", ok);
        if (ok == 50) { printf("  [PASS]\n\n"); passed++; }
        else { printf("  [FAIL]\n\n"); failed++; }
    }
    
    // ═══════════════════════════════════════════════════════════
    // TEST 8: Deterministic hash
    // ═══════════════════════════════════════════════════════════
    printf("--- TEST 8: Hash Determinism ---\n");
    {
        uint8_t h1[32], h2[32];
        phi_hash32((const uint8_t*)"test_input_12345", 16, h1);
        phi_hash32((const uint8_t*)"test_input_12345", 16, h2);
        int match = (memcmp(h1, h2, 32) == 0);
        printf("  Same input → same hash: %s\n", match ? "YES" : "NO");
        if (match) { printf("  [PASS]\n\n"); passed++; }
        else { printf("  [FAIL]\n\n"); failed++; }
    }
    
    // ═══════════════════════════════════════════════════════════
    // TEST 9: Avalanche effect
    // ═══════════════════════════════════════════════════════════
    printf("--- TEST 9: Hash Avalanche ---\n");
    {
        uint8_t in1[17] = "test_input_00000";
        uint8_t in2[17] = "test_input_00001";
        uint8_t h1[32], h2[32];
        phi_hash32(in1, 16, h1);
        phi_hash32(in2, 16, h2);
        
        int diff_bits = 0;
        for (int i = 0; i < 32; i++) {
            uint8_t x = h1[i] ^ h2[i];
            for (int j = 0; j < 8; j++) if (x & (1 << j)) diff_bits++;
        }
        printf("  Bits changed (1-bit input diff): %d/256\n", diff_bits);
        if (diff_bits > 80) { printf("  [PASS] Good avalanche\n\n"); passed++; }
        else { printf("  [WARN] Weak avalanche\n\n"); failed++; }
    }
    
    // ═══════════════════════════════════════════════════════════
    // SUMMARY
    // ═══════════════════════════════════════════════════════════
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║  SECURITY AUDIT SUMMARY                                      ║\n");
    printf("╠══════════════════════════════════════════════════════════════╣\n");
    printf("║  Tests passed: %2d/9                                          ║\n", passed);
    printf("║  Tests failed: %2d/9                                          ║\n", failed);
    
    if (failed == 0) {
        printf("║                                                              ║\n");
        printf("║  [PASS] PHI-QR-KEM — ALL SECURITY TESTS PASSED               ║\n");
    }
    printf("╚══════════════════════════════════════════════════════════════╝\n\n");
    
    return (failed == 0) ? 0 : 1;
}
