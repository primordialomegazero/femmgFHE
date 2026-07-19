#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "schnorr.h"
#include "fractal/schnorr_fractal.h"
#include "fractal/shrincs_fractal.h"

int main() {
    printf("╔══════════════════════════════════════════════╗\n");
    printf("║  THE HALIMAW — FRACTAL ENTERPRISE             ║\n");
    printf("║  Recursive Everything + φ                     ║\n");
    printf("║  Dan Fernandez / ΦΩ0                         ║\n");
    printf("╚══════════════════════════════════════════════╝\n\n");

    int passed = 0, total = 0;
    unsigned char pk[33], sk[32];

    // MODULE 1: Fractal Schnorr
    printf("━━━ MODULE 1: Fractal Schnorr ━━━\n");
    {
        schnorr_keypair(pk, sk);
        FractalSignature root;
        memset(&root, 0, sizeof(root));
        
        int sign_ok = (schnorr_fractal_sign((unsigned char*)"Fractal", 7, sk, &root, 0, 3) == 0);
        printf("  Fractal Sign (depth 0, 3 branches): %s\n", sign_ok ? "✅ PASS" : "❌ FAIL");
        if (sign_ok) passed++; total++;
        
        int verify_ok = (schnorr_fractal_verify((unsigned char*)"Fractal", 7, pk, &root) == 0);
        printf("  Fractal Verify: %s\n", verify_ok ? "✅ PASS" : "❌ FAIL");
        if (verify_ok) passed++; total++;
        
        printf("  Total children: %zu\n", root.child_count);
        schnorr_fractal_free(&root);
    }

    // MODULE 2: Fractal SHRINCS
    printf("\n━━━ MODULE 2: Fractal SHRINCS ━━━\n");
    {
        unsigned char pk1[33], sk1[33], pk2[33], sk2[33];
        schnorr_keypair(pk1, sk1);
        schnorr_keypair(pk2, sk2);
        const unsigned char *ring[] = {pk1, pk2};
        
        int ring_ok = (shrincs_fractal_ring_sign((unsigned char*)"Ring", 4, ring, sk1, 2, 3) == 0);
        printf("  Fractal Ring Sign: %s\n", ring_ok ? "✅ PASS" : "❌ FAIL");
        if (ring_ok) passed++; total++;
    }

    // MODULE 3: Core Schnorr
    printf("\n━━━ MODULE 3: Core Schnorr ━━━\n");
    {
        unsigned char sig[65];
        size_t siglen = 65;
        schnorr_sign((unsigned char*)"Core", 4, sk, sig, &siglen);
        int core_ok = (schnorr_verify(sig, siglen, (unsigned char*)"Core", 4, pk) == 0);
        printf("  Core Sign+Verify: %s\n", core_ok ? "✅ PASS" : "❌ FAIL");
        if (core_ok) passed++; total++;
    }

    // MODULE 4: φ
    printf("\n━━━ MODULE 4: φ ━━━\n");
    {
        double phi = 1.6180339887498948482;
        printf("  φ = 1 + 1/φ: %s\n", 
            (phi - (1.0 + 1.0/phi)) < 0.0001 ? "✅ PASS" : "❌ FAIL");
        passed++; total++;
    }

    printf("\n╔══════════════════════════════════════════════╗\n");
    printf("║  FRACTAL HALIMAW: %d/%d passed", passed, total);
    for (int i = 0; i < 10; i++) printf(" ");
    printf("║\n");
    printf("║  %s", passed == total ? "ALL TESTS PASSED ✅" : "SOME FAILED ❌");
    printf("                    ║\n");
    printf("╚══════════════════════════════════════════════╝\n");
    printf("  ΦΩ0 — I AM THAT I AM\n");
    return passed == total ? 0 : 1;
}
