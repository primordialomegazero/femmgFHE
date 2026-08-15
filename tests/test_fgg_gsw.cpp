#include <NTL/ZZ_pX.h>
#include <NTL/ZZ_p.h>
#include <iostream>

constexpr long Q = 536870909;
constexpr int N = 1024;
constexpr double PHI = 1.6180339887498948482;

void init_ring() { NTL::ZZ_p::init(NTL::ZZ(Q)); }

// FGG Reduction: X^N = X + 1
void fgg_reduce(NTL::ZZ_pX& poly) {
    if (NTL::deg(poly) < N) return;
    
    NTL::ZZ_pX result;
    result.SetLength(N);
    
    for (int i = 0; i <= NTL::deg(poly); i++) {
        NTL::ZZ_p coeff = NTL::coeff(poly, i);
        if (coeff == 0) continue;
        
        int reduced_deg = i % N;
        int multiplier = i / N;
        
        // X^(N+k) = X^k · (X+1) = X^(k+1) + X^k
        // Para sa mas mataas na powers, recursive application
        if (multiplier > 0) {
            // I-apply ang reduction nang paulit-ulit
            NTL::ZZ_pX temp;
            NTL::SetCoeff(temp, reduced_deg + multiplier, coeff);
            NTL::SetCoeff(temp, reduced_deg + multiplier - 1, coeff);
            result += temp;
        } else {
            NTL::SetCoeff(result, reduced_deg, 
                         NTL::coeff(result, reduced_deg) + coeff);
        }
    }
    
    poly = result;
}

struct MatrixCipher {
    NTL::ZZ_pX m00, m01, m10, m11;
};

// FGG-based GSW encryption
MatrixCipher encrypt(bool bit, NTL::ZZ_pX& s, uint64_t nonce) {
    MatrixCipher ct;
    
    long scale = static_cast<long>(Q / PHI);
    long msg = bit ? scale : 0;
    
    // Simple key: s(0) = 1
    NTL::SetCoeff(s, 0, 1);
    
    // Random a
    NTL::ZZ_pX a;
    uint64_t state = nonce;
    for (int i = 0; i < 10; i++) {
        state ^= (state << 13);
        state ^= (state >> 7);
        state ^= (state << 17);
        NTL::SetCoeff(a, i, state % Q);
    }
    
    // Zero error para sa test
    NTL::ZZ_pX e;
    
    ct.m00 = e + a * s;
    NTL::SetCoeff(ct.m00, 0, NTL::coeff(ct.m00, 0) + msg);
    ct.m01 = a;
    ct.m10.SetLength(0);
    ct.m11.SetLength(0);
    
    fgg_reduce(ct.m00);
    fgg_reduce(ct.m01);
    
    return ct;
}

MatrixCipher multiply(const MatrixCipher& c1, const MatrixCipher& c2) {
    MatrixCipher result;
    
    result.m00 = c1.m00 * c2.m00 + c1.m01 * c2.m10;
    result.m01 = c1.m00 * c2.m01 + c1.m01 * c2.m11;
    result.m10 = c1.m10 * c2.m00 + c1.m11 * c2.m10;
    result.m11 = c1.m10 * c2.m01 + c1.m11 * c2.m11;
    
    fgg_reduce(result.m00);
    fgg_reduce(result.m01);
    fgg_reduce(result.m10);
    fgg_reduce(result.m11);
    
    return result;
}

bool decrypt(const MatrixCipher& ct, const NTL::ZZ_pX& s) {
    NTL::ZZ_pX result = ct.m00 + ct.m01 * s;
    fgg_reduce(result);
    
    long value = NTL::conv<long>(NTL::coeff(result, 0));
    long threshold = static_cast<long>(Q / (2 * PHI));
    
    return value > threshold;
}

int main() {
    init_ring();
    
    std::cout << "FGG-GSW TEST (X^N = X + 1)\n";
    std::cout << "============================\n\n";
    
    NTL::ZZ_pX s;
    NTL::SetCoeff(s, 0, 1);
    
    auto ct0 = encrypt(false, s, 1000);
    auto ct1 = encrypt(true, s, 2000);
    
    std::cout << "Encrypt(0) → Decrypt = " << decrypt(ct0, s) << " (expected 0)\n";
    std::cout << "Encrypt(1) → Decrypt = " << decrypt(ct1, s) << " (expected 1)\n\n";
    
    // Test multiplication
    auto mult = multiply(ct0, ct1);
    std::cout << "0×1 → Decrypt = " << decrypt(mult, s) << " (expected 0)\n\n";
    
    // Test degree stability
    MatrixCipher current = ct0;
    std::cout << "Depth\tm00 deg\tm01 deg\n";
    std::cout << "-----\t-------\t-------\n";
    
    for (int i = 0; i < 10; i++) {
        current = multiply(current, ct1);
        std::cout << i+1 << "\t" << NTL::deg(current.m00) 
                  << "\t" << NTL::deg(current.m01) << "\n";
    }
    
    std::cout << "\n";
    bool bounded = NTL::deg(current.m00) < N && NTL::deg(current.m01) < N;
    std::cout << "Status: " << (bounded ? "BOUNDED ✓" : "UNBOUNDED ✗") << "\n";
    
    return 0;
}
