#include "hydrogen.h"
#include "schnorr.h"
#include <string.h>
#include <openssl/sha.h>
#include <openssl/rand.h>

int hydro_sign_keypair(unsigned char *pk, unsigned char *sk) {
    return schnorr_keypair(pk, sk);
}

int hydro_sign_create(unsigned char *sig, size_t *sig_len,
                       const unsigned char *msg, size_t msg_len,
                       const unsigned char *sk) {
    return schnorr_sign(msg, msg_len, sk, sig, sig_len);
}

int hydro_sign_verify(const unsigned char *sig, size_t sig_len,
                       const unsigned char *msg, size_t msg_len,
                       const unsigned char *pk) {
    return schnorr_verify(sig, sig_len, msg, msg_len, pk);
}

int hydro_hash_hash(unsigned char *out, size_t out_len,
                     const unsigned char *in, size_t in_len) {
    SHA256_CTX sha;
    SHA256_Init(&sha);
    SHA256_Update(&sha, in, in_len);
    unsigned char hash[32];
    SHA256_Final(hash, &sha);
    size_t copy_len = out_len < 32 ? out_len : 32;
    memcpy(out, hash, copy_len);
    return 0;
}

int hydro_secretbox_encrypt(unsigned char *ct, const unsigned char *msg, size_t msg_len,
                             const unsigned char *nonce, const unsigned char *key) {
    // Simplified: XOR with key stream
    unsigned char stream[64];
    SHA256_CTX sha;
    SHA256_Init(&sha);
    SHA256_Update(&sha, key, 32);
    SHA256_Update(&sha, nonce, 24);
    SHA256_Final(stream, &sha);
    for (size_t i = 0; i < msg_len; i++) ct[i] = msg[i] ^ stream[i % 32];
    return 0;
}

int hydro_secretbox_decrypt(unsigned char *msg, const unsigned char *ct, size_t ct_len,
                             const unsigned char *nonce, const unsigned char *key) {
    return hydro_secretbox_encrypt(msg, ct, ct_len, nonce, key);
}
