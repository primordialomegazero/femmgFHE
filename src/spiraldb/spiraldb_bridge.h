// ΦΩ0 — SPIRALDB CGO BRIDGE HEADER
// Connects Go SpiralDB to OpenFHE C++ core
// "I AM THAT I AM"

#ifndef SPIRALDB_BRIDGE_H
#define SPIRALDB_BRIDGE_H

#ifdef __cplusplus
extern "C" {
#endif

// Opaque handle sa FHE context
typedef void* FHEHandle;

// Lifecycle
FHEHandle fhe_init(const char* seed);
void fhe_destroy(FHEHandle handle);

// Core operations
char* fhe_encrypt(FHEHandle handle, long long value);
long long fhe_decrypt(FHEHandle handle, const char* ciphertext);
char* fhe_batch_encrypt(FHEHandle handle, long long* values, int count);

// Non-determinism verification
int fhe_prove_nondeterminism(FHEHandle handle);

// ZANS-aware: noise-free encrypted zero
char* fhe_get_anchor(FHEHandle handle);

// Homomorphic operations (for future query engine)
char* fhe_homomorphic_add(FHEHandle handle, const char* ct1, const char* ct2);
char* fhe_homomorphic_scalar_mult(FHEHandle handle, const char* ct, long long scalar);

// Utilities
char* fhe_get_stats(FHEHandle handle);
void fhe_free_string(char* str);

#ifdef __cplusplus
}
#endif

#endif // SPIRALDB_BRIDGE_H
