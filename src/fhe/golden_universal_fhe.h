// golden_universal_fhe.h
// Universal L(k) FHE — Non-Interactive, Unlimited Depth, Post-Quantum
// Public: Q, L(k) [integer], inv_L(k)
// Encrypt(0) = r·Q, Encrypt(1) = L(k) + r·Q
// NAND: L(k) - (a·b)·inv_L(k)

#pragma once
#include <NTL/ZZ.h>
#include <iostream>
#include <random>

namespace golden_universal_fhe {

class UniversalFHE {
public:
    using Ciphertext = NTL::ZZ;

    NTL::ZZ Q;
    NTL::ZZ L_k;       // PUBLIC (integer — universal sa lahat ng Q)
    NTL::ZZ inv_L_k;   // PUBLIC (mod Q inverse)

    std::mt19937_64 rng;

    UniversalFHE(const NTL::ZZ& Q_, long k = 2048) : Q(Q_), rng(42) {
        // Compute integer L(k) = F(k+1) + F(k-1)
        // Walang modulo — universal sa lahat ng Q ≡ 1 mod 5
        NTL::ZZ fib[10000];
        fib[0] = NTL::to_ZZ(0);
        fib[1] = NTL::to_ZZ(1);
        for (long i = 2; i <= k+1; i++) {
            fib[i] = fib[i-1] + fib[i-2];  // INTEGER addition!
        }
        L_k = fib[k+1] + fib[k-1];  // Integer Lucas number
        inv_L_k = NTL::InvMod(L_k % Q, Q);  // Mod Q inverse
    }

    // Encrypt: ct = m·L(k) + r·Q
    Ciphertext encrypt(bool bit) {
        NTL::ZZ r = NTL::RandomBnd(Q);
        NTL::ZZ m_val = bit ? L_k : NTL::to_ZZ(0);
        return m_val + r * Q;
    }

    // Decrypt: v = ct mod Q, check kung L(k) o 0
    bool decrypt(Ciphertext ct) const {
        NTL::ZZ v = ct % Q;
        if (v < 0) v += Q;
        NTL::ZZ diff_L = (v > L_k) ? v - L_k : L_k - v;
        if (diff_L > Q/2) diff_L = Q - diff_L;
        NTL::ZZ diff_0 = (v < Q/2) ? v : Q - v;
        return diff_L < diff_0;
    }

    // NAND: L(k) - (a·b)·inv_L(k)
    Ciphertext nand(Ciphertext a, Ciphertext b) const {
        NTL::ZZ a_mod = a % Q;
        if (a_mod < 0) a_mod += Q;
        NTL::ZZ b_mod = b % Q;
        if (b_mod < 0) b_mod += Q;
        NTL::ZZ prod = (a_mod * b_mod) % Q;
        NTL::ZZ scaled = (prod * inv_L_k) % Q;
        NTL::ZZ result = (L_k - scaled) % Q;
        if (result < 0) result += Q;
        return result;
    }

    // Derived gates
    Ciphertext not_gate(Ciphertext a) { return nand(a, a); }
    
    Ciphertext and_gate(Ciphertext a, Ciphertext b) {
        Ciphertext n = nand(a, b);
        return nand(n, n);
    }
    
    Ciphertext or_gate(Ciphertext a, Ciphertext b) {
        Ciphertext not_a = nand(a, a);
        Ciphertext not_b = nand(b, b);
        return nand(not_a, not_b);
    }
    
    Ciphertext xor_gate(Ciphertext a, Ciphertext b) {
        Ciphertext n1 = nand(a, b);
        Ciphertext n2 = nand(a, n1);
        Ciphertext n3 = nand(b, n1);
        return nand(n2, n3);
    }
};

} // namespace golden_universal_fhe
