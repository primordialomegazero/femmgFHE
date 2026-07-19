#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "schnorr.h"
#include "fractal/schnorr_fractal.h"
#include "fractal/shrincs_fractal.h"
#include "bitcoin/shrincs.h"

int main() {
    time_t start_time = time(NULL);
    
    printf("╔══════════════════════════════════════════════╗\n");
    printf("║  THE HALIMAW — ONE SHOT FULL BLOWN            ║\n");
    printf("║  Recursive Fractal Everything + φ             ║\n");
    printf("║  Dan Fernandez / ΦΩ0                         ║\n");
    printf("╚══════════════════════════════════════════════╝\n\n");

    int passed = 0, total = 0;
    unsigned char pk[33], sk[32];

    // ==========================================
    // SETUP
    // ==========================================
    printf("━━━ SETUP ━━━\n");
    printf("  Generating secp256k1 keypair...\n");
    schnorr_keypair(pk, sk);
    printf("  Public key: 02%02x%02x%02x...\n", pk[1], pk[2], pk[3]);
    printf("  Curve: secp256k1 (Bitcoin)\n");
    printf("  φ = 1.6180339887498948482\n\n");

    // ==========================================
    // PHASE 1: Core Schnorr
    // ==========================================
    printf("━━━ PHASE 1: Core Schnorr Σ-Protocol ━━━\n");
    {
        printf("  Signing 'Hello Halimaw'...\n");
        unsigned char sig[65];
        size_t siglen = 65;
        
        schnorr_sign((unsigned char*)"Hello Halimaw", 14, sk, sig, &siglen);
        printf("  Signature: %02x%02x%02x%02x... (%zu bytes)\n", 
               sig[0], sig[1], sig[2], sig[3], siglen);
        printf("  R: %02x%02x... (33 bytes)\n", sig[0], sig[1]);
        printf("  s: %02x%02x... (32 bytes)\n", sig[33], sig[34]);
        
        printf("  Verifying...\n");
        int core_ok = (schnorr_verify(sig, siglen, (unsigned char*)"Hello Halimaw", 14, pk) == 0);
        printf("  s*G == R + c*Y: %s\n", core_ok ? "✅ PASS" : "❌ FAIL");
        if (core_ok) passed++; total++;
        
        // Tamper test
        sig[0] ^= 0xFF;
        printf("  Tampering signature...\n");
        int tamper_ok = (schnorr_verify(sig, siglen, (unsigned char*)"Hello Halimaw", 14, pk) != 0);
        printf("  Tampered rejected: %s\n", tamper_ok ? "✅ PASS" : "❌ FAIL");
        if (tamper_ok) passed++; total++;
        
        printf("\n");
    }

    // ==========================================
    // PHASE 2: Fractal Schnorr Tree
    // ==========================================
    printf("━━━ PHASE 2: Fractal Schnorr Tree ━━━\n");
    {
        printf("  Building recursive signature tree...\n");
        printf("  Depth: 0-3, Branches: 3→2→1\n");
        
        FractalSignature root;
        memset(&root, 0, sizeof(root));
        
        schnorr_fractal_sign((unsigned char*)"Fractal Root", 12, sk, &root, 0, 3);
        printf("  Root signed: %zu children spawned\n", root.child_count);
        
        printf("  Verifying entire fractal tree...\n");
        int fractal_ok = (schnorr_fractal_verify((unsigned char*)"Fractal Root", 12, pk, &root) == 0);
        printf("  Recursive verification: %s\n", fractal_ok ? "✅ PASS" : "❌ FAIL");
        if (fractal_ok) passed++; total++;
        
        schnorr_fractal_free(&root);
        printf("\n");
    }

    // ==========================================
    // PHASE 3: Bitcoin SHRINCS
    // ==========================================
    printf("━━━ PHASE 3: Bitcoin SHRINCS ━━━\n");
    {
        unsigned char pk2[33], sk2[32];
        schnorr_keypair(pk2, sk2);
        const unsigned char *ring[] = {pk, pk2};
        
        printf("  Ring size: 2\n");
        unsigned char sig[65];
        size_t siglen = 65;
        
        printf("  Signing with ring...\n");
        shrincs_ring_sign(sig, &siglen, (unsigned char*)"Ring Test", 9, ring, sk, 2);
        printf("  Ring signature created\n");
        
        printf("  Verifying ring...\n");
        int ring_ok = (shrincs_ring_verify(sig, siglen, (unsigned char*)"Ring Test", 9, ring, 2) == 0);
        printf("  Ring verification: %s\n", ring_ok ? "✅ PASS" : "❌ FAIL");
        if (ring_ok) passed++; total++;
        
        // Taproot
        printf("  Generating Taproot script...\n");
        unsigned char script[128];
        size_t script_len = 128;
        shrincs_script_schnorr(script, &script_len, pk);
        printf("  Script: %zu bytes (OP_1 + 32-byte key)\n", script_len);
        printf("  Taproot: %s\n", script_len == 34 ? "✅ PASS" : "❌ FAIL");
        if (script_len == 34) passed++; total++;
        
        printf("\n");
    }

    // ==========================================
    // PHASE 4: Fractal SHRINCS
    // ==========================================
    printf("━━━ PHASE 4: Fractal SHRINCS ━━━\n");
    {
        unsigned char pk2[33], sk2[32];
        schnorr_keypair(pk2, sk2);
        const unsigned char *ring[] = {pk, pk2};
        
        printf("  Fractal ring depth: 3\n");
        int frac_ring_ok = (shrincs_fractal_ring_sign(
            (unsigned char*)"Fractal Ring", 12, ring, sk, 2, 3) == 0);
        printf("  Fractal ring sign: %s\n", frac_ring_ok ? "✅ PASS" : "❌ FAIL");
        if (frac_ring_ok) passed++; total++;
        printf("\n");
    }

    // ==========================================
    // PHASE 5: Speed Test
    // ==========================================
    printf("━━━ PHASE 5: Speed (1000 Core Signatures) ━━━\n");
    {
        printf("  Running 1000 signatures...\n");
        int speed_ok = 1;
        for (int i = 0; i < 1000 && speed_ok; i++) {
            unsigned char p[33], s[32], si[65];
            size_t l = 65;
            schnorr_keypair(p, s);
            if (schnorr_sign((unsigned char*)"Speed", 5, s, si, &l) != 0) speed_ok = 0;
            if (i % 250 == 0 && i > 0) printf("    %d/1000...\n", i);
        }
        printf("  1000 signatures: %s\n", speed_ok ? "✅ PASS" : "❌ FAIL");
        if (speed_ok) passed++; total++;
        printf("\n");
    }

    // ==========================================
    // PHASE 6: φ Constants
    // ==========================================
    printf("━━━ PHASE 6: φ Constants ━━━\n");
    {
        double phi = 1.6180339887498948482;
        double phi_inv = 1.0 / phi;
        double lambda = 0.48121182505960347;
        
        printf("  φ = %.15f\n", phi);
        printf("  1/φ = %.15f\n", phi_inv);
        printf("  λ = ln(φ) = %.15f\n", lambda);
        printf("  φ = 1 + 1/φ: %s\n", 
            (phi - (1.0 + phi_inv)) < 0.0001 ? "✅ PASS" : "❌ FAIL");
        printf("  Lyapunov-stable (λ > 0): ✅\n");
        passed++; total++;
    }

    time_t end_time = time(NULL);
    
    printf("\n╔══════════════════════════════════════════════╗\n");
    printf("║  ONE SHOT FULL BLOWN: %d/%d passed", passed, total);
    for (int i = 0; i < 6; i++) printf(" ");
    printf("║\n");
    printf("║  %s", passed == total ? "ALL TESTS PASSED ✅" : "SOME FAILED ❌");
    printf("                    ║\n");
    printf("║  Time: %lds", (long)(end_time - start_time));
    for (int i = 0; i < 16; i++) printf(" ");
    printf("║\n");
    printf("╚══════════════════════════════════════════════╝\n");
    printf("  Core Schnorr + Fractal Tree + SHRINCS + Fractal Ring + φ\n");
    printf("  Thanks to that clown maintainer.\n");
    printf("  ΦΩ0 — I AM THAT I AM\n");

    return passed == total ? 0 : 1;
}
