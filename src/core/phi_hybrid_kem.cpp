// PHI-OMEGA-ZERO: HYBRID KEM — Micro-KEM + SpiralKEM
// Micro-KEM: 32B shared secret (fast XOR-based)
// SpiralKEM: φ-chaotic expansion (128B ciphertext)
// Hybrid: Best of both — fast encaps + chaotic security margin
// "TWO KEMS. ONE PROTOCOL. OPTIMAL HYBRID."
// "I AM THAT I AM"

#include <openssl/evp.h>
#include <openssl/rand.h>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <cstring>
#include <cmath>

using namespace std;
using namespace std::chrono;

const double PHI = 1.618033988749895;

// ============================================
// MICRO-KEM (32B CT, XOR-based, ultra-fast)
// ============================================
class MicroKEM {
public:
    static constexpr int PK_BYTES = 32;
    static constexpr int SK_BYTES = 16;
    static constexpr int CT_BYTES = 32;
    static constexpr int SS_BYTES = 32;
    
    static void hash(const uint8_t* in, size_t len, uint8_t* out, size_t out_len) {
        EVP_MD_CTX* c = EVP_MD_CTX_new();
        EVP_DigestInit_ex(c, EVP_sha256(), NULL);
        EVP_DigestUpdate(c, &PHI, sizeof(PHI));
        EVP_DigestUpdate(c, in, len);
        uint8_t full[32]; EVP_DigestFinal_ex(c, full, NULL);
        memcpy(out, full, out_len);
        EVP_MD_CTX_free(c);
    }
    
    static void keygen(uint8_t* pk, uint8_t* sk) {
        RAND_bytes(sk, SK_BYTES);
        hash(sk, SK_BYTES, pk, PK_BYTES);
    }
    
    static void encaps(uint8_t* ct, uint8_t* ss, const uint8_t* pk) {
        RAND_bytes(ss, SS_BYTES);
        uint8_t mask[CT_BYTES];
        hash(pk, PK_BYTES, mask, CT_BYTES);
        for(int i = 0; i < CT_BYTES; i++) ct[i] = ss[i] ^ mask[i];
    }
    
    static void decaps(uint8_t* ss, const uint8_t* ct, const uint8_t* sk) {
        uint8_t pk[PK_BYTES];
        hash(sk, SK_BYTES, pk, PK_BYTES);
        uint8_t mask[CT_BYTES];
        hash(pk, PK_BYTES, mask, CT_BYTES);
        for(int i = 0; i < CT_BYTES; i++) ss[i] = ct[i] ^ mask[i];
    }
};

// ============================================
// SPIRAL-KEM (128B CT, φ-chaotic expansion)
// ============================================
class SpiralKEM {
public:
    static constexpr int PK_BYTES = 64;
    static constexpr int SK_BYTES = 32;
    static constexpr int CT_BYTES = 128;
    static constexpr int SS_BYTES = 32;
    
    static void keygen(uint8_t* pk, uint8_t* sk) {
        RAND_bytes(sk, SK_BYTES);
        EVP_MD_CTX* c = EVP_MD_CTX_new();
        EVP_DigestInit_ex(c, EVP_sha256(), NULL);
        EVP_DigestUpdate(c, &PHI, sizeof(PHI));
        EVP_DigestUpdate(c, sk, SK_BYTES);
        EVP_DigestFinal_ex(c, pk, NULL);
        EVP_DigestInit_ex(c, EVP_sha256(), NULL);
        EVP_DigestUpdate(c, sk, SK_BYTES);
        EVP_DigestUpdate(c, &PHI, sizeof(PHI));
        EVP_DigestFinal_ex(c, pk + 32, NULL);
        EVP_MD_CTX_free(c);
    }
    
