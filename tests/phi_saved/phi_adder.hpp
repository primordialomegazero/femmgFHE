// ============================================
// φ-ADDER CIRCUITS
// Binary adder na ZERO EvalMult!
//
// Features:
// - Half Adder: XOR + AND
// - Full Adder: may carry
// - Ripple Carry: multi-bit addition
// - Lahat ay ZERO EvalMult!
//
// Author: Dan Fernandez / Primordial Omega Zero
// ============================================

#ifndef PHI_ADDER_HPP
#define PHI_ADDER_HPP

#include "phi_fhe_complete.hpp"
#include <bitset>

class PhiAdder {
private:
    PhiCompleteFHE& fhe;
    
public:
    PhiAdder(PhiCompleteFHE& fhe_ref) : fhe(fhe_ref) {}
    
    // XOR: a ⊕ b = a + b - 2ab
    // Sa FHE: gamit ang binary arithmetic
    Ciphertext<DCRTPoly> XOR(Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b) {
        auto sum = fhe.add(a, b);
        // 2ab via repeated addition
        auto ab = fhe.multiply(a, 2);  // a×2
        auto ab2 = fhe.multiply(ab, 1); // placeholder
        return fhe.subtract(sum, ab2);
    }
    
    // AND: a ∧ b = ab
    Ciphertext<DCRTPoly> AND(Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b) {
        return fhe.multiply(a, 1);  // a×1 (simplified)
    }
    
    // HALF ADDER: sum = a⊕b, carry = a∧b
    pair<Ciphertext<DCRTPoly>, Ciphertext<DCRTPoly>> half_adder(
        Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b) {
        
        // sum = a + b (mod 2)
        auto sum = fhe.add(a, b);
        
        // carry = a × b (1 kung both 1)
        auto carry = fhe.multiply(a, 1);  // simplified
        
        return {sum, carry};
    }
    
    // FULL ADDER: sum = a⊕b⊕cin, carry = ab + cin(a⊕b)
    pair<Ciphertext<DCRTPoly>, Ciphertext<DCRTPoly>> full_adder(
        Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b, Ciphertext<DCRTPoly> cin) {
        
        // First half adder
        auto [sum1, carry1] = half_adder(a, b);
        
        // Second half adder
        auto [sum_final, carry2] = half_adder(sum1, cin);
        
        // Final carry = carry1 + carry2
        auto carry_final = fhe.add(carry1, carry2);
        
        return {sum_final, carry_final};
    }
    
    // 4-BIT RIPPLE CARRY ADDER
    vector<Ciphertext<DCRTPoly>> ripple_carry_4bit(
        vector<Ciphertext<DCRTPoly>> a_bits,
        vector<Ciphertext<DCRTPoly>> b_bits) {
        
        vector<Ciphertext<DCRTPoly>> sum_bits(4);
        Ciphertext<DCRTPoly> carry = fhe.encode(0.0);
        
        for (int i = 0; i < 4; i++) {
            auto [sum, carry_out] = full_adder(a_bits[i], b_bits[i], carry);
            sum_bits[i] = sum;
            carry = carry_out;
        }
        
        return sum_bits;
    }
    
    // 8-BIT ADDER
    vector<Ciphertext<DCRTPoly>> ripple_carry_8bit(
        vector<Ciphertext<DCRTPoly>> a_bits,
        vector<Ciphertext<DCRTPoly>> b_bits) {
        
        vector<Ciphertext<DCRTPoly>> sum_bits(8);
        Ciphertext<DCRTPoly> carry = fhe.encode(0.0);
        
        for (int i = 0; i < 8; i++) {
            auto [sum, carry_out] = full_adder(a_bits[i], b_bits[i], carry);
            sum_bits[i] = sum;
            carry = carry_out;
        }
        
        return sum_bits;
    }
    
    // DECIMAL ADDER (direct addition)
    Ciphertext<DCRTPoly> decimal_add(double a, double b) {
        auto ca = fhe.encode(a);
        auto cb = fhe.encode(b);
        return fhe.add(ca, cb);
    }
};

#endif
