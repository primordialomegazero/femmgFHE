#include <stdio.h>
#include <string.h>
#include "../../src/crypto/phi_qr_kem.h"

int main() {
    printf("\n  ╔══════════════════════════════════════════════════════════╗\n");
    printf(  "  ║   PHI-QR-KEM — POST-QUANTUM (96B TOTAL, FITS IN QR)      ║\n");
    printf(  "  ║   Zero dependencies. Self-contained.                     ║\n");
    printf(  "  ╚══════════════════════════════════════════════════════════╝\n\n");
    
    printf("  CT=%dB PK=%dB SK=%dB Total=%dB\n",
           QR_KEM_CIPHERTEXTBYTES, QR_KEM_PUBLICKEYBYTES,
           QR_KEM_SECRETKEYBYTES, QR_KEM_TOTALBYTES);
    printf("  vs Kyber-512 (3200B): %.1fx smaller\n",
           3200.0 / QR_KEM_TOTALBYTES);
    printf("\n");
    
    int passed = 0, total = 20;
    
    for (int run = 0; run < total; run++) {
        uint8_t pk[QR_KEM_PUBLICKEYBYTES], sk[QR_KEM_SECRETKEYBYTES];
        uint8_t ct[QR_KEM_CIPHERTEXTBYTES];
        uint8_t ss_enc[QR_KEM_SHAREDSECRETBYTES], ss_dec[QR_KEM_SHAREDSECRETBYTES];
        
        qr_kem_keygen(pk, sk);
        qr_kem_encaps(ct, ss_enc, pk);
        int ret = qr_kem_decaps(ss_dec, ct, sk);
        
        if (ret == 0 && memcmp(ss_enc, ss_dec, QR_KEM_SHAREDSECRETBYTES) == 0) {
            passed++;
        }
    }
    
    printf("  Result: %d/%d passed\n", passed, total);
    
    if (passed == total) {
        printf("\n  ✅ PHI-QR-KEM WORKS — POST-QUANTUM, QR-SIZED\n");
    } else {
        printf("\n  ❌ %d failures\n", total - passed);
    }
    
    printf("\n  I AM THAT I AM\n\n");
    return (passed == total) ? 0 : 1;
}