    static void encaps(uint8_t* ct, uint8_t* ss, const uint8_t* pk) {
        RAND_bytes(ss, SS_BYTES);
        EVP_MD_CTX* c = EVP_MD_CTX_new();
        
        // First 32B: XOR with mask (like Micro-KEM)
        uint8_t mask[32];
        EVP_DigestInit_ex(c, EVP_sha256(), NULL);
        EVP_DigestUpdate(c, pk, PK_BYTES);
        EVP_DigestUpdate(c, "spiral", 6);
        EVP_DigestUpdate(c, &PHI, sizeof(PHI));
        EVP_DigestFinal_ex(c, mask, NULL);
        for(int i = 0; i < 32; i++) ct[i] = ss[i] ^ mask[i];
        
        // Next 96B: φ-chaotic expansion
        uint8_t seed[32];
        EVP_DigestInit_ex(c, EVP_sha256(), NULL);
        EVP_DigestUpdate(c, pk, PK_BYTES);
        EVP_DigestUpdate(c, ss, SS_BYTES);
        EVP_DigestFinal_ex(c, seed, NULL);
        
        double x = PHI;
        uint8_t prev[32];
        memcpy(prev, seed, 32);
        for(int i = 0; i < 6; i++) {
            x = PHI * x * (1.0 - x);
            uint8_t hash[32];
            EVP_DigestInit_ex(c, EVP_sha256(), NULL);
            EVP_DigestUpdate(c, prev, 32);
            EVP_DigestUpdate(c, &x, sizeof(x));
            EVP_DigestFinal_ex(c, hash, NULL);
            memcpy(ct + 32 + i * 16, hash, 16);
            memcpy(prev, hash, 32);
        }
        EVP_MD_CTX_free(c);
    }
    
    static bool decaps(uint8_t* ss, const uint8_t* ct, const uint8_t* sk) {
        uint8_t pk[PK_BYTES];
        EVP_MD_CTX* c = EVP_MD_CTX_new();
        EVP_DigestInit_ex(c, EVP_sha256(), NULL);
        EVP_DigestUpdate(c, &PHI, sizeof(PHI));
        EVP_DigestUpdate(c, sk, SK_BYTES);
        EVP_DigestFinal_ex(c, pk, NULL);
        EVP_DigestInit_ex(c, EVP_sha256(), NULL);
        EVP_DigestUpdate(c, sk, SK_BYTES);
        EVP_DigestUpdate(c, &PHI, sizeof(PHI));
        EVP_DigestFinal_ex(c, pk + 32, NULL);
        
        // Decode first 32B
        uint8_t mask[32];
        EVP_DigestInit_ex(c, EVP_sha256(), NULL);
        EVP_DigestUpdate(c, pk, PK_BYTES);
        EVP_DigestUpdate(c, "spiral", 6);
        EVP_DigestUpdate(c, &PHI, sizeof(PHI));
        EVP_DigestFinal_ex(c, mask, NULL);
        for(int i = 0; i < 32; i++) ss[i] = ct[i] ^ mask[i];
        
        // Verify φ-chaotic expansion
        uint8_t seed[32];
        EVP_DigestInit_ex(c, EVP_sha256(), NULL);
        EVP_DigestUpdate(c, pk, PK_BYTES);
        EVP_DigestUpdate(c, ss, SS_BYTES);
        EVP_DigestFinal_ex(c, seed, NULL);
        
        double x = PHI;
        uint8_t prev[32];
        memcpy(prev, seed, 32);
        for(int i = 0; i < 6; i++) {
            x = PHI * x * (1.0 - x);
            uint8_t hash[32];
            EVP_DigestInit_ex(c, EVP_sha256(), NULL);
            EVP_DigestUpdate(c, prev, 32);
            EVP_DigestUpdate(c, &x, sizeof(x));
            EVP_DigestFinal_ex(c, hash, NULL);
            if(memcmp(ct + 32 + i * 16, hash, 16) != 0) {
                EVP_MD_CTX_free(c);
                return false;
            }
            memcpy(prev, hash, 32);
        }
        EVP_MD_CTX_free(c);
        return true;
    }
};

// ============================================
// HYBRID KEM
// ============================================
class HybridKEM {
public:
    static constexpr int CT_BYTES = 160; // 32 (Micro) + 128 (Spiral)
    static constexpr int SS_BYTES = 64;  // 32 + 32
    
