#ifndef SCHNORR_FRACTAL_H
#define SCHNORR_FRACTAL_H

#include <stddef.h>

#define FRACTAL_MAX_DEPTH 7
#define FRACTAL_MAX_BRANCHES 5
#define FRACTAL_SIGNATURE_BYTES 65

typedef struct FractalSignature {
    unsigned char sig[65];
    size_t depth;
    size_t branch_id;
    struct FractalSignature *children[5];
    size_t child_count;
} FractalSignature;

int schnorr_fractal_sign(const unsigned char *msg, size_t msg_len,
                          const unsigned char *sk,
                          FractalSignature *root,
                          size_t depth, size_t branches);

int schnorr_fractal_verify(const unsigned char *msg, size_t msg_len,
                            const unsigned char *pk,
                            const FractalSignature *node);

void schnorr_fractal_free(FractalSignature *node);

#endif
