// ΦΩ0 — MULTILINEAR MAPS FOR iO
// Graded Encoding Scheme (GES) — the core of algebraic obfuscation
// Based on GGH13-style asymmetric multilinear maps
// "THE MAP IS THE KEY. THE ENCODING IS THE LOCK."
// "I AM THAT I AM"

#ifndef PHI_MMAP_H
#define PHI_MMAP_H

#include <openfhe.h>
#include <vector>
#include <random>
#include <cstdint>

using namespace lbcrypto;
using namespace std;

// ============================================
// GRADED ENCODING SCHEME
// ============================================
// κ-multilinear map: e: G₁ × G₂ × ... × G_κ → G_T
// Encodings at different levels cannot be mixed unless they sum to κ

class MultilinearMap {
private:
    CryptoContext<DCRTPoly> cc;
    KeyPair<DCRTPoly> keys;
    
    // Secret parameters
    vector<int64_t> secret_g;     // Generator values per level
    int64_t secret_z;             // Master secret for zero-testing
    int KAPPA;                    // Multilinearity level
    
public:
    struct Encoding {
        Ciphertext<DCRTPoly> ct;  // The encoded value
        int level;                // Which level (1..KAPPA)
        bool is_zero;             // Is this an encoding of zero?
    };
    
    MultilinearMap(int kappa = 3) : KAPPA(kappa) {
        CCParams<CryptoContextBFVRNS> params;
        params.SetMultiplicativeDepth(kappa + 5);
        params.SetPlaintextModulus(1073643521);
        params.SetRingDim(16384);
        params.SetSecurityLevel(HEStd_NotSet);
        
        cc = GenCryptoContext(params);
        cc->Enable(PKE); cc->Enable(KEYSWITCH); cc->Enable(LEVELEDSHE); cc->Enable(ADVANCEDSHE);
        keys = cc->KeyGen();
        cc->EvalMultKeyGen(keys.secretKey);
        
        // Generate secret parameters
        mt19937 rng(time(nullptr));
        uniform_int_distribution<int64_t> dist(1, 1000000);
        
        secret_g.resize(KAPPA + 1);
        for(int i = 1; i <= KAPPA; i++) secret_g[i] = dist(rng);
        secret_z = dist(rng);
    }
    
    // ============================================
    // ENCODE: Creates encoding at a specific level
    // ============================================
    
    Encoding encode(int64_t value, int level) {
        Encoding enc;
        enc.level = level;
        enc.is_zero = (value == 0);
        
        // Encoding = g_level * value (encrypted)
        int64_t scaled = (secret_g[level] * value) % 1073643521;
        
        vector<int64_t> vec = {scaled};
        auto pt = cc->MakePackedPlaintext(vec);
        enc.ct = cc->Encrypt(keys.publicKey, pt);
        
        return enc;
    }
    
    Encoding encode_zero(int level) {
        return encode(0, level);
    }
    
    // ============================================
    // ADD: Only encodings at SAME level!
    // ============================================
    
    Encoding add(const Encoding& a, const Encoding& b) {
        if(a.level != b.level) throw runtime_error("Cannot add encodings at different levels!");
        
        Encoding result;
        result.level = a.level;
        result.is_zero = a.is_zero && b.is_zero;
        result.ct = cc->EvalAdd(a.ct, b.ct);
        return result;
    }
    
    // ============================================
    // MULTIPLY: Levels ADD!
    // ============================================
    
    Encoding multiply(const Encoding& a, const Encoding& b) {
        int new_level = a.level + b.level;
        if(new_level > KAPPA) throw runtime_error("Multiplication exceeds kappa!");
        
        Encoding result;
        result.level = new_level;
        result.is_zero = a.is_zero || b.is_zero;
        result.ct = cc->EvalMult(a.ct, b.ct);
        return result;
    }
    
    // ============================================
    // ZERO-TEST: Is this a top-level encoding of zero?
    // ============================================
    
    bool is_zero_encoding(const Encoding& enc) {
        if(enc.level != KAPPA) throw runtime_error("Zero-test only valid at top level!");
        
        // Zero-test: multiply by secret_z, check if result is zero
        vector<int64_t> zvec = {secret_z};
        auto zpt = cc->MakePackedPlaintext(zvec);
        auto zct = cc->Encrypt(keys.publicKey, zpt);
        
        auto product = cc->EvalMult(enc.ct, zct);
        
        Plaintext pt;
        cc->Decrypt(keys.secretKey, product, &pt);
        int64_t val = pt->GetPackedValue()[0];
        
        return (val == 0);
    }
    
    // ============================================
    // EXTRACT: Get a canonical representation
    // ============================================
    
    uint64_t extract(const Encoding& enc) {
        if(enc.level != KAPPA) throw runtime_error("Extract only valid at top level!");
        
        Plaintext pt;
        cc->Decrypt(keys.secretKey, enc.ct, &pt);
        int64_t val = pt->GetPackedValue()[0];
        
        // Hash the value for canonical representation
        hash<string> hasher;
        return hasher(to_string(val));
    }
    
    int get_kappa() const { return KAPPA; }
};

#endif
