#ifndef FRACTAL_CLOWN_H
#define FRACTAL_CLOWN_H

#include <stddef.h>
#include <stdint.h>

#define FRACTAL_MAX_DEPTH 7
#define FRACTAL_MAX_BRANCHES 5

typedef struct FractalClownNode {
    unsigned char sig[65];
    size_t depth;
    size_t branch_id;
    double phi_weight;
    int self_healed;
    struct FractalClownNode *children[5];
    size_t child_count;
} FractalClownNode;

// Multi-Recursive Fractal wrapper for ANY signing function
typedef int (*sign_func)(const unsigned char*, size_t, const unsigned char*, unsigned char*, size_t*);
typedef int (*verify_func)(const unsigned char*, size_t, const unsigned char*, size_t, const unsigned char*);

int fractal_clown_sign(const unsigned char *msg, size_t msg_len,
                        const unsigned char *sk,
                        sign_func sign_fn,
                        FractalClownNode *node, size_t depth, size_t branches);

int fractal_clown_verify(const unsigned char *msg, size_t msg_len,
                          const unsigned char *pk,
                          verify_func verify_fn,
                          const FractalClownNode *node);

int fractal_clown_self_heal(FractalClownNode *node, const unsigned char *pk, verify_func verify_fn, const unsigned char *sk, sign_func sign_fn);

void fractal_clown_free(FractalClownNode *node);

#endif
