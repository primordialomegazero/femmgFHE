#pragma once
#include "seal/seal.h"
#include <cmath>
#include <vector>
#include <iostream>
#include <iomanip>

namespace phi_fhe {

constexpr double PHI = 1.6180339887498948482;
constexpr double PHI_INV = 0.6180339887498948482;
constexpr double NOISE_FIX = 1.82815;

// === AGGRESSIVE ZANS ===
inline void apply_zans(seal::Ciphertext &ct, 
                        seal::Evaluator &evaluator,
                        seal::Encryptor &encryptor,
                        const seal::SEALContext &context,
                        int iterations = 10) {
    seal::Plaintext pt_zero("0");
    
    for (int i = 0; i < iterations; i++) {
        seal::Ciphertext ct_zero(context, ct.parms_id());
        encryptor.encrypt(pt_zero, ct_zero);
        evaluator.add_inplace(ct, ct_zero);
    }
}

// === PHI MULTIPLY with context ===
inline void phi_multiply(seal::Ciphertext &ct1,
                          const seal::Ciphertext &ct2,
                          seal::Evaluator &evaluator,
                          seal::Encryptor &encryptor,
                          seal::RelinKeys &relin_keys,
                          const seal::SEALContext &context,
                          int zans_before = 5,
                          int zans_after = 10) {
    apply_zans(ct1, evaluator, encryptor, context, zans_before);
    evaluator.multiply_inplace(ct1, ct2);
    evaluator.relinearize_inplace(ct1, relin_keys);
    apply_zans(ct1, evaluator, encryptor, context, zans_after);
}

// === PHI SQUARE ===
inline void phi_square(seal::Ciphertext &ct,
                        seal::Evaluator &evaluator,
                        seal::Encryptor &encryptor,
                        seal::RelinKeys &relin_keys,
                        const seal::SEALContext &context,
                        int zans_strength = 10) {
    seal::Ciphertext ct_copy = ct;
    phi_multiply(ct, ct_copy, evaluator, encryptor, relin_keys, context, zans_strength/2, zans_strength);
}

// === FIBONACCI-WEIGHTED ZANS: Apply ZANS with Fibonacci-spaced intensity ===
inline void fibonacci_zans(seal::Ciphertext &ct,
                            seal::Evaluator &evaluator,
                            seal::Encryptor &encryptor,
                            const seal::SEALContext &context,
                            seal::Decryptor &decryptor,
                            int steps = 8) {
    std::vector<int> fib = {1, 1, 2, 3, 5, 8, 13, 21, 34, 55};
    
    std::cout << "  Fib-ZANS: ";
    for (int i = 0; i < steps && i < (int)fib.size(); i++) {
        int before = decryptor.invariant_noise_budget(ct);
        apply_zans(ct, evaluator, encryptor, context, fib[i]);
        int after = decryptor.invariant_noise_budget(ct);
        std::cout << before << "→" << after;
        if (i < steps - 1) std::cout << " | ";
        if (after <= 0) { std::cout << " DEAD"; break; }
    }
    std::cout << "\n";
}

// === BANACH REGENERATION: Apply noise model, check convergence ===
inline double banach_predict(double noise, int k) {
    return NOISE_FIX + (noise - NOISE_FIX) * std::pow(PHI_INV, k);
}

inline void banach_analysis(int start_noise, int steps) {
    std::cout << "  Banach prediction (k=" << steps << "): ";
    double predicted = banach_predict(start_noise, steps);
    std::cout << "noise → " << std::fixed << std::setprecision(2) << predicted << "\n";
    std::cout << "  φ⁻¹ = " << PHI_INV << " | Fixed point = " << NOISE_FIX << "\n";
}

} // namespace phi_fhe
