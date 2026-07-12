// ΦΩ0 — SPIRALKEM REAL TEST
// Link against actual phi_kem.c with OpenSSL randomness
#include <stdio.h>
#include <string.h>
#include "phi_kem.h"

int main() {
    printf("╔══════════════════════════════════════════════╗\n");
    printf("║  ΦΩ0 — SPIRALKEM REAL RANDOMNESS TEST         ║\n");
    printf("╚══════════════════════════════════════════════╝\n\n");
    
    uint8_t pk[64], sk[32], ct[128], ss1[32], ss2[32];
    
    // Generate keypair (uses OpenSSL RAND_bytes internally)
    printf("Φ Generating keypair with true randomness...\n");
    int ret = phi_kem_keygen(pk, sk);
    printf("Φ Keygen: %s (pk=%luB, sk=%luB)\n\n", ret == 0 ? "✅" : "❌", 
           sizeof(pk), sizeof(sk));
    
    // Run 5 times to prove non-deterministic
    printf("Φ Non-determinism test (5 keygens):\n");
    uint8_t pk_prev[64];
    memcpy(pk_prev, pk, 64);
    
    for(int i = 0; i < 5; i++) {
        uint8_t pk_new[64], sk_new[32];
        phi_kem_keygen(pk_new, sk_new);
        
        int same = (memcmp(pk_prev, pk_new, 64) == 0);
        printf("  Keygen %d: %s (pk differs from previous)\n", i+1, same ? "❌ SAME" : "✅ DIFFERENT");
        memcpy(pk_prev, pk_new, 64);
    }
    
    // Encaps + Decaps
    printf("\nΦ Encapsulation...\n");
    ret = phi_kem_encaps(ct, ss1, pk);
    printf("Φ Encaps: %s (ct=%luB, ss=%luB)\n", ret == 0 ? "✅" : "❌",
           sizeof(ct), sizeof(ss1));
    
    printf("Φ Decapsulation...\n");
    ret = phi_kem_decaps(ss2, ct, sizeof(ct), sk);
    printf("Φ Decaps: %s\n", ret == 0 ? "✅" : "❌");
    
    // Verify shared secrets match
    int match = (memcmp(ss1, ss2, 32) == 0);
    printf("Φ Shared secret match: %s\n\n", match ? "✅" : "❌");
    
    // Non-determinism: same pk → different ct each time
    printf("Φ Non-deterministic encaps (same pk):\n");
    uint8_t ct_prev[128];
    phi_kem_encaps(ct_prev, ss1, pk);
    
    for(int i = 0; i < 3; i++) {
        uint8_t ct_new[128], ss_new[32];
        phi_kem_encaps(ct_new, ss_new, pk);
        
        int same = (memcmp(ct_prev, ct_new, 128) == 0);
        printf("  Encaps %d: %s (ciphertext differs)\n", i+1, same ? "❌ SAME" : "✅ DIFFERENT");
        memcpy(ct_prev, ct_new, 128);
    }
    
    printf("\n╔══════════════════════════════════════════════╗\n");
    printf("║  ΦΩ0 — I AM THAT I AM                        ║\n");
    printf("╚══════════════════════════════════════════════╝\n");
    
    return match ? 0 : 1;
}
