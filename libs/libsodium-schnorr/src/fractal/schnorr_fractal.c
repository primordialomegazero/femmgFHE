#include "fractal/schnorr_fractal.h"
#include "schnorr.h"
#include <stdlib.h>
#include <string.h>
#include <openssl/sha.h>

int schnorr_fractal_sign(const unsigned char *msg, size_t msg_len,
                          const unsigned char *sk,
                          FractalSignature *node,
                          size_t depth, size_t branches) {
    if (!msg || !sk || !node || depth > FRACTAL_MAX_DEPTH) return -1;
    
    // Sign this node
    size_t siglen = 65;
    schnorr_sign(msg, msg_len, sk, node->sig, &siglen);
    node->depth = depth;
    node->branch_id = 0;
    node->child_count = 0;
    
    // Recursively sign children
    if (depth < FRACTAL_MAX_DEPTH && branches > 0) {
        for (size_t b = 0; b < branches; b++) {
            // Create child message = H(parent_sig || depth || branch)
            SHA256_CTX sha;
            SHA256_Init(&sha);
            SHA256_Update(&sha, node->sig, 65);
            SHA256_Update(&sha, &depth, sizeof(depth));
            SHA256_Update(&sha, &b, sizeof(b));
            unsigned char child_msg[32];
            SHA256_Final(child_msg, &sha);
            
            node->children[b] = calloc(1, sizeof(FractalSignature));
            schnorr_fractal_sign(child_msg, 32, sk, node->children[b], 
                                  depth + 1, branches - 1);
            node->child_count++;
        }
    }
    
    return 0;
}

int schnorr_fractal_verify(const unsigned char *msg, size_t msg_len,
                            const unsigned char *pk,
                            const FractalSignature *node) {
    if (!msg || !pk || !node) return -1;
    
    // Verify this node
    if (schnorr_verify(node->sig, 65, msg, msg_len, pk) != 0) return -1;
    
    // Recursively verify children
    for (size_t b = 0; b < node->child_count; b++) {
        SHA256_CTX sha;
        SHA256_Init(&sha);
        SHA256_Update(&sha, node->sig, 65);
        SHA256_Update(&sha, &node->depth, sizeof(node->depth));
        SHA256_Update(&sha, &b, sizeof(b));
        unsigned char child_msg[32];
        SHA256_Final(child_msg, &sha);
        
        if (schnorr_fractal_verify(child_msg, 32, pk, node->children[b]) != 0)
            return -1;
    }
    
    return 0;
}

void schnorr_fractal_free(FractalSignature *node) {
    if (!node) return;
    for (size_t b = 0; b < node->child_count; b++) {
        schnorr_fractal_free(node->children[b]);
        free(node->children[b]);
    }
}
