#include <stdio.h>
#include <string.h>
#include "../../src/crypto/phi_qr_kem.h"

int main() {
    printf("=== PHI-QR-KEM DEBUG ===\n\n");
    
    // Test RNG
    printf("--- RNG Test ---\n");
    phi_rng_seed();
    uint8_t rng_out[32];
    phi_randombytes(rng_out, 32);
    
    int zeros = 0;
    for (int i = 0; i < 32; i++) if (rng_out[i] == 0) zeros++;
    printf("  RNG zeros: %d/32\n", zeros);
    printf("  RNG sample: ");
    for (int i = 0; i < 8; i++) printf("%02x", rng_out[i]);
    printf("\n\n");
    
    // Test Hash
    printf("--- Hash Test ---\n");
    uint8_t hash_out[32];
    phi_hash((const uint8_t*)"test", 4, hash_out);
    printf("  Hash of 'test': ");
    for (int i = 0; i < 8; i++) printf("%02x", hash_out[i]);
    printf("\n");
    
    // Determinism check
    uint8_t hash_out2[32];
    phi_hash((const uint8_t*)"test", 4, hash_out2);
    int hash_match = (memcmp(hash_out, hash_out2, 32) == 0);
    printf("  Hash deterministic: %s\n\n", hash_match ? "YES" : "NO");
    
    // Test Keygen
    printf("--- Keygen Test ---\n");
    uint8_t pk[QR_KEM_PUBLICKEYBYTES], sk[QR_KEM_SECRETKEYBYTES];
    qr_kem_keygen(pk, sk);
    printf("  PK: ");
    for (int i = 0; i < 8; i++) printf("%02x", pk[i]);
    printf("\n");
    printf("  SK: ");
    for (int i = 0; i < 8; i++) printf("%02x", sk[i]);
    printf("\n\n");
    
    // Full trace for ONE encaps/decaps
    printf("--- Full Trace ---\n");
    
    qr_kem_keygen(pk, sk);
    
    // Encaps
    uint8_t ss_enc[QR_KEM_SHAREDSECRETBYTES];
    uint8_t ct[QR_KEM_CIPHERTEXTBYTES];
    int enc_ret = qr_kem_encaps(ct, ss_enc, pk);
    printf("  Encaps ret: %d\n", enc_ret);
    printf("  SS_enc: ");
    for (int i = 0; i < 8; i++) printf("%02x", ss_enc[i]);
    printf("\n");
    printf("  CT: ");
    for (int i = 0; i < 8; i++) printf("%02x", ct[i]);
    printf("\n\n");
    
    // Decaps
    uint8_t ss_dec[QR_KEM_SHAREDSECRETBYTES];
    int dec_ret = qr_kem_decaps(ss_dec, ct, sk);
    printf("  Decaps ret: %d\n", dec_ret);
    printf("  SS_dec: ");
    for (int i = 0; i < 8; i++) printf("%02x", ss_dec[i]);
    printf("\n");
    
    int match = (dec_ret == 0 && memcmp(ss_enc, ss_dec, QR_KEM_SHAREDSECRETBYTES) == 0);
    printf("  Match: %s\n", match ? "YES" : "NO");
    
    return match ? 0 : 1;
}
