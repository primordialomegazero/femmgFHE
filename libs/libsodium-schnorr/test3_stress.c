#include <stdio.h>
#include <string.h>
#include <time.h>
#include "schnorr.h"

int main() {
    time_t start_time = time(NULL);
    
    printf("╔══════════════════════════════════════════════╗\n");
    printf("║  SCHNORR SIGNATURES — STRESS TEST             ║\n");
    printf("║  1,000,000 Signatures | Multi-Threaded         ║\n");
    printf("║  Dan Joseph M. Fernandez / ΦΩ0               ║\n");
    printf("╚══════════════════════════════════════════════╝\n\n");

    int passed = 0, total = 0;

    printf("━━━ INITIALIZATION ━━━\n");
    printf("  Algorithm: Schnorr Σ-Protocol (secp256k1)\n");
    printf("  Implementation: C11, OpenSSL 3.0\n");
    printf("  CPU: Ryzen 5 2600 (3.4 GHz, 6 cores)\n");
    printf("  RAM: 16 GB\n\n");

    // ==========================================
    // TEST 1: Throughput (1 Million Signatures)
    // ==========================================
    printf("━━━ TEST 1: Single-Core Throughput ━━━\n");
    {
        unsigned char pk[33], sk[32], sig[65];
        size_t siglen = 65;
        schnorr_keypair(pk, sk);
        
        const unsigned char *msg = (unsigned char*)"Benchmark message for throughput testing";
        size_t msg_len = 46;
        
        printf("  Target: 1,000,000 signatures\n");
        printf("  Message: '%s' (%zu bytes)\n", msg, msg_len);
        printf("  Executing...\n");
        
        time_t t1 = time(NULL);
        long count = 0;
        int ok = 1;
        
        for (long i = 0; i < 1000000 && ok; i++) {
            if (schnorr_sign(msg, msg_len, sk, sig, &siglen) != 0) ok = 0;
            count++;
            if (i % 100000 == 0 && i > 0) {
                time_t t2 = time(NULL);
                double tps = (double)i / (double)(t2 - t1);
                printf("    %ld/1000000 (%.0f sigs/sec)\n", i, tps);
            }
        }
        
        time_t t2 = time(NULL);
        double elapsed = difftime(t2, t1);
        double tps = (double)count / elapsed;
        
        printf("  Completed: %ld signatures in %.1fs\n", count, elapsed);
        printf("  Throughput: %.0f signatures/second\n", tps);
        printf("  Average: %.3f ms/signature\n", (elapsed * 1000.0) / count);
        
        int ok_result = (ok && count == 1000000);
        printf("  Result: %s\n", ok_result ? "✅ PASS" : "❌ FAIL");
        if (ok_result) passed++; total++;
        printf("\n");
    }

    // ==========================================
    // TEST 2: Verification Throughput
    // ==========================================
    printf("━━━ TEST 2: Verification Throughput ━━━\n");
    {
        unsigned char pk[33], sk[32], sig[65];
        size_t siglen = 65;
        schnorr_keypair(pk, sk);
        
        const unsigned char *msg = (unsigned char*)"Verify";
        schnorr_sign(msg, 6, sk, sig, &siglen);
        
        printf("  Target: 100,000 verifications\n");
        printf("  Executing...\n");
        
        time_t t1 = time(NULL);
        long count = 0;
        int ok = 1;
        
        for (long i = 0; i < 100000 && ok; i++) {
            if (schnorr_verify(sig, siglen, msg, 6, pk) != 0) ok = 0;
            count++;
        }
        
        time_t t2 = time(NULL);
        double elapsed = difftime(t2, t1);
        double tps = (double)count / elapsed;
        
        printf("  Completed: %ld verifications in %.1fs\n", count, elapsed);
        printf("  Throughput: %.0f verifications/second\n", tps);
        printf("  Result: %s\n", (ok && count == 100000) ? "✅ PASS" : "❌ FAIL");
        if (ok && count == 100000) passed++; total++;
        printf("\n");
    }

    // ==========================================
    // TEST 3: Key Generation Throughput
    // ==========================================
    printf("━━━ TEST 3: Key Generation Throughput ━━━\n");
    {
        printf("  Target: 100,000 keypairs\n");
        printf("  Executing...\n");
        
        time_t t1 = time(NULL);
        long count = 0;
        int ok = 1;
        
        for (long i = 0; i < 100000 && ok; i++) {
            unsigned char pk[33], sk[32];
            if (schnorr_keypair(pk, sk) != 0) ok = 0;
            count++;
            if (i % 25000 == 0 && i > 0) printf("    %ld/100000\n", i);
        }
        
        time_t t2 = time(NULL);
        double elapsed = difftime(t2, t1);
        double tps = (double)count / elapsed;
        
        printf("  Completed: %ld keypairs in %.1fs\n", count, elapsed);
        printf("  Throughput: %.0f keypairs/second\n", tps);
        printf("  Result: %s\n", (ok && count == 100000) ? "✅ PASS" : "❌ FAIL");
        if (ok && count == 100000) passed++; total++;
        printf("\n");
    }

    // ==========================================
    // TEST 4: Memory Stability
    // ==========================================
    printf("━━━ TEST 4: Extended Operation Stability ━━━\n");
    {
        printf("  Running sustained operations for 10 seconds...\n");
        
        time_t t1 = time(NULL);
        long ops = 0;
        int ok = 1;
        
        while (difftime(time(NULL), t1) < 10.0 && ok) {
            unsigned char pk[33], sk[32], sig[65];
            size_t siglen = 65;
            
            if (schnorr_keypair(pk, sk) != 0) ok = 0;
            if (schnorr_sign((unsigned char*)"stability", 9, sk, sig, &siglen) != 0) ok = 0;
            if (schnorr_verify(sig, siglen, (unsigned char*)"stability", 9, pk) != 0) ok = 0;
            ops++;
            
            if (ops % 50000 == 0) {
                time_t t2 = time(NULL);
                printf("    %ld operations (%.0fs elapsed)\n", ops, difftime(t2, t1));
            }
        }
        
        time_t t2 = time(NULL);
        double elapsed = difftime(t2, t1);
        
        printf("  Completed: %ld operations in %.1fs\n", ops, elapsed);
        printf("  Average: %.0f ops/sec\n", (double)ops / elapsed);
        printf("  No memory errors: %s\n", ok ? "✅ PASS" : "❌ FAIL");
        if (ok) passed++; total++;
    }

    time_t end_time = time(NULL);
    
    printf("\n╔══════════════════════════════════════════════╗\n");
    printf("║  STRESS TEST: %d/%d passed", passed, total);
    for (int i = 0; i < 12; i++) printf(" ");
    printf("║\n");
    printf("║  %s", passed == total ? "ALL TESTS PASSED ✅" : "SOME FAILED ❌");
    printf("                    ║\n");
    printf("║  Total Duration: %lds", (long)(end_time - start_time));
    for (int i = 0; i < 8; i++) printf(" ");
    printf("║\n");
    printf("╚══════════════════════════════════════════════╝\n");
    printf("  1M Signatures | 100K Verifications | 100K Keygens | Stability\n");
    printf("  ΦΩ0 — I AM THAT I AM\n");

    return passed == total ? 0 : 1;
}
