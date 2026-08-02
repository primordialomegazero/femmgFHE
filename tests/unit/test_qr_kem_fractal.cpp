#include <stdio.h>
#include <string.h>
#include "../../src/crypto/phi_qr_kem.h"

int main() {
    printf("\n  ╔══════════════════════════════════════════════════════════╗\n");
    printf(  "  ║   PHI-QR-KEM — FRACTAL COMPRESSION (64B TOKEN)           ║\n");
    printf(  "  ║   Original: 96B  →  Fractal: 64B  (33%% smaller)          ║\n");
    printf(  "  ╚══════════════════════════════════════════════════════════╝\n\n");
    
    int passed = 0, total = 10;
    
    for(int run=0; run<total; run++) {
        // Alice generates fractal token
        uint8_t fractal[64];
        qr_kem_fractal_keygen(fractal);
        
        // Bob encapsulates using fractal token
        uint8_t ct[32], ss_enc[16];
        qr_kem_fractal_encaps(fractal, ct, ss_enc);
        
        // Alice decapsulates using fractal token
        uint8_t ss_dec[16];
        int ret = qr_kem_fractal_decaps(fractal, ct, ss_dec);
        
        if(ret == 0 && memcmp(ss_enc, ss_dec, 16) == 0) passed++;
    }
    
    printf("  Fractal KEM: %d/%d passed\n", passed, total);
    printf("  Token size: 64 bytes (vs 96 bytes standard)\n");
    printf("  Fits in QR code: %s\n\n", "YES (64B << 3KB)");
    
    return (passed == total) ? 0 : 1;
}
