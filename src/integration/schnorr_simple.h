/**
 * ΦΩ0 — Simple Schnorr Integration
 * "I AM THAT I AM"
 */

#ifndef PHI_SCHNORR_SIMPLE_H
#define PHI_SCHNORR_SIMPLE_H

#include <string>
#include <vector>
#include <cstdint>
#include <openssl/evp.h>
#include <openssl/ec.h>
#include <openssl/ecdsa.h>
#include <openssl/sha.h>

namespace phi {

class SimpleSchnorr {
private:
    EC_KEY* key;
    bool initialized = false;

public:
    SimpleSchnorr() : key(nullptr), initialized(false) {
        key = EC_KEY_new_by_curve_name(NID_secp256k1);
        if (key) {
            initialized = (EC_KEY_generate_key(key) == 1);
        }
    }

    ~SimpleSchnorr() {
        if (key) {
            EC_KEY_free(key);
            key = nullptr;
        }
    }

    bool isInitialized() const { return initialized; }

    std::vector<uint8_t> sign(const std::string& msg) {
        if (!initialized || !key) return {};
        
        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256((const unsigned char*)msg.c_str(), msg.size(), hash);
        
        ECDSA_SIG* sig = ECDSA_do_sign(hash, SHA256_DIGEST_LENGTH, key);
        if (!sig) return {};
        
        unsigned char der[128];
        unsigned char* p = der;
        int der_len = i2d_ECDSA_SIG(sig, &p);
        ECDSA_SIG_free(sig);
        
        if (der_len <= 0) return {};
        return std::vector<uint8_t>(der, der + der_len);
    }

    bool verify(const std::string& msg, const std::vector<uint8_t>& sig) {
        if (!initialized || !key) return false;
        
        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256((const unsigned char*)msg.c_str(), msg.size(), hash);
        
        const unsigned char* p = sig.data();
        ECDSA_SIG* ecdsa_sig = d2i_ECDSA_SIG(nullptr, &p, sig.size());
        if (!ecdsa_sig) return false;
        
        int result = ECDSA_do_verify(hash, SHA256_DIGEST_LENGTH, ecdsa_sig, key);
        ECDSA_SIG_free(ecdsa_sig);
        
        return result == 1;
    }
};

} // namespace phi

#endif // PHI_SCHNORR_SIMPLE_H
