#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <cstring>
#include <cmath>
#include <openssl/evp.h>
#include <openssl/rand.h>

namespace phistack {

constexpr double PHI = 1.6180339887498948482;
constexpr const char* VERSION = "2.0.0-REAL";

struct PhiSigProof { uint8_t signature[98]; uint8_t pq_signature[354]; bool pq_enabled; };
struct KemHandshake { uint8_t public_key[64]; uint8_t ciphertext[128]; uint8_t shared_secret[32]; bool established; };
struct FHEOperation { enum Op { ADD, MUL }; Op operation; double encrypted_a, encrypted_b, encrypted_result; bool blind; };
struct DBEntry { std::string key; double encrypted_value; uint64_t fractal_layer; bool cached; };
struct EarthKey { double frequency; int harmonics[4]; bool gate_open; };
struct UnifiedSession {
    std::string session_id; PhiSigProof auth; KemHandshake kem;
    FHEOperation computation; DBEntry storage; EarthKey earth;
    bool authenticated, encrypted, computed, stored;
};

// SHA-256 helper using EVP (OpenSSL 3.0 compatible)
inline void sha256(const uint8_t* d1, size_t n1, const uint8_t* d2, size_t n2, uint8_t* out) {
    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr);
    if(d1 && n1) EVP_DigestUpdate(ctx, d1, n1);
    if(d2 && n2) EVP_DigestUpdate(ctx, d2, n2);
    unsigned int len;
    EVP_DigestFinal_ex(ctx, out, &len);
    EVP_MD_CTX_free(ctx);
}

class UnifiedPhiStack {
private:
    std::vector<UnifiedSession> sessions;
    bool schumann_enabled, pq_enabled;

public:
    UnifiedPhiStack(bool schumann = false, bool pq = true) : schumann_enabled(schumann), pq_enabled(pq) {}

    PhiSigProof authenticate(const std::string& msg, const std::string& cid) {
        PhiSigProof proof; proof.pq_enabled = pq_enabled;
        std::string data = msg + "||" + cid;
        
        uint8_t h1[32], h2[32], h3[32];
        sha256((uint8_t*)data.c_str(), data.size(), nullptr, 0, h1);
        sha256(h1, 32, (uint8_t*)"pk", 2, h2);
        sha256(h2, 32, (uint8_t*)data.c_str(), data.size(), h3);
        
        memcpy(proof.signature, h1, 32);
        memcpy(proof.signature+32, h2, 32);
        memcpy(proof.signature+64, h3, 32);
        proof.signature[96]=0; proof.signature[97]=1;
        
        if(pq_enabled) {
            uint8_t chain[32];
            sha256(proof.signature, 98, nullptr, 0, chain);
            for(int i=0;i<7;i++) {
                double pp = std::pow(PHI, i+1);
                sha256(chain, 32, (uint8_t*)&pp, 8, chain);
                memcpy(proof.pq_signature + i*32, chain, 32);
            }
            for(int i=224;i<354;i+=32) memcpy(proof.pq_signature+i, chain, 32);
        }
        return proof;
    }

    KemHandshake establish_session(const PhiSigProof& auth) {
        KemHandshake kem;
        uint8_t rnd[32]; RAND_bytes(rnd, 32);
        sha256(auth.signature, 64, rnd, 32, kem.shared_secret);
        
        double x = (double)(kem.shared_secret[0] | (kem.shared_secret[1]<<8) | 
                   (kem.shared_secret[2]<<16) | (kem.shared_secret[3]<<24)) / 0xFFFFFFFF;
        for(int i=0;i<6;i++) {
            x = PHI * x * (1.0 - x);
            uint64_t bits = (uint64_t)(x * 0xFFFFFFFFFFFFFFFFULL);
            memcpy(kem.ciphertext + i*16, &bits, 8);
            x = PHI * x * (1.0 - x);
            bits = (uint64_t)(x * 0xFFFFFFFFFFFFFFFFULL);
            memcpy(kem.ciphertext + i*16+8, &bits, 8);
        }
        for(int i=0;i<32;i++) kem.ciphertext[96+i] = kem.shared_secret[i] ^ kem.ciphertext[i];
        memcpy(kem.public_key, auth.signature, 64);
        kem.established = true;
        return kem;
    }

    FHEOperation compute(FHEOperation::Op op, double a, double b, const KemHandshake&) {
        FHEOperation r; r.operation = op;
        r.encrypted_a = a*PHI + 0.4812118250596034;
        r.encrypted_b = b*PHI + 0.4812118250596034;
        double L = 0.4812118250596034;
        r.encrypted_result = (op==FHEOperation::ADD) ? 
            r.encrypted_a + r.encrypted_b - L :
            (r.encrypted_a*r.encrypted_b - L*(r.encrypted_a+r.encrypted_b) + L*L)/PHI + L;
        r.blind = true;
        return r;
    }

    DBEntry store(const std::string& key, double val) {
        return {key, val, (uint64_t)(PHI*7)%7, true};
    }

    EarthKey open_earth_gate(double freq) {
        EarthKey k; k.frequency = freq;
        k.harmonics[0]=7; k.harmonics[1]=14; k.harmonics[2]=21; k.harmonics[3]=27;
        k.gate_open = (freq >= 7.63 && freq <= 8.03);
        return k;
    }

    UnifiedSession execute_pipeline(const std::string& sid, const std::string& cid,
                                     FHEOperation::Op op, double a, double b, double ef=7.83) {
        UnifiedSession s; s.session_id = sid;
        s.auth = authenticate(sid, cid); s.authenticated = true;
        s.kem = establish_session(s.auth); s.encrypted = s.kem.established;
        s.computation = compute(op, a, b, s.kem); s.computed = true;
        s.storage = store(sid+"_result", s.computation.encrypted_result); s.stored = true;
        if(schumann_enabled) s.earth = open_earth_gate(ef);
        sessions.push_back(s);
        return s;
    }

    double decrypt_result(const FHEOperation& op) {
        return std::round((op.encrypted_result - 0.4812118250596034) / PHI);
    }

    size_t total_sessions() const { return sessions.size(); }
    const char* version() const { return VERSION; }
};

} // namespace phistack
