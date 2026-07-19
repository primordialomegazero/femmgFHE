#include "dnscrypt_proxy.h"
#include "schnorr.h"
#include <string.h>
#include <openssl/sha.h>

int dnscrypt_keypair(unsigned char *pk, unsigned char *sk) {
    return schnorr_keypair(pk, sk);
}

int dnscrypt_sign_query(unsigned char *signed_query, size_t *sq_len,
                         const unsigned char *query, size_t q_len,
                         const unsigned char *sk) {
    if (!signed_query || !query || !sk) return -1;
    
    // Format: query || signature(65 bytes)
    memcpy(signed_query, query, q_len);
    size_t sig_len = 65;
    return schnorr_sign(query, q_len, sk, signed_query + q_len, &sig_len);
}

int dnscrypt_verify_response(const unsigned char *response, size_t r_len,
                              const unsigned char *signed_response,
                              const unsigned char *pk) {
    if (!response || !signed_response || !pk) return -1;
    return schnorr_verify(signed_response + r_len, 65, response, r_len, pk);
}

int dnscrypt_create_certificate(unsigned char *cert, size_t *cert_len,
                                 const unsigned char *pk,
                                 uint32_t serial, uint64_t ts_start, uint64_t ts_end) {
    // Simplified DNSCrypt certificate
    memcpy(cert, &serial, 4);
    memcpy(cert + 4, &ts_start, 8);
    memcpy(cert + 12, &ts_end, 8);
    memcpy(cert + 20, pk, 33);
    *cert_len = 53;
    return 0;
}
