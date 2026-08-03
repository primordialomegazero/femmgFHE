#pragma once
// ═══════════════════════════════════════════════════════════════════════════════
// libspiral.h — Stable C API for Spiral Fractal iO
// ═══════════════════════════════════════════════════════════════════════════════
#ifndef LIBSPIRAL_H
#define LIBSPIRAL_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// ═══════════════════════════════════════════════════════════════
// VERSION
// ═══════════════════════════════════════════════════════════════
#define SPIRAL_VERSION_MAJOR 31
#define SPIRAL_VERSION_MINOR 7
#define SPIRAL_VERSION_PATCH 0
const char* spiral_version(void);

// ═══════════════════════════════════════════════════════════════
// OBFUSCATION
// ═══════════════════════════════════════════════════════════════
// Obfuscate a program. Returns 0 on success.
//   source:      C/C++ source code (null-terminated string)
//   ring_dim:    Ring dimension (4096, 16384, 32768). 0 = auto.
//   N_gates:     Number of gates. 0 = auto (source length).
//   output_path: Where to write .obf file
int spiral_obfuscate(const char* source, uint32_t ring_dim, uint32_t N_gates, 
                     const char* output_path);

// ═══════════════════════════════════════════════════════════════
// EXECUTION
// ═══════════════════════════════════════════════════════════════
// Execute an obfuscated program. Returns number of output values.
//   obf_path:    Path to .obf file
//   inputs:      Array of input values
//   n_inputs:    Number of inputs
//   outputs:     Output buffer (caller-allocated, min 256 doubles)
int spiral_execute(const char* obf_path, const double* inputs, int n_inputs,
                   double* outputs);

// ═══════════════════════════════════════════════════════════════
// VERIFICATION
// ═══════════════════════════════════════════════════════════════
// Verify two programs are indistinguishable (KS test).
// Returns 0 on success, stores KS statistic in ks_score.
// KS < 0.001 = indistinguishable.
int spiral_verify(const char* prog_a, const char* prog_b, uint32_t ring_dim,
                  double* ks_score);

// ═══════════════════════════════════════════════════════════════
// KEY ENCAPSULATION (Ultra Rashomon KEM)
// ═══════════════════════════════════════════════════════════════
// Generate a keypair. pk and sk must be 64 bytes each.
int spiral_kem_keygen(uint8_t* pk, uint8_t* sk);

// Encapsulate: ct (64 bytes) and shared_secret (32 bytes).
int spiral_kem_encaps(uint8_t* ct, uint8_t* shared_secret, const uint8_t* pk);

// Decapsulate: shared_secret (32 bytes).
int spiral_kem_decaps(uint8_t* shared_secret, const uint8_t* ct, const uint8_t* sk);

// ═══════════════════════════════════════════════════════════════
// PHI STACK VERIFICATION
// ═══════════════════════════════════════════════════════════════
// Verify φ/ψ identities. Returns 1 if all identities hold.
int spiral_verify_phi_identities(void);

#ifdef __cplusplus
}
#endif

#endif // LIBSPIRAL_H
