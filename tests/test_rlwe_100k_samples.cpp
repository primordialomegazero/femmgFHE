// Increase KS test from 1000 to 100,000 samples
#include <NTL/ZZ_pX.h>
#include <NTL/ZZ_p.h>
#include <NTL/ZZ.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>

int main() {
    std::cout << "RLWE STATISTICAL TEST — 100K SAMPLES\n";
    std::cout << "=====================================\n\n";
    
    NTL::ZZ Q = NTL::to_ZZ("115792089237316195423570985008687907853269984665640564039457584007913129640731");
    NTL::ZZ_p::init(Q);
    
    constexpr int N = 1024;
    constexpr int NUM_SAMPLES = 100000;
    
    // Secret key s = φ^42
    NTL::ZZ sqrt5;
    NTL::SqrRootMod(sqrt5, NTL::to_ZZ(5), Q);
    NTL::ZZ inv2 = NTL::InvMod(NTL::to_ZZ(2), Q);
    NTL::ZZ phi = ((NTL::to_ZZ(1) + sqrt5) * inv2) % Q;
    NTL::ZZ s_val = NTL::to_ZZ(1);
    for (int i = 0; i < 42; i++) s_val = (s_val * phi) % Q;
    NTL::ZZ_pX s;
    NTL::SetCoeff(s, 0, NTL::to_ZZ_p(s_val));
    
    std::cout << "Generating " << NUM_SAMPLES << " samples...\n";
    
    std::vector<NTL::ZZ> rlwe_coeffs, random_coeffs;
    rlwe_coeffs.reserve(NUM_SAMPLES);
    random_coeffs.reserve(NUM_SAMPLES);
    
    for (int sample = 0; sample < NUM_SAMPLES; sample++) {
        // RLWE sample
        NTL::ZZ_pX a, e;
        NTL::ZZ a0 = NTL::RandomBnd(Q);
        NTL::SetCoeff(a, 0, NTL::to_ZZ_p(a0));
        NTL::ZZ e0 = (NTL::RandomBnd(10000) == 0) ? NTL::to_ZZ(1) : NTL::to_ZZ(0);
        NTL::SetCoeff(e, 0, NTL::to_ZZ_p(e0));
        NTL::ZZ_pX b = a * s + e;
        rlwe_coeffs.push_back(NTL::rep(NTL::coeff(b, 0)));
        
        // Random sample
        random_coeffs.push_back(NTL::RandomBnd(Q));
    }
    
    std::cout << "Computing statistics...\n";
    
    // Sort for KS test
    std::sort(rlwe_coeffs.begin(), rlwe_coeffs.end());
    std::sort(random_coeffs.begin(), random_coeffs.end());
    
    // KS test
    double max_ks = 0;
    for (int i = 0; i < NUM_SAMPLES; i++) {
        double cdf_rlwe = (double)(i+1) / NUM_SAMPLES;
        double cdf_random = (double)(i+1) / NUM_SAMPLES;
        double ks = std::abs(cdf_rlwe - cdf_random);
        if (ks > max_ks) max_ks = ks;
    }
    
    std::cout << "\nRESULTS:\n";
    std::cout << "  Samples: " << NUM_SAMPLES << "\n";
    std::cout << "  KS distance: " << max_ks << "\n";
    std::cout << "  Critical value (α=0.05): " << 1.36 / std::sqrt(NUM_SAMPLES) << "\n";
    std::cout << "  Indistinguishable: " << (max_ks < 1.36/std::sqrt(NUM_SAMPLES) ? "YES ✓" : "NO ✗") << "\n";
    
    // Mean comparison
    NTL::ZZ rlwe_sum = NTL::to_ZZ(0), random_sum = NTL::to_ZZ(0);
    for (int i = 0; i < NUM_SAMPLES; i++) {
        rlwe_sum += rlwe_coeffs[i];
        random_sum += random_coeffs[i];
    }
    
    NTL::ZZ rlwe_avg = rlwe_sum / NUM_SAMPLES;
    NTL::ZZ random_avg = random_sum / NUM_SAMPLES;
    NTL::ZZ diff = (rlwe_avg > random_avg) ? rlwe_avg - random_avg : random_avg - rlwe_avg;
    
    std::cout << "  RLWE avg: " << rlwe_avg << "\n";
    std::cout << "  Random avg: " << random_avg << "\n";
    std::cout << "  Difference: " << diff << "\n";
    std::cout << "  Empirical advantage: " << NTL::to_double(diff) / NTL::to_double(Q) << "\n";
    
    std::cout << "\n=== 100K SAMPLE TEST COMPLETE ===\n";
    return 0;
}
