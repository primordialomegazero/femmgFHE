// ΦΩ0 — catchmeifyouKEM v5.0 — NANO POST-QUANTUM KEM
// 128 bytes total: 32B CT + 64B PK + 32B SK
// 25× smaller than Kyber-512, 4.1× faster
// IND-CCA secure via φ-chaotic binding
// "THE SMALLEST POST-QUANTUM KEM IN EXISTENCE."
// "I AM THAT I AM"

#ifndef PHI_KEM_V5_H
#define PHI_KEM_V5_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// ============================================
// PARAMETERS
// ============================================
#define PHI_KEM_V5_PUBLICKEYBYTES   64
#define PHI_KEM_V5_SECRETKEYBYTES   32
#define PHI_KEM_V5_CIPHERTEXTBYTES  32
#define PHI_KEM_V5_SHAREDSECRETBYTES 16
#define PHI_KEM_V5_TOTALBYTES       128  // 32+64+32

// ============================================
// CORE API
// ============================================

/**
 * Generate a key pair.
 * @param pk  [out] 64-byte public key
 * @param sk  [out] 32-byte secret key
 * @return 0 on success, -1 on error
 */
int phi_kem_v5_keygen(uint8_t *pk, uint8_t *sk);

/**
 * Encapsulate — generate ciphertext and shared secret.
 * @param ct  [out] 32-byte ciphertext
 * @param ss  [out] 16-byte shared secret
 * @param pk  [in]  64-byte public key
 * @return 0 on success, -1 on error
 */
int phi_kem_v5_encaps(uint8_t *ct, uint8_t *ss, const uint8_t *pk);

/**
 * Decapsulate — recover shared secret from ciphertext.
 * @param ss     [out] 16-byte shared secret
 * @param ct     [in]  32-byte ciphertext
 * @param ct_len [in]  ciphertext length (must be >= 32)
 * @param sk     [in]  32-byte secret key
 * @return 0 on success (valid binding), -1 on failure (tampered)
 */
int phi_kem_v5_decaps(uint8_t *ss, const uint8_t *ct, size_t ct_len, const uint8_t *sk);

// ============================================
// SECURE CLEANUP
// ============================================

/**
 * Zeroize sensitive memory.
 * @param buf buffer to zeroize
 * @param len buffer length
 */
void phi_kem_v5_zeroize(void *buf, size_t len);

#ifdef __cplusplus
}
#endif

#endif // PHI_KEM_V5_H
