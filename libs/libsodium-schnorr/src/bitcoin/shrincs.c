#include "shrincs.h"
#include "../../include/schnorr.h"
#include <string.h>
#include <stdio.h>

int shrincs_ring_sign(unsigned char *sig, size_t *siglen,
                       const unsigned char *msg, size_t msglen,
                       const unsigned char **pks, const unsigned char *sk,
                       size_t ring_size) {
    // For now: Single signer Schnorr (ring_size=1)
    if (ring_size == 0 || !pks || !sk) return -1;
    return schnorr_sign(msg, msglen, sk, sig, siglen);
}

int shrincs_ring_verify(const unsigned char *sig, size_t siglen,
                         const unsigned char *msg, size_t msglen,
                         const unsigned char **pks, size_t ring_size) {
    if (ring_size == 0 || !pks) return -1;
    return schnorr_verify(sig, siglen, msg, msglen, pks[0]);
}

int shrincs_nonce_commit(unsigned char *commitment,
                          const unsigned char *nonce, size_t nonce_len) {
    // Simple SHA-256 commitment
    extern void SHA256(const unsigned char *d, size_t n, unsigned char *md);
    // Placeholder — use schnorr hash
    return 0;
}

int shrincs_invoice_create(char *invoice, size_t invoice_len,
                            const unsigned char *address,
                            uint64_t amount_satoshis,
                            const unsigned char *memo, size_t memo_len) {
    snprintf(invoice, invoice_len, "bitcoin:%s?amount=%lu&memo=%s",
             address, amount_satoshis, memo);
    return 0;
}

int shrincs_script_schnorr(unsigned char *script, size_t *script_len,
                            const unsigned char *pk) {
    // Taproot-style Schnorr script
    script[0] = 0x51; // OP_1 (Taproot)
    script[1] = 0x20; // 32 bytes
    memcpy(script + 2, pk, 32);
    *script_len = 34;
    return 0;
}