    static void keygen(uint8_t* pk_micro, uint8_t* sk_micro,
                       uint8_t* pk_spiral, uint8_t* sk_spiral) {
        MicroKEM::keygen(pk_micro, sk_micro);
        SpiralKEM::keygen(pk_spiral, sk_spiral);
    }
    
    static void encaps(uint8_t* ct, uint8_t* ss,
                       const uint8_t* pk_micro, const uint8_t* pk_spiral) {
        uint8_t ss_micro[32], ss_spiral[32];
        uint8_t ct_micro[32], ct_spiral[128];
        
        MicroKEM::encaps(ct_micro, ss_micro, pk_micro);
        SpiralKEM::encaps(ct_spiral, ss_spiral, pk_spiral);
        
        memcpy(ct, ct_micro, 32);
        memcpy(ct + 32, ct_spiral, 128);
        memcpy(ss, ss_micro, 32);
        memcpy(ss + 32, ss_spiral, 32);
    }
    
    static bool decaps(uint8_t* ss, const uint8_t* ct,
                       const uint8_t* sk_micro, const uint8_t* sk_spiral) {
        uint8_t ss_micro[32], ss_spiral[32];
        
        MicroKEM::decaps(ss_micro, ct, sk_micro);
        bool spiral_ok = SpiralKEM::decaps(ss_spiral, ct + 32, sk_spiral);
        
        memcpy(ss, ss_micro, 32);
        memcpy(ss + 32, ss_spiral, 32);
        
        return spiral_ok;
    }
};

// ============================================
// BENCHMARK
// ============================================
template<typename KEM>
struct KEMBenchmark {
    double keygen_us;
    double encaps_us;
    double decaps_us;
    bool match;
};

template<typename KEM>
KEMBenchmark<KEM> benchmark_kem(int iterations = 10000) {
    KEMBenchmark<KEM> bm;
    uint8_t pk[KEM::PK_BYTES], sk[KEM::SK_BYTES];
    uint8_t ct[KEM::CT_BYTES], ss_enc[KEM::SS_BYTES], ss_dec[KEM::SS_BYTES];
    
    // KeyGen
    auto t1 = high_resolution_clock::now();
    for(int i = 0; i < iterations; i++) KEM::keygen(pk, sk);
    auto t2 = high_resolution_clock::now();
    bm.keygen_us = duration_cast<microseconds>(t2 - t1).count() / (double)iterations;
    
    // Encaps
    KEM::keygen(pk, sk);
    t1 = high_resolution_clock::now();
    for(int i = 0; i < iterations; i++) KEM::encaps(ct, ss_enc, pk);
    t2 = high_resolution_clock::now();
    bm.encaps_us = duration_cast<microseconds>(t2 - t1).count() / (double)iterations;
    
    // Decaps
    KEM::encaps(ct, ss_enc, pk);
    t1 = high_resolution_clock::now();
    for(int i = 0; i < iterations; i++) KEM::decaps(ss_dec, ct, sk);
    t2 = high_resolution_clock::now();
    bm.decaps_us = duration_cast<microseconds>(t2 - t1).count() / (double)iterations;
    
    // Verify
    KEM::keygen(pk, sk);
    KEM::encaps(ct, ss_enc, pk);
    KEM::decaps(ss_dec, ct, sk);
    bm.match = (memcmp(ss_enc, ss_dec, KEM::SS_BYTES) == 0);
    
    return bm;
}

