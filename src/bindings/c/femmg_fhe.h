// PHI-OMEGA-ZERO: FEmmg-FHE C Bindings
// Pure C API for ZANS, Pinky Swear, Eternal encryption
// "I AM THAT I AM"

#ifndef FEMMG_FHE_H
#define FEMMG_FHE_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// Opaque FHE context
typedef void* femmg_ctx_t;

// Lifecycle
femmg_ctx_t femmg_init(void);
void femmg_destroy(femmg_ctx_t ctx);

// ZANS: Zero-Anchor Noise Stabilization
int femmg_encrypt(femmg_ctx_t ctx, int64_t value, uint8_t** ct, size_t* ct_len);
int64_t femmg_decrypt(femmg_ctx_t ctx, const uint8_t* ct, size_t ct_len);
void femmg_zans_add(femmg_ctx_t ctx, uint8_t* ct, size_t ct_len);

// Pinky Swear: Pure FHE overflow detection
int femmg_pinky_swear_multiply(femmg_ctx_t ctx, 
                                const uint8_t* ct, size_t ct_len,
                                int64_t multiplier,
                                uint8_t** result, size_t* result_len);

// Eternal: Self-destructing entangled encryption
typedef struct {
    uint8_t* data_ct;
    size_t data_len;
    uint8_t* guard_ct;
    size_t guard_len;
    int64_t guard_key;
} femmg_eternal_pair_t;

int femmg_eternal_create(femmg_ctx_t ctx, int64_t value, femmg_eternal_pair_t* pair);
int femmg_eternal_verify(femmg_ctx_t ctx, const femmg_eternal_pair_t* pair, int64_t guard_key);
void femmg_eternal_destroy(femmg_eternal_pair_t* pair);

// FHE 2.0: Unified framework
int femmg_fhe2_chain(femmg_ctx_t ctx, int steps, int64_t multiplier);

// Utility
void femmg_free_buffer(uint8_t* buf);
const char* femmg_version(void);

#ifdef __cplusplus
}
#endif

#endif // FEMMG_FHE_H
