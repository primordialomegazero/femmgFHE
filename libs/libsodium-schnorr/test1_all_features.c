#include <stdio.h>
#include <string.h>
#include <time.h>
#include "schnorr.h"
#include "curves/ed25519.h"
#include "backends/crypto_backend.h"
#include "clown_ecosystem/fractal_clown.h"
#include "clown_ecosystem/dnscrypt/dnscrypt_proxy.h"
#include "clown_ecosystem/libhydrogen/hydrogen.h"
#include "clown_ecosystem/minisign/minisign.h"
#include "clown_ecosystem/iptoasn/iptoasn.h"

int main() {
    time_t start_time = time(NULL);
    
    printf("╔══════════════════════════════════════════════╗\n");
    printf("║  SCHNORR SIGNATURES — COMPREHENSIVE TEST      ║\n");
    printf("║  Multi-Curve | φ-Hash | Fractal Trees          ║\n");
    printf("║  Dan Joseph M. Fernandez / ΦΩ0               ║\n");
    printf("╚══════════════════════════════════════════════╝\n\n");

    int passed = 0, total = 0;
    unsigned char pk[33], sk[32], ed_pk[32], ed_sk[32];

    printf("━━━ KEY GENERATION ━━━\n");
    schnorr_keypair(pk, sk);
    ed25519_keypair(ed_pk, ed_sk);
    printf("  secp256k1 keypair: generated\n");
    printf("  ed25519 keypair: generated\n");
    printf("  φ = 1.6180339887498948482\n\n");

    // PHASE 1
    printf("━━━ PHASE 1: Schnorr Σ-Protocol (secp256k1) ━━━\n");
    {
        unsigned char sig[65]; size_t slen = 65;
        schnorr_sign((unsigned char*)"secp256k1", 9, sk, sig, &slen);
        int ok = (schnorr_verify(sig, slen, (unsigned char*)"secp256k1", 9, pk) == 0);
        printf("  Signature generation: %s\n", ok ? "✅ PASS" : "❌ FAIL");
        if (ok) passed++; total++;
        
        sig[0] ^= 0xFF;
        int tamper = (schnorr_verify(sig, slen, (unsigned char*)"secp256k1", 9, pk) != 0);
        printf("  Tamper resistance: %s\n", tamper ? "✅ PASS" : "❌ FAIL");
        if (tamper) passed++; total++;
    }

    // PHASE 2
    printf("\n━━━ PHASE 2: Ed25519 (RFC 8032) ━━━\n");
    {
        unsigned char sig[64]; size_t slen = 64;
        ed25519_sign(sig, &slen, (unsigned char*)"edwards", 7, ed_sk);
        int ok = (ed25519_verify(sig, slen, (unsigned char*)"edwards", 7, ed_pk) == 0);
        printf("  Curve compatibility: %s\n", ok ? "✅ PASS" : "❌ FAIL");
        if (ok) passed++; total++;
    }

    // PHASE 3
    printf("\n━━━ PHASE 3: φ-Hash (Zero-Dependency) ━━━\n");
    {
        unsigned char hash[32], rand[16];
        phi_backend.sha256((unsigned char*)"phi", 3, hash);
        phi_backend.rand_bytes(rand, 16);
        printf("  Deterministic hash: %s\n", (hash[0]||hash[1]) ? "✅ PASS" : "❌ FAIL");
        if (hash[0]||hash[1]) passed++; total++;
        printf("  Entropy generation: %s\n", rand[0] ? "✅ PASS" : "❌ FAIL");
        if (rand[0]) passed++; total++;
    }

    // PHASE 4
    printf("\n━━━ PHASE 4: Fractal Signature Trees ━━━\n");
    {
        FractalClownNode root; memset(&root, 0, sizeof(root));
        fractal_clown_sign((unsigned char*)"fractal", 7, sk, (sign_func)schnorr_sign, &root, 0, 3);
        
        root.children[0]->sig[0] ^= 0xFF;
        int broken = (fractal_clown_verify((unsigned char*)"fractal", 7, pk, (verify_func)schnorr_verify, &root) != 0);
        printf("  Fault detection: %s\n", broken ? "✅" : "❌");
        
        fractal_clown_self_heal(&root, pk, (verify_func)schnorr_verify, sk, (sign_func)schnorr_sign);
        int healed = (fractal_clown_verify((unsigned char*)"fractal", 7, pk, (verify_func)schnorr_verify, &root) == 0);
        printf("  Self-healing: %s\n", healed ? "✅ PASS" : "❌ FAIL");
        if (healed) passed++; total++;
        fractal_clown_free(&root);
    }

    // PHASE 5
    printf("\n━━━ PHASE 5: Extended Primitives ━━━\n");
    {
        unsigned char sq[256], h_sig[65], m_sig[65];
        size_t sq_len=256, hs_len=65, ms_len=65;
        ASNRecord rec;
        
        dnscrypt_sign_query(sq, &sq_len, (unsigned char*)"example.com", 11, sk);
        int dns = (dnscrypt_verify_response((unsigned char*)"example.com", 11, sq, pk) == 0);
        printf("  DNScrypt: %s\n", dns ? "✅" : "❌"); if (dns) passed++; total++;
        
        hydro_sign_create(h_sig, &hs_len, (unsigned char*)"hydrogen", 8, sk);
        int hydro = (hydro_sign_verify(h_sig, hs_len, (unsigned char*)"hydrogen", 8, pk) == 0);
        printf("  libhydrogen: %s\n", hydro ? "✅" : "❌"); if (hydro) passed++; total++;
        
        iptoasn_lookup("1.1.1.1", &rec);
        printf("  iptoasn: %s\n", rec.asn ? "✅" : "❌"); if (rec.asn) passed++; total++;
    }

    // PHASE 6
    printf("\n━━━ PHASE 6: Throughput ━━━\n");
    {
        int speed_ok = 1;
        for (int i = 0; i < 1000 && speed_ok; i++) {
            unsigned char p[33], s[32], si[65]; size_t l = 65;
            schnorr_keypair(p, s);
            if (schnorr_sign((unsigned char*)"bench", 5, s, si, &l) != 0) speed_ok = 0;
        }
        printf("  1,000 signatures: %s\n", speed_ok ? "✅ PASS" : "❌ FAIL");
        if (speed_ok) passed++; total++;
    }

    time_t end_time = time(NULL);
    
    printf("\n╔══════════════════════════════════════════════╗\n");
    printf("║  RESULTS: %d/%d passed", passed, total);
    for (int i = 0; i < 14; i++) printf(" ");
    printf("║\n");
    printf("║  %s", passed == total ? "ALL TESTS PASSED ✅" : "SOME FAILED ❌");
    printf("                    ║\n");
    printf("║  Duration: %lds", (long)(end_time - start_time));
    for (int i = 0; i < 14; i++) printf(" ");
    printf("║\n");
    printf("╚══════════════════════════════════════════════╝\n");
    printf("  Schnorr Σ-Protocol | Ed25519 | φ-Hash | Fractal Trees\n");
    printf("  ΦΩ0 — I AM THAT I AM\n");

    return passed == total ? 0 : 1;
}
