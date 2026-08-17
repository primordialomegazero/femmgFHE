// PERIOD-2 NOISE REDUCTION TEST
// May natural bang noise refresh pagkatapos ng 2 NANDs?

#include <NTL/ZZ.h>
#include <iostream>
#include <random>

int main() {
    std::cout << "PERIOD-2 NOISE REDUCTION TEST\n";
    std::cout << "=============================\n\n";
    
    NTL::ZZ Q = NTL::to_ZZ("115792089237316195423570985008687907853269984665640564039457584007913129640731");
    
    NTL::ZZ sqrt5;
    NTL::SqrRootMod(sqrt5, NTL::to_ZZ(5), Q);
    NTL::ZZ inv2 = NTL::InvMod(NTL::to_ZZ(2), Q);
    NTL::ZZ phi = ((NTL::to_ZZ(1) + sqrt5) * inv2) % Q;
    NTL::ZZ psi = (NTL::to_ZZ(1) - phi + Q) % Q;
    
    NTL::ZZ phi_k = NTL::to_ZZ(1);
    NTL::ZZ psi_k = NTL::to_ZZ(1);
    for (int i = 0; i < 42; i++) {
        phi_k = (phi_k * phi) % Q;
        psi_k = (psi_k * psi) % Q;
    }
    NTL::ZZ inv_phi_k = NTL::InvMod(phi_k, Q);
    
    std::cout << "1. PURE φ TEST (walang noise):\n";
    NTL::ZZ inner = phi_k;  // message 1
    std::cout << "   NAND(φ^k, φ^k) = 0\n";
    std::cout << "   NAND(0, 0) = φ^k\n";
    std::cout << "   → Period-2: φ^k → 0 → φ^k → 0 ...\n\n";
    
    std::cout << "2. WITH NOISE (e·ψ^k):\n";
    std::cout << "   ct = inner + e·ψ^k\n";
    std::cout << "   NAND(ct, ct) = φ^k - ct²·φ^(-k)\n";
    std::cout << "   = φ^k - (inner + e·ψ^k)²·φ^(-k)\n";
    std::cout << "   = φ^k - (inner² + 2·inner·e·ψ^k + e²·ψ^(2k))·φ^(-k)\n";
    std::cout << "   = φ^k - inner²·φ^(-k) - 2·inner·e - e²·ψ^(2k)·φ^(-k)\n\n";
    
    std::cout << "3. KEY: 2·inner·e ay CONSTANT (hindi ψ-dependent)\n";
    std::cout << "   e²·ψ^(2k)·φ^(-k) = e²·L(k)·ψ^k·φ^(-k) - e²·φ^(-k)\n";
    std::cout << "   = e²·L(k) - e²·φ^(-k) (CONSTANT + small)\n\n";
    
    std::cout << "4. ANG NOISE AY NAGIGING SCALAR:\n";
    std::cout << "   After NAND: noise = 2·inner·e + e²·L(k) - e²·φ^(-k)\n";
    std::cout << "   → Lahat ay CONSTANT (hindi ψ^k-dependent)!\n";
    std::cout << "   → Hindi na ito ψ^k noise!\n";
    std::cout << "   → Ito ay SCALAR noise na maliit!\n\n";
    
    std::cout << "5. TEST: I-verify ang scalar noise\n";
    std::mt19937_64 rng(42);
    
    // Encrypt with noise
    auto encrypt_noisy = [&](bool bit, NTL::ZZ e) {
        NTL::ZZ inner = bit ? phi_k : NTL::to_ZZ(0);
        return inner + e * psi_k;
    };
    
    // NAND
    auto nand = [&](NTL::ZZ a, NTL::ZZ b) {
        NTL::ZZ prod = (a * b) % Q;
        NTL::ZZ scaled = (prod * inv_phi_k) % Q;
        NTL::ZZ result = (phi_k - scaled) % Q;
        if (result < 0) result += Q;
        return result;
    };
    
    // Decrypt
    auto decrypt = [&](NTL::ZZ ct) {
        NTL::ZZ scaled = (ct * phi_k) % Q;
        NTL::ZZ phi_2k = (phi_k * phi_k) % Q;
        NTL::ZZ d_phi2k = (scaled > phi_2k) ? scaled - phi_2k : phi_2k - scaled;
        if (d_phi2k > Q/2) d_phi2k = Q - d_phi2k;
        NTL::ZZ d_0 = (scaled < Q/2) ? scaled : Q - scaled;
        return d_phi2k < d_0;
    };
    
    // Trace noise
    NTL::ZZ e = NTL::to_ZZ(10);
    auto current = encrypt_noisy(true, e);
    
    std::cout << "   Depth | Decrypt | Expected | Scaled value\n";
    std::cout << "   ------|---------|----------|-------------\n";
    
    for (int i = 0; i <= 10; i++) {
        bool dec = decrypt(current);
        bool expected = (i % 2 == 0) ? 1 : 0;
        NTL::ZZ scaled = (current * phi_k) % Q;
        std::cout << "   " << i << " | " << dec << " | " << expected << " | " 
                  << scaled << "\n";
        current = nand(current, current);
    }
    
    return 0;
}
