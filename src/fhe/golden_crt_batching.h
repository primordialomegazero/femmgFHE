#pragma once
#include "golden_quantum_fhe.h"
#include <vector>
#include <array>

namespace GoldenCRTBatching {

using namespace GoldenFHE;

// CRT Batching: I-encode ang maraming plaintexts sa isang ciphertext
// Para sa N=1024 cyclotomic ring, kaya nating mag-encode ng hanggang 1024 values

class CRTBatcher {
private:
    // Precomputed CRT factors
    std::vector<long> crt_primes;
    
public:
    CRTBatcher() {
        // Maghanap ng primes na may root of unity para sa N=1024
        // Para sa simpleng version: gamitin ang Q bilang single prime
        
        // Sa totoong CRT batching, kailangan natin ng multiple primes
        // na ang product ay mas malaki sa Q
        
        crt_primes.push_back(536870909);  // Q mismo
        // Add more primes para sa batching
        crt_primes.push_back(536870923);
        crt_primes.push_back(536870953);
    }
    
    // I-encode ang maraming bits sa isang ciphertext
    // Bawat bit ay naka-encode sa iba't ibang CRT slot
    Cipher encode_batch(const std::vector<bool>& bits, const PublicKey& pk, uint64_t nonce) {
        init_ring();
        
        NTL::ZZ_pX combined;
        
        // I-encode ang bawat bit sa iba't ibang coefficient
        for (size_t i = 0; i < bits.size() && i < N; i++) {
            if (bits[i]) {
                long golden_plain = static_cast<long>(Q / PHI);
                NTL::SetCoeff(combined, i, golden_plain);
            }
        }
        
        // I-encrypt ang combined polynomial
        uint64_t state = nonce;
        NTL::ZZ_pX u, e0, e1;
        for (int i = 0; i < N; i++) {
            state ^= (state << 13);
            state ^= (state >> 7);
            state ^= (state << 17);
            NTL::SetCoeff(u, i, (state % 3) - 1);
            NTL::SetCoeff(e0, i, (state % 10000) == 0 ? 1 : 0);
            NTL::SetCoeff(e1, i, (state % 10000) == 0 ? 1 : 0);
        }
        
        Cipher ct;
        ct.c0 = pk.pk0 * u + e0 + combined;
        ct.c1 = pk.pk1 * u + e1;
        ct.c2 = NTL::ZZ_pX();
        
        return ct;
    }
    
    // I-decode ang batch mula sa ciphertext
    std::vector<bool> decode_batch(const Cipher& ct, const SecretKey& sk, int num_bits) {
        init_ring();
        
        NTL::ZZ_pX s = sk.sk;
        NTL::ZZ_pX s2 = s * s;
        NTL::ZZ_pX noise = ct.c0 + ct.c1 * s + ct.c2 * s2;
        
        std::vector<bool> bits;
        long threshold = static_cast<long>(Q / (2 * PHI));
        
        for (int i = 0; i < num_bits && i < N; i++) {
            long v = NTL::conv<long>(NTL::coeff(noise, i));
            bits.push_back(v > threshold);
        }
        
        return bits;
    }
    
    // Batch NOT: i-flip lahat ng bits sa batch
    Cipher batch_not(const Cipher& ct, const PublicKey& pk, uint64_t nonce) {
        init_ring();
        
        // NOT sa bawat slot: 1 - value
        NTL::ZZ_pX golden_poly;
        long golden_plain = static_cast<long>(Q / PHI);
        
        for (int i = 0; i < N; i++) {
            NTL::SetCoeff(golden_poly, i, golden_plain);
        }
        
        Cipher result;
        result.c0 = golden_poly - ct.c0;
        result.c1 = -ct.c1;
        result.c2 = -ct.c2;
        
        return result;
    }
};

// Test CRT batching
inline bool test_crt_batching() {
    init_ring();
    
    PublicKey pk;
    SecretKey sk;
    keygen(pk, sk, 42);
    
    CRTBatcher batcher;
    
    // I-encode ang 8 bits
    std::vector<bool> input = {true, false, true, false, true, false, true, false};
    Cipher encoded = batcher.encode_batch(input, pk, 1000000);
    
    // I-decode
    std::vector<bool> decoded = batcher.decode_batch(encoded, sk, 8);
    
    // I-verify
    for (int i = 0; i < 8; i++) {
        if (decoded[i] != input[i]) {
            return false;
        }
    }
    
    return true;
}

} // namespace GoldenCRTBatching
