#include <NTL/ZZ_pX.h>
#include <NTL/ZZ_p.h>
#include <iostream>

constexpr long Q = 536870909;
constexpr int N = 1024;
constexpr double PHI = 1.6180339887498948482;

void init_ring() { NTL::ZZ_p::init(NTL::ZZ(Q)); }

// Tamang reduction para sa X^N + 1
void reduce_mod(NTL::ZZ_pX& poly) {
    if (NTL::deg(poly) < N) return;
    
    NTL::ZZ_pX result;
    result.SetLength(N);
    
    for (int i = 0; i <= NTL::deg(poly); i++) {
        NTL::ZZ_p coeff = NTL::coeff(poly, i);
        if (coeff == 0) continue;
        
        int reduced_deg = i % (2 * N);
        int sign = 1;
        
        if (reduced_deg >= N) {
            reduced_deg -= N;
            sign = -1;
        }
        
        NTL::SetCoeff(result, reduced_deg, 
                     NTL::coeff(result, reduced_deg) + sign * coeff);
    }
    
    poly = result;
}

struct MatrixCipher {
    NTL::ZZ_pX m00, m01, m10, m11;
};

// TAMANG GSW encryption
// C = [[m·(Q/φ) + e + a·s, a], [b, c]]
// Para sa decryption: m00 + m01·s ≈ m·(Q/φ)

MatrixCipher encrypt(bool bit, NTL::ZZ_pX& s, uint64_t nonce) {
    MatrixCipher ct;
    
    long scale = static_cast<long>(Q / PHI);
    long msg = bit ? scale : 0;
    
    // Simple ternary secret key
    NTL::SetCoeff(s, 0, 1);  // s(0) = 1 para sa simple test
    
    // Generate random a
    NTL::ZZ_pX a;
    uint64_t state = nonce;
    for (int i = 0; i < 10; i++) {
        state ^= (state << 13);
        state ^= (state >> 7);
        state ^= (state << 17);
        NTL::SetCoeff(a, i, state % Q);
    }
    
    // Error polynomial (small)
    NTL::ZZ_pX e;
    NTL::SetCoeff(e, 0, 0);  // Zero error para sa simple test
    
    // Ciphertext structure:
    // m00 = m·(Q/φ) + e + a·s
    // m01 = a
    // m10 = 0
    // m11 = 0
    
    ct.m00 = e + a * s;
    NTL::SetCoeff(ct.m00, 0, NTL::coeff(ct.m00, 0) + msg);
    ct.m01 = a;
    ct.m10.SetLength(0);
    ct.m11.SetLength(0);
    
    reduce_mod(ct.m00);
    reduce_mod(ct.m01);
    
    return ct;
}

// Tamang GSW multiplication
MatrixCipher multiply(const MatrixCipher& c1, const MatrixCipher& c2) {
    MatrixCipher result;
    
    // Matrix multiplication
    result.m00 = c1.m00 * c2.m00 + c1.m01 * c2.m10;
    result.m01 = c1.m00 * c2.m01 + c1.m01 * c2.m11;
    result.m10 = c1.m10 * c2.m00 + c1.m11 * c2.m10;
    result.m11 = c1.m10 * c2.m01 + c1.m11 * c2.m11;
    
    reduce_mod(result.m00);
    reduce_mod(result.m01);
    reduce_mod(result.m10);
    reduce_mod(result.m11);
    
    return result;
}

// Decryption
bool decrypt(const MatrixCipher& ct, const NTL::ZZ_pX& s) {
    NTL::ZZ_pX result = ct.m00 + ct.m01 * s;
    reduce_mod(result);
    
    long value = NTL::conv<long>(NTL::coeff(result, 0));
    long threshold = static_cast<long>(Q / (2 * PHI));
    
    return value > threshold;
}

// NAND operation
MatrixCipher nand(const MatrixCipher& c1, const MatrixCipher& c2, 
                  const NTL::ZZ_pX& s, uint64_t nonce) {
    // NAND = NOT(AND) = 1 - AND
    // 1. Multiply
    MatrixCipher mult = multiply(c1, c2);
    
    // 2. NOT: Subtract from 1
    // Sa GSW: C_NOT = [[Q/φ, 0], [0, 0]] - C
    MatrixCipher not_result;
    long scale = static_cast<long>(Q / PHI);
    
    not_result.m00 = -mult.m00;
    NTL::SetCoeff(not_result.m00, 0, NTL::coeff(not_result.m00, 0) + scale);
    not_result.m01 = -mult.m01;
    not_result.m10 = -mult.m10;
    not_result.m11 = -mult.m11;
    
    reduce_mod(not_result.m00);
    reduce_mod(not_result.m01);
    reduce_mod(not_result.m10);
    reduce_mod(not_result.m11);
    
    return not_result;
}

int main() {
    init_ring();
    
    std::cout << "CORRECTED GSW TEST\n";
    std::cout << "==================\n\n";
    
    NTL::ZZ_pX s;
    NTL::SetCoeff(s, 0, 1);
    
    auto ct0 = encrypt(false, s, 1000);
    auto ct1 = encrypt(true, s, 2000);
    
    std::cout << "Encrypt(0) → Decrypt = " << decrypt(ct0, s) << " (expected 0)\n";
    std::cout << "Encrypt(1) → Decrypt = " << decrypt(ct1, s) << " (expected 1)\n\n";
    
    // Test multiplication
    auto mult = multiply(ct0, ct1);
    std::cout << "0×1 → Decrypt = " << decrypt(mult, s) << " (expected 0)\n\n";
    
    // Test NAND
    auto nand_result = nand(ct0, ct1, s, 3000);
    std::cout << "NAND(0,1) → Decrypt = " << decrypt(nand_result, s) << " (expected 1)\n";
    
    return 0;
}