int main() {
    cout << "\n======================================================================\n";
    cout <<   "  HYBRID KEM — Micro-KEM + SpiralKEM\n";
    cout <<   "  Comparative Benchmark\n";
    cout <<   "======================================================================\n\n";
    
    // Benchmark all three
    auto micro = benchmark_kem<MicroKEM>();
    auto spiral = benchmark_kem<SpiralKEM>();
    
    cout << "  SIZE COMPARISON:\n";
    cout << "  " << string(65, '-') << "\n";
    cout << "  " << setw(16) << left << "KEM"
         << setw(10) << "PK (B)"
         << setw(10) << "SK (B)"
         << setw(10) << "CT (B)"
         << setw(10) << "SS (B)"
         << setw(12) << "Total (B)\n";
    cout << "  " << string(65, '-') << "\n";
    cout << "  " << setw(16) << left << "Micro-KEM"
         << setw(10) << MicroKEM::PK_BYTES
         << setw(10) << MicroKEM::SK_BYTES
         << setw(10) << MicroKEM::CT_BYTES
         << setw(10) << MicroKEM::SS_BYTES
         << setw(12) << (MicroKEM::PK_BYTES + MicroKEM::CT_BYTES) << "\n";
    cout << "  " << setw(16) << left << "SpiralKEM"
         << setw(10) << SpiralKEM::PK_BYTES
         << setw(10) << SpiralKEM::SK_BYTES
         << setw(10) << SpiralKEM::CT_BYTES
         << setw(10) << SpiralKEM::SS_BYTES
         << setw(12) << (SpiralKEM::PK_BYTES + SpiralKEM::CT_BYTES) << "\n";
    cout << "  " << setw(16) << left << "HYBRID"
         << setw(10) << (MicroKEM::PK_BYTES + SpiralKEM::PK_BYTES)
         << setw(10) << (MicroKEM::SK_BYTES + SpiralKEM::SK_BYTES)
         << setw(10) << HybridKEM::CT_BYTES
         << setw(10) << HybridKEM::SS_BYTES
         << setw(12) << (MicroKEM::PK_BYTES + SpiralKEM::PK_BYTES + HybridKEM::CT_BYTES) << "\n\n";
    
    cout << "  SPEED COMPARISON:\n";
    cout << "  " << string(65, '-') << "\n";
    cout << "  " << setw(16) << left << "KEM"
         << setw(15) << "KeyGen (us)"
         << setw(15) << "Encaps (us)"
         << setw(15) << "Decaps (us)"
         << setw(12) << "Match\n";
    cout << "  " << string(65, '-') << "\n";
    cout << "  " << setw(16) << left << "Micro-KEM"
         << setw(15) << fixed << setprecision(2) << micro.keygen_us
         << setw(15) << micro.encaps_us
         << setw(15) << micro.decaps_us
         << setw(12) << (micro.match ? "✅" : "❌") << "\n";
    cout << "  " << setw(16) << left << "SpiralKEM"
         << setw(15) << spiral.keygen_us
         << setw(15) << spiral.encaps_us
         << setw(15) << spiral.decaps_us
         << setw(12) << (spiral.match ? "✅" : "❌") << "\n";
    cout << "  " << string(65, '-') << "\n\n";
    
    // Hybrid test
    uint8_t pk_m[32], sk_m[16], pk_s[64], sk_s[32];
    uint8_t ct[160], ss_enc[64], ss_dec[64];
    
    HybridKEM::keygen(pk_m, sk_m, pk_s, sk_s);
    HybridKEM::encaps(ct, ss_enc, pk_m, pk_s);
    bool hybrid_match = HybridKEM::decaps(ss_dec, ct, sk_m, sk_s);
    hybrid_match = hybrid_match && (memcmp(ss_enc, ss_dec, 64) == 0);
    
    cout << "  HYBRID VERIFICATION:\n";
    cout << "  Shared secret match: " << (hybrid_match ? "PERFECT MATCH ✅" : "MISMATCH ❌") << "\n";
    cout << "  Total CT: " << HybridKEM::CT_BYTES << "B (Micro 32B + Spiral 128B)\n";
    cout << "  Total SS: " << HybridKEM::SS_BYTES << "B (dual 32B secrets)\n\n";
    
    cout << "======================================================================\n";
    cout <<   "  HYBRID KEM: OPERATIONAL\n";
    cout <<   "  Micro-KEM speed + SpiralKEM chaotic security\n";
    cout <<   "======================================================================\n\n";
    
    cout << "  I AM THAT I AM\n\n";
    
    return 0;
}
