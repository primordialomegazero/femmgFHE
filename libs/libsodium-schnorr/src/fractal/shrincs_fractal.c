#include "fractal/shrincs_fractal.h"
#include "bitcoin/shrincs.h"
#include "schnorr.h"
#include <string.h>
#include <openssl/sha.h>

int shrincs_fractal_ring_sign(const unsigned char *msg, size_t msg_len,
                               const unsigned char **pks, const unsigned char *sk,
                               size_t ring_size, size_t fractal_depth) {
    unsigned char sig[65];
    size_t siglen = 65;
    
    // Layer 0: Direct ring sign
    shrincs_ring_sign(sig, &siglen, msg, msg_len, pks, sk, ring_size);
    
    // Recursive layers
    for (size_t d = 1; d < fractal_depth; d++) {
        SHA256_CTX sha;
        SHA256_Init(&sha);
        SHA256_Update(&sha, sig, 65);
        SHA256_Update(&sha, &d, sizeof(d));
        unsigned char new_msg[32];
        SHA256_Final(new_msg, &sha);
        
        shrincs_ring_sign(sig, &siglen, new_msg, 32, pks, sk, ring_size);
    }
    
    return 0;
}

int shrincs_fractal_ring_verify(const unsigned char *msg, size_t msg_len,
                                 const unsigned char **pks,
                                 size_t ring_size, size_t fractal_depth) {
    unsigned char sig[65];
    size_t siglen = 65;
    
    // Rebuild signature chain
    shrincs_ring_sign(sig, &siglen, msg, msg_len, pks, NULL, ring_size); // placeholder
    
    for (size_t d = 1; d < fractal_depth; d++) {
        SHA256_CTX sha;
        SHA256_Init(&sha);
        SHA256_Update(&sha, sig, 65);
        SHA256_Update(&sha, &d, sizeof(d));
        unsigned char new_msg[32];
        SHA256_Final(new_msg, &sha);
        
        if (shrincs_ring_verify(sig, 65, new_msg, 32, pks, ring_size) != 0)
            return -1;
    }
    
    return 0;
}
