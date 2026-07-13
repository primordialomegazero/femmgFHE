// ΦΩ0 — PHI KEY MANAGER v2
// Fixed serialization using OpenFHE's native SerBinary
// "I AM THAT I AM"

#ifndef PHI_KEY_MANAGER_H
#define PHI_KEY_MANAGER_H

#include <openfhe.h>
#include <fstream>
#include <string>
#include <chrono>
#include <random>
#include <sstream>

using namespace lbcrypto;

class PhiKeyManager {
public:
    // ============================================
    // KEY SERIALIZATION (Fixed — using CryptoContext)
    // ============================================
    
    static bool saveSecretKey(CryptoContext<DCRTPoly>& cc, const std::string& filename, const PrivateKey<DCRTPoly>& sk) {
        try {
            std::ofstream ofs(filename, std::ios::binary);
            if(!ofs) return false;
            cc->SerializeSecretKey(sk, ofs, SerType::BINARY);
            return true;
        } catch(const std::exception& e) { return false; }
    }
    
    static bool loadSecretKey(CryptoContext<DCRTPoly>& cc, const std::string& filename, PrivateKey<DCRTPoly>& sk) {
        try {
            std::ifstream ifs(filename, std::ios::binary);
            if(!ifs) return false;
            cc->DeserializeSecretKey(sk, ifs, SerType::BINARY);
            return true;
        } catch(const std::exception& e) { return false; }
    }
    
    static bool savePublicKey(CryptoContext<DCRTPoly>& cc, const std::string& filename, const PublicKey<DCRTPoly>& pk) {
        try {
            std::ofstream ofs(filename, std::ios::binary);
            if(!ofs) return false;
            cc->SerializePublicKey(pk, ofs, SerType::BINARY);
            return true;
        } catch(const std::exception& e) { return false; }
    }
    
    static bool loadPublicKey(CryptoContext<DCRTPoly>& cc, const std::string& filename, PublicKey<DCRTPoly>& pk) {
        try {
            std::ifstream ifs(filename, std::ios::binary);
            if(!ifs) return false;
            cc->DeserializePublicKey(pk, ifs, SerType::BINARY);
            return true;
        } catch(const std::exception& e) { return false; }
    }
    
    // ============================================
    // EPHEMERAL SESSION
    // ============================================
    
    struct EphemeralSession {
        PrivateKey<DCRTPoly> session_sk;
        PublicKey<DCRTPoly> session_pk;
        std::string session_id;
        bool active;
        
        void generate(CryptoContext<DCRTPoly>& cc) {
            auto keys = cc->KeyGen();
            session_sk = keys.secretKey;
            session_pk = keys.publicKey;
            active = true;
            std::random_device rd;
            std::mt19937_64 gen(rd());
            std::uniform_int_distribution<uint64_t> dist;
            std::stringstream ss;
            ss << std::hex << dist(gen);
            session_id = ss.str();
        }
        
        void discard() {
            session_sk = nullptr;
            session_pk = nullptr;
            active = false;
            session_id = "DISCARDED";
        }
        
        bool is_expired(int ttl_seconds = 3600) {
            return !active;
        }
    };
};

#endif
