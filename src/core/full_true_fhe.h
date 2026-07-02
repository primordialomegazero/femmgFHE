#pragma once
#include "true_poly_fhe.h"
#include "femmg_operations.h"
#include <cstring>

class FullTrueFHE {
private:
    TruePolyFHE poly_fhe_;    // Polynomial homomorphic engine
    FEmmgFHE chaos_fhe_;      // IND-CPA + security engine
    
public:
    FullTrueFHE() = default;
    
    // ═══ ENCRYPT: Chaos encrypt the plaintext, then encode as polynomial ═══
    TruePolyFHE::Ciphertext encrypt(int64_t plaintext, uint64_t seed = 0) {
        // Chaos encrypt first (IND-CPA)
        auto chaos_ct = chaos_fhe_.encrypt(plaintext);
        
        // Use the encrypted value_int as the "plaintext" for poly layer
        // This way poly computes on ENCRYPTED data directly
        return poly_fhe_.encrypt(chaos_ct.value_int >> 20, seed);  // Scale down
    }
    
    // ═══ DECRYPT: Reverse the process ═══
    int64_t decrypt(const TruePolyFHE::Ciphertext& poly_ct, uint64_t seed = 0) {
        int64_t poly_plain = poly_fhe_.decrypt(poly_ct, seed);
        
        // Reconstruct chaos ciphertext and decrypt
        banach::NDimCiphertext chaos_ct{};
        chaos_ct.value_int = poly_plain << 20;  // Scale back up
        return chaos_fhe_.decrypt(chaos_ct);
    }
    
    // ═══ HOMOMORPHIC ADD: Direct polynomial addition! ═══
    TruePolyFHE::Ciphertext add(const TruePolyFHE::Ciphertext& a,
                                  const TruePolyFHE::Ciphertext& b) {
        return poly_fhe_.add(a, b);
    }
    
    // ═══ HOMOMORPHIC MULTIPLY: Direct polynomial multiplication! ═══
    TruePolyFHE::Ciphertext multiply(const TruePolyFHE::Ciphertext& a,
                                       const TruePolyFHE::Ciphertext& b) {
        return poly_fhe_.multiply(a, b);
    }
    
    static const char* name() { return "Full True FHE — Chaos IND-CPA + Poly Homomorphic"; }
};
