#include <stdlib.h>
#include <string.h>
#include "minisign.h"
#include "schnorr.h"

int minisign_keypair(unsigned char *pk, unsigned char *sk) {
    return schnorr_keypair(pk, sk);
}

int minisign_sign(const unsigned char *msg, size_t msg_len,
                   const unsigned char *sk,
                   unsigned char *sig, size_t *sig_len,
                   const char *trusted_comment, const char *untrusted_comment) {
    size_t tlen = trusted_comment ? strlen(trusted_comment) : 0;
    size_t ulen = untrusted_comment ? strlen(untrusted_comment) : 0;
    
    // Build full message: msg || trusted || untrusted
    size_t full_len = msg_len + tlen + ulen + 2;
    unsigned char *full_msg = malloc(full_len);
    size_t pos = 0;
    memcpy(full_msg + pos, msg, msg_len); pos += msg_len;
    if (trusted_comment) { memcpy(full_msg + pos, trusted_comment, tlen); pos += tlen; }
    if (untrusted_comment) { memcpy(full_msg + pos, untrusted_comment, ulen); pos += ulen; }
    
    int ret = schnorr_sign(full_msg, full_len, sk, sig, sig_len);
    free(full_msg);
    return ret;
}

int minisign_verify(const unsigned char *msg, size_t msg_len,
                     const unsigned char *sig, size_t sig_len,
                     const unsigned char *pk) {
    // For verification without comments, just verify raw message
    return schnorr_verify(sig, sig_len, msg, msg_len, pk);
}

// Verify with comments (full minisign format)
int minisign_verify_full(const unsigned char *msg, size_t msg_len,
                          const unsigned char *sig, size_t sig_len,
                          const unsigned char *pk,
                          const char *trusted_comment, const char *untrusted_comment) {
    size_t tlen = trusted_comment ? strlen(trusted_comment) : 0;
    size_t ulen = untrusted_comment ? strlen(untrusted_comment) : 0;
    
    size_t full_len = msg_len + tlen + ulen + 2;
    unsigned char *full_msg = malloc(full_len);
    size_t pos = 0;
    memcpy(full_msg + pos, msg, msg_len); pos += msg_len;
    if (trusted_comment) { memcpy(full_msg + pos, trusted_comment, tlen); pos += tlen; }
    if (untrusted_comment) { memcpy(full_msg + pos, untrusted_comment, ulen); pos += ulen; }
    
    int ret = schnorr_verify(sig, sig_len, full_msg, full_len, pk);
    free(full_msg);
    return ret;
}
