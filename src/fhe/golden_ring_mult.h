#pragma once
#include <NTL/ZZ_pX.h>
#include <NTL/ZZ_p.h>
#include <vector>

namespace GoldenRingMult {

constexpr int N = 1024;
constexpr long Q = 536870909;

inline void init_ring() { NTL::ZZ_p::init(NTL::ZZ(Q)); }

// Tamang multiplication sa ring Z_Q[X]/(X^N + 1)
// Ang reduction: X^N = -1, kaya X^(N+i) = -X^i
inline void reduce_mod_cyclotomic(NTL::ZZ_pX& poly) {
    init_ring();
    
    // I-convert sa vector para sa manual reduction
    std::vector<NTL::ZZ_p> coeffs(N);
    for (int i = 0; i < N; i++) {
        coeffs[i] = NTL::coeff(poly, i);
    }
    
    // Para sa degree >= N, i-reduce gamit ang X^N = -1
    long deg = NTL::deg(poly);
    for (long i = N; i <= deg; i++) {
        NTL::ZZ_p coeff = NTL::coeff(poly, i);
        // X^i = X^(i-N) * X^N = -X^(i-N)
        long target = i - N;
        coeffs[target] -= coeff;
    }
    
    // I-reconstruct ang polynomial
    poly = NTL::ZZ_pX();
    for (int i = 0; i < N; i++) {
        NTL::SetCoeff(poly, i, coeffs[i]);
    }
}

// Tamang multiplication sa ring
inline NTL::ZZ_pX ring_multiply(const NTL::ZZ_pX& a, const NTL::ZZ_pX& b) {
    init_ring();
    
    NTL::ZZ_pX result = a * b;
    reduce_mod_cyclotomic(result);
    return result;
}

// Tamang homomorphic multiplication para sa FHE
inline void homomorphic_multiply(const NTL::ZZ_pX& a0, const NTL::ZZ_pX& a1,
                                  const NTL::ZZ_pX& b0, const NTL::ZZ_pX& b1,
                                  NTL::ZZ_pX& c0, NTL::ZZ_pX& c1) {
    init_ring();
    
    // (a0 + a1*s) * (b0 + b1*s)
    // = a0*b0 + (a0*b1 + a1*b0)*s + a1*b1*s^2
    // Sa ring X^N + 1, s^2 = -1 (approx)
    // Pero para sa tamang computation:
    // c0 = a0*b0 - a1*b1 (since s^2 = -1)
    // c1 = a0*b1 + a1*b0
    
    NTL::ZZ_pX t0 = ring_multiply(a0, b0);
    NTL::ZZ_pX t1 = ring_multiply(a0, b1);
    NTL::ZZ_pX t2 = ring_multiply(a1, b0);
    NTL::ZZ_pX t3 = ring_multiply(a1, b1);
    
    c0 = t0 - t3;  // a0*b0 - a1*b1
    c1 = t1 + t2;  // a0*b1 + a1*b0
}

} // namespace GoldenRingMult
