#include "phi_algo_merge.h"
#include <stdio.h>
#include <string.h>

int main() {
    printf("Test: FEmmg Integration\n");
    printf("=======================\n\n");
    
    const char *msg = "This is a secret message for FEmmg-FHE!";
    size_t len = strlen(msg);
    uint8_t plaintext[64], ciphertext[64], decrypted[64];
    uint8_t nonce[32];
    
    memcpy(plaintext, msg, len);
    
    // Encrypt
    phi_secure_encrypt(plaintext, len, ciphertext, nonce);
    printf("Plaintext:  %s\n", plaintext);
    printf("Ciphertext: ");
    for(int i=0; i<16; i++) printf("%02x", ciphertext[i]);
    printf("...\n");
    printf("Nonce:      ");
    for(int i=0; i<8; i++) printf("%02x", nonce[i]);
    printf("...\n\n");
    
    // Decrypt (same nonce)
    for(size_t i=0; i<len; i++) {
        decrypted[i] = ciphertext[i] ^ nonce[i % 32];
    }
    decrypted[len] = 0;
    
    printf("Decrypted:  %s\n", decrypted);
    printf("Match:      %s\n\n", memcmp(plaintext, decrypted, len) == 0 ? "✅" : "❌");
    
    // Wrong nonce test
    uint8_t wrong_nonce[32];
    RAND_bytes(wrong_nonce, 32);
    for(size_t i=0; i<len; i++) {
        decrypted[i] = ciphertext[i] ^ wrong_nonce[i % 32];
    }
    decrypted[len] = 0;
    printf("Wrong nonce: %s\n", decrypted);
    printf("Garbage:    %s\n", memcmp(plaintext, decrypted, len) != 0 ? "✅" : "❌");
    
    return 0;
}
