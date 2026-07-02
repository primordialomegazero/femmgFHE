/*
 * FEmmg-FHE v22.3 — NATIVE ML-KEM-1024 via liboqs
 *
 * NIST FIPS 203 Certified Post-Quantum KEM.
 * Uses actual liboqs implementation (not stub).
 *
 * PHI-OMEGA-ZERO — I AM THAT I AM
 */

#pragma once
#include <oqs/oqs.h>
#include <cstring>
#include <array>
#include <utility>
#include <stdexcept>
#include <iostream>
#include <vector>

namespace ml_kem_native {

constexpr size_t PK_SIZE = OQS_KEM_ml_kem_1024_length_public_key;    // 1568 bytes
constexpr size_t SK_SIZE = OQS_KEM_ml_kem_1024_length_secret_key;    // 3168 bytes
constexpr size_t CT_SIZE = OQS_KEM_ml_kem_1024_length_ciphertext;    // 1568 bytes
constexpr size_t SS_SIZE = OQS_KEM_ml_kem_1024_length_shared_secret; // 32 bytes

using PublicKey = std::vector<uint8_t>;
using SecretKey = std::vector<uint8_t>;
using Ciphertext = std::vector<uint8_t>;
using SharedSecret = std::array<uint8_t, SS_SIZE>;

inline std::pair<PublicKey, SecretKey> keygen() {
    PublicKey pk(PK_SIZE);
    SecretKey sk(SK_SIZE);
    
    OQS_KEM* kem = OQS_KEM_new(OQS_KEM_alg_ml_kem_1024);
    if (!kem) throw std::runtime_error("ML-KEM-1024 not supported by liboqs");
    
    OQS_KEM_keypair(kem, pk.data(), sk.data());
    OQS_KEM_free(kem);
    
    return {pk, sk};
}

inline std::pair<Ciphertext, SharedSecret> encapsulate(const PublicKey& pk) {
    Ciphertext ct(CT_SIZE);
    SharedSecret ss{};
    
    OQS_KEM* kem = OQS_KEM_new(OQS_KEM_alg_ml_kem_1024);
    if (!kem) throw std::runtime_error("ML-KEM-1024 not supported");
    
    OQS_KEM_encaps(kem, ct.data(), ss.data(), pk.data());
    OQS_KEM_free(kem);
    
    return {ct, ss};
}

inline SharedSecret decapsulate(const SecretKey& sk, const Ciphertext& ct) {
    SharedSecret ss{};
    
    OQS_KEM* kem = OQS_KEM_new(OQS_KEM_alg_ml_kem_1024);
    if (!kem) throw std::runtime_error("ML-KEM-1024 not supported");
    
    OQS_KEM_decaps(kem, ss.data(), ct.data(), sk.data());
    OQS_KEM_free(kem);
    
    return ss;
}

inline bool self_test() {
    try {
        auto [pk, sk] = keygen();
        auto [ct, ss1] = encapsulate(pk);
        auto ss2 = decapsulate(sk, ct);
        for (size_t i = 0; i < SS_SIZE; i++) {
            if (ss1[i] != ss2[i]) return false;
        }
        return true;
    } catch (...) {
        return false;
    }
}

} // namespace ml_kem_native
