#include <stdio.h>
#include <string.h>
#include "../../src/crypto/phi_qr_kem.h"

int main() {
    uint8_t pk[32], sk[32], ct[32], ss_enc[16], ss_dec[16];
    
    qr_kem_keygen(pk, sk);
    printf("PK: "); for(int i=0;i<8;i++) printf("%02x",pk[i]); printf("\n");
    
    qr_kem_encaps(ct, ss_enc, pk);
    printf("SS: "); for(int i=0;i<8;i++) printf("%02x",ss_enc[i]); printf("\n");
    printf("CT: "); for(int i=0;i<8;i++) printf("%02x",ct[i]); printf("\n\n");
    
    // Manual decaps trace
    uint8_t pk2[32];
    phi_hash(sk, 32, pk2);
    printf("PK2: "); for(int i=0;i<8;i++) printf("%02x",pk2[i]); 
    printf(" match=%d\n", memcmp(pk, pk2, 32) == 0);
    
    // Mask
    uint8_t mask[16];
    uint8_t combined[39];
    memcpy(combined, pk2, 32);
    memcpy(combined+32, "encaps", 6);
    phi_hash(combined, 38, mask);
    
    // Decrypt ss
    for(int i=0;i<16;i++) ss_dec[i] = ct[i] ^ mask[i];
    printf("SS2: "); for(int i=0;i<8;i++) printf("%02x",ss_dec[i]);
    printf(" match=%d\n", memcmp(ss_enc, ss_dec, 16) == 0);
    
    // Binding seed
    uint8_t bs1[48], bs2[48];
    memcpy(bs1, pk, 32); memcpy(bs1+32, ss_enc, 16);
    memcpy(bs2, pk2, 32); memcpy(bs2+32, ss_dec, 16);
    printf("BS1: "); for(int i=0;i<8;i++) printf("%02x",bs1[i]); printf("\n");
    printf("BS2: "); for(int i=0;i<8;i++) printf("%02x",bs2[i]); 
    printf(" match=%d\n", memcmp(bs1, bs2, 48) == 0);
    
    // Spiral
    uint8_t sp1[32], sp2[32];
    fib_spiral(bs1, 48, sp1, 3);
    fib_spiral(bs2, 48, sp2, 3);
    printf("SP1: "); for(int i=0;i<8;i++) printf("%02x",sp1[i]); printf("\n");
    printf("SP2: "); for(int i=0;i<8;i++) printf("%02x",sp2[i]);
    printf(" match=%d\n", memcmp(sp1, sp2, 32) == 0);
    
    return 0;
}
