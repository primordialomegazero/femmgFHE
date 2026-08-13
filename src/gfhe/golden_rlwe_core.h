#pragma once

#include <NTL/ZZ_pX.h>
#include <NTL/ZZ_p.h>
#include <NTL/ZZ_pXFactoring.h>
#include <cmath>
#include <vector>
#include <random>
#include <stdexcept>

namespace GoldenRLWE {

constexpr int N = 2048;
constexpr long Q = 2147483647;
constexpr int BASE_BITS = 2; // φ²+ψ²=3 ~ 2 bits
constexpr long BASE = 3; // φ²+ψ²
constexpr int NUM_LIMBS = 32;

inline void init_ring() { NTL::ZZ_p::init(NTL::ZZ(Q)); }

class GoldenPRNG {
private:
    double state;
public:
    GoldenPRNG(double seed = 42.0) : state(std::fmod(std::abs(seed), 1.0)) {}
    double next_double() {
        double a = state, b = std::fmod(state + 1.6180339887498948482, 1.0);
        for (int i = 0; i < 8; i++) { double t = std::fmod((a+b)*1.6180339887498948482,1.0); a=b; b=t; }
        state = b; return state;
    }
    long next_long(long mod) { return static_cast<long>(next_double()*mod)%mod; }
    long small_coef() { double v=next_double(); if(v<0.2) return -1; if(v<0.4) return 1; return 0; }
};

inline NTL::ZZ_pX poly_from_vector(const std::vector<long>& coefs) {
    NTL::ZZ_pX p; for (size_t i=0;i<coefs.size();++i) NTL::SetCoeff(p,i,coefs[i]); return p;
}
inline NTL::ZZ_pX random_small_poly(GoldenPRNG& prng) {
    std::vector<long> coefs(N,0); for(int i=0;i<N;++i) coefs[i]=prng.small_coef(); return poly_from_vector(coefs);
}
inline NTL::ZZ_pX golden_secret_key(double master_seed) {
    GoldenPRNG prng(master_seed); std::vector<long> coefs(N,0); for(int i=0;i<N;++i) coefs[i]=prng.small_coef(); return poly_from_vector(coefs);
}

struct PublicKey { NTL::ZZ_pX p0,p1; };

struct RelinKey {
    NTL::ZZ_pX r0[NUM_LIMBS];
    NTL::ZZ_pX r1[NUM_LIMBS];
};

inline PublicKey golden_keygen(const NTL::ZZ_pX& sk, double seed) {
    GoldenPRNG prng(seed);
    std::vector<long> a_coefs(N,0); for(int i=0;i<N;++i) a_coefs[i]=prng.next_long(Q);
    NTL::ZZ_pX a = poly_from_vector(a_coefs);
    NTL::ZZ_pX e = random_small_poly(prng);
    PublicKey pk; pk.p0 = -(a*sk + e); pk.p1 = a; return pk;
}

inline RelinKey golden_relin_keygen(const NTL::ZZ_pX& sk, double seed) {
    RelinKey rk;
    for (int j = 0; j < NUM_LIMBS; j++) {
        GoldenPRNG prng(seed + j * 1000.0);
        std::vector<long> a_coefs(N,0);
        for(int i=0;i<N;++i) a_coefs[i]=prng.next_long(Q);
        NTL::ZZ_pX a = poly_from_vector(a_coefs);
        NTL::ZZ_pX e = random_small_poly(prng);

        // r0 = -(a*s + e) + (BASE^j) * s^2
        NTL::ZZ_pX s2 = sk * sk;
        for (int i = 0; i < N; i++) {
            long coeff = NTL::conv<long>(NTL::coeff(s2, i));
            NTL::SetCoeff(s2, i, coeff * (BASE % Q));
        }
        rk.r0[j] = -(a*sk + e) + s2;
        rk.r1[j] = a;
    }
    return rk;
}

struct Ciphertext { NTL::ZZ_pX c0,c1,c2; };

inline Ciphertext encrypt(const PublicKey& pk, const NTL::ZZ_pX& msg, double seed) {
    GoldenPRNG prng(seed);
    NTL::ZZ_pX u = random_small_poly(prng);
    NTL::ZZ_pX e0 = random_small_poly(prng);
    NTL::ZZ_pX e1 = random_small_poly(prng);

    NTL::ZZ_pX scaled_msg;
    for (int i=0;i<N;i++) {
        long m = NTL::conv<long>(NTL::coeff(msg,i));
        long encoded = (m==0) ? (Q - Q/4) : (Q/4);
        NTL::SetCoeff(scaled_msg,i,encoded);
    }

    Ciphertext ct;
    ct.c0 = pk.p0*u + e0 + scaled_msg;
    ct.c1 = pk.p1*u + e1;
    ct.c2 = NTL::ZZ_pX();
    return ct;
}

inline Ciphertext add(const Ciphertext& a, const Ciphertext& b) {
    Ciphertext r; r.c0=a.c0+b.c0; r.c1=a.c1+b.c1; r.c2=a.c2+b.c2; return r;
}

inline Ciphertext mul(const Ciphertext& a, const Ciphertext& b) {
    Ciphertext r;
    r.c0 = a.c0*b.c0;
    r.c1 = a.c0*b.c1 + a.c1*b.c0;
    r.c2 = a.c1*b.c1;
    return r;
}

inline Ciphertext relinearize(const Ciphertext& ct, const RelinKey& rk) {
    Ciphertext r = ct;
    // decompose ct.c2 into limbs
    NTL::ZZ_pX c2 = ct.c2;
    std::vector<NTL::ZZ_pX> limbs(NUM_LIMBS);
    for (int j = 0; j < NUM_LIMBS; j++) {
        for (int i = 0; i < N; i++) {
            long coeff = NTL::conv<long>(NTL::coeff(c2, i));
            long limb = coeff & (BASE-1);
            NTL::SetCoeff(limbs[j], i, limb);
            coeff >>= BASE_BITS;
            NTL::SetCoeff(c2, i, coeff);
        }
        r.c0 = r.c0 + limbs[j] * rk.r0[j];
        r.c1 = r.c1 + limbs[j] * rk.r1[j];
    }
    r.c2 = NTL::ZZ_pX();
    return r;
}

inline NTL::ZZ_pX decrypt(const Ciphertext& ct, const NTL::ZZ_pX& sk) {
    NTL::ZZ_pX noise = ct.c0 + ct.c1*sk + ct.c2*sk*sk;
    NTL::ZZ_pX msg;
    long half = Q/2;
    long quarter = Q/4;
    for (int i=0;i<N;i++) {
        long v = NTL::conv<long>(NTL::coeff(noise,i));
        long centered = (v % Q + Q) % Q;
        if (centered >= half) centered -= Q;
        long abs_val = std::abs(centered);
        if (abs_val < quarter) NTL::SetCoeff(msg,i,1);
        else NTL::SetCoeff(msg,i,0);
    }
    return msg;
}

} // namespace GoldenRLWE
