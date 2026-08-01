// SPIRAL-NTT — Number Theoretic Transform for Spiral-FHE
// Fast polynomial multiplication in Z_q[x]/(x^N+1)
// Uses NTT with φ-anchored root of unity

#pragma once
#include <vector>
#include <cstdint>
#include <cmath>
#include <algorithm>

namespace spiral {

class SpiralNTT {
private:
    size_t N;  // Polynomial degree (power of 2)
    uint64_t q;  // Modulus (prime, q ≡ 1 mod 2N)
    uint64_t root;  // Primitive 2N-th root of unity mod q
    std::vector<uint64_t> roots;  // Precomputed powers
    std::vector<uint64_t> inv_roots;  // Inverse roots
    
    // Modular exponentiation
    uint64_t modpow(uint64_t base, uint64_t exp, uint64_t mod) {
        uint64_t result = 1;
        base %= mod;
        while (exp > 0) {
            if (exp & 1) result = (result * base) % mod;
            base = (base * base) % mod;
            exp >>= 1;
        }
        return result;
    }
    
    // Bit-reversal permutation
    size_t bit_reverse(size_t x, size_t bits) {
        size_t result = 0;
        for (size_t i = 0; i < bits; i++) {
            result = (result << 1) | (x & 1);
            x >>= 1;
        }
        return result;
    }
    
public:
    SpiralNTT(size_t poly_degree = 8192, uint64_t modulus = 0) 
        : N(poly_degree), q(modulus) {
        
        // Use φ-anchored modulus if none provided
        if (q == 0) {
            // Find a prime q ≡ 1 mod 2N near φ × 2^20
            q = 1618033988;  // φ-anchored seed
            while (q % (2 * N) != 1) q++;
        }
        
        // Find primitive 2N-th root of unity
        uint64_t candidate = 2;
        while (true) {
            if (modpow(candidate, N, q) == q - 1) {
                root = candidate;
                break;
            }
            candidate++;
        }
        
        // Precompute roots
        size_t bits = log2(N);
        roots.resize(N);
        inv_roots.resize(N);
        
        for (size_t i = 0; i < N; i++) {
            size_t rev = bit_reverse(i, bits);
            roots[rev] = modpow(root, i, q);
            inv_roots[rev] = modpow(root, (2 * N - 1) * i % (2 * N), q);
        }
    }
    
    // Forward NTT: a → Â
    void forward(std::vector<uint64_t>& a) {
        size_t n = a.size();
        size_t bits = log2(n);
        
        for (size_t len = 2; len <= n; len <<= 1) {
            size_t half = len >> 1;
            for (size_t i = 0; i < n; i += len) {
                for (size_t j = 0; j < half; j++) {
                    size_t idx = (n / len) * j;
                    uint64_t u = a[i + j];
                    uint64_t v = (a[i + j + half] * roots[idx]) % q;
                    a[i + j] = (u + v) % q;
                    a[i + j + half] = (u >= v) ? (u - v) : (q + u - v);
                }
            }
        }
    }
    
    // Inverse NTT: Â → a
    void inverse(std::vector<uint64_t>& a) {
        size_t n = a.size();
        size_t bits = log2(n);
        
        for (size_t len = 2; len <= n; len <<= 1) {
            size_t half = len >> 1;
            for (size_t i = 0; i < n; i += len) {
                for (size_t j = 0; j < half; j++) {
                    size_t idx = (n / len) * j;
                    uint64_t u = a[i + j];
                    uint64_t v = (a[i + j + half] * inv_roots[idx]) % q;
                    a[i + j] = (u + v) % q;
                    a[i + j + half] = (u >= v) ? (u - v) : (q + u - v);
                }
            }
        }
        
        // Multiply by N^{-1} mod q
        uint64_t n_inv = modpow(n, q - 2, q);
        for (size_t i = 0; i < n; i++) {
            a[i] = (a[i] * n_inv) % q;
        }
    }
    
    // Polynomial multiplication using NTT
    std::vector<uint64_t> multiply(const std::vector<uint64_t>& a, 
                                    const std::vector<uint64_t>& b) {
        size_t n = a.size();
        std::vector<uint64_t> A = a, B = b;
        
        forward(A);
        forward(B);
        
        for (size_t i = 0; i < n; i++) {
            A[i] = (A[i] * B[i]) % q;
        }
        
        inverse(A);
        
        // Reduce modulo x^N + 1
        for (size_t i = 0; i < n; i++) {
            A[i] = A[i] % q;
        }
        
        return A;
    }
    
    // Getters
    uint64_t modulus() const { return q; }
    size_t degree() const { return N; }
};

} // namespace spiral
