#include "fractal_clown.h"
#include "schnorr.h"
#include <stdlib.h>
#include <string.h>
#include <openssl/sha.h>
#include <math.h>

#define PHI 1.6180339887498948482
#define PHI_INV 0.6180339887498948482

int fractal_clown_sign(const unsigned char *msg, size_t msg_len,
                        const unsigned char *sk, sign_func sign_fn,
                        FractalClownNode *node, size_t depth, size_t branches) {
    if (!msg || !sk || !node || !sign_fn) return -1;
    if (depth > FRACTAL_MAX_DEPTH) return 0;
    
    node->depth = depth;
    node->branch_id = 0;
    node->phi_weight = pow(PHI_INV, depth + 1);
    node->self_healed = 0;
    node->child_count = 0;
    
    size_t sig_len = 65;
    sign_fn(msg, msg_len, sk, node->sig, &sig_len);
    
    if (depth < FRACTAL_MAX_DEPTH && branches > 0) {
        for (size_t b = 0; b < branches; b++) {
            SHA256_CTX sha;
            SHA256_Init(&sha);
            SHA256_Update(&sha, node->sig, 65);
            SHA256_Update(&sha, &depth, sizeof(depth));
            SHA256_Update(&sha, &b, sizeof(b));
            unsigned char child_msg[32];
            SHA256_Final(child_msg, &sha);
            
            node->children[b] = calloc(1, sizeof(FractalClownNode));
            fractal_clown_sign(child_msg, 32, sk, sign_fn, 
                               node->children[b], depth + 1, branches - 1);
            node->children[b]->branch_id = b;
            node->child_count++;
        }
    }
    
    return 0;
}

int fractal_clown_verify(const unsigned char *msg, size_t msg_len,
                          const unsigned char *pk, verify_func verify_fn,
                          const FractalClownNode *node) {
    if (!msg || !pk || !node || !verify_fn) return -1;
    
    if (verify_fn(node->sig, 65, msg, msg_len, pk) != 0) return -1;
    
    for (size_t b = 0; b < node->child_count; b++) {
        SHA256_CTX sha;
        SHA256_Init(&sha);
        SHA256_Update(&sha, node->sig, 65);
        SHA256_Update(&sha, &node->depth, sizeof(node->depth));
        SHA256_Update(&sha, &b, sizeof(b));
        unsigned char child_msg[32];
        SHA256_Final(child_msg, &sha);
        
        if (fractal_clown_verify(child_msg, 32, pk, verify_fn, node->children[b]) != 0)
            return -1;
    }
    
    return 0;
}

int fractal_clown_self_heal(FractalClownNode *node, const unsigned char *pk, 
                             verify_func verify_fn, const unsigned char *sk, sign_func sign_fn) {
    if (!node || !pk || !verify_fn || !sk || !sign_fn) return -1;
    
    for (size_t b = 0; b < node->child_count; b++) {
        // Recompute child message
        SHA256_CTX sha;
        SHA256_Init(&sha);
        SHA256_Update(&sha, node->sig, 65);
        SHA256_Update(&sha, &node->depth, sizeof(node->depth));
        SHA256_Update(&sha, &b, sizeof(b));
        unsigned char child_msg[32];
        SHA256_Final(child_msg, &sha);
        
        // Check if child is broken
        if (verify_fn(node->children[b]->sig, 65, child_msg, 32, pk) != 0) {
            // RE-SIGN the child!
            size_t sig_len = 65;
            sign_fn(child_msg, 32, sk, node->children[b]->sig, &sig_len);
            node->children[b]->self_healed = 1;
            
            // RECURSIVELY heal grandchildren
            fractal_clown_self_heal(node->children[b], pk, verify_fn, sk, sign_fn);
        }
    }
    
    return 0;
}

void fractal_clown_free(FractalClownNode *node) {
    if (!node) return;
    for (size_t b = 0; b < node->child_count; b++) {
        fractal_clown_free(node->children[b]);
        free(node->children[b]);
    }
}
