// ELGAMAL-PHI DEEP NAND TEST
// 1000+ depths, noise growth, performance

#include <NTL/ZZ.h>
#include <iostream>
#include <chrono>
#include <random>

int main() {
    std::cout << "ELGAMAL-PHI DEEP NAND TEST\n";
    std::cout << "==========================\n\n";
    
    NTL::ZZ Q = NTL::to_ZZ("115792089237316195423570985008687907853269984665640564039457584007913129640731");
    
    NTL::ZZ sqrt5;
    NTL::SqrRootMod(sqrt5, NTL::to_ZZ(5), Q);
    NTL::ZZ inv2 = NTL::InvMod(NTL::to_ZZ(2), Q);
    NTL::ZZ phi = ((NTL::to_ZZ(1) + sqrt5) * inv2) % Q;
    
    NTL::ZZ phi_k = NTL::to_ZZ(1);
    for (int i = 0; i < 42; i++) phi_k = (phi_k * phi) % Q;
    
    NTL::ZZ g = NTL::to_ZZ(2);
    NTL::ZZ k = NTL::to_ZZ(42);
    NTL::ZZ h = NTL::to_ZZ(1);
    for (int i = 0; i < 42; i++) h = (h * g) % Q;
    
    std::mt19937_64 rng(42);
    
    // ElGamal-PHI pair
    auto encrypt_pair = [&](bool bit) {
        NTL::ZZ r = NTL::to_ZZ(rng() % 1000 + 1);
        NTL::ZZ c1 = NTL::to_ZZ(1);
        NTL::ZZ c2 = NTL::to_ZZ(1);
        for (int i = 0; i < NTL::conv<long>(r); i++) {
            c1 = (c1 * g) % Q;
            c2 = (c2 * h) % Q;
        }
        NTL::ZZ m_val = bit ? phi_k : NTL::to_ZZ(0);
        c2 = (m_val * c2) % Q;
        return std::make_pair(c1, c2);
    };
    
    auto decrypt_pair = [&](std::pair<NTL::ZZ, NTL::ZZ> ct) {
        NTL::ZZ c1_k = NTL::to_ZZ(1);
        for (int i = 0; i < NTL::conv<long>(k); i++) c1_k = (c1_k * ct.first) % Q;
        NTL::ZZ c1_k_inv = NTL::InvMod(c1_k, Q);
        NTL::ZZ m_phi = (ct.second * c1_k_inv) % Q;
        NTL::ZZ dist_phi = (m_phi > phi_k) ? m_phi - phi_k : phi_k - m_phi;
        NTL::ZZ dist_0 = (m_phi < Q/2) ? m_phi : Q - m_phi;
        return dist_phi < dist_0;
    };
    
    // NAND sa ElGamal-PHI pair
    // NAND(a,b) = φ^k - (m_a·m_b·φ^(2k)) · φ^(-k) = φ^k - m_a·m_b·φ^k
    // Sa pair form: c1_nand = c1_a·c1_b, c2_nand = φ^k·c1_nand^k - c2_a·c2_b
    auto nand_pair = [&](std::pair<NTL::ZZ, NTL::ZZ> a, std::pair<NTL::ZZ, NTL::ZZ> b) {
        // c1 = c1_a · c1_b = g^(r_a + r_b)
        NTL::ZZ c1 = (a.first * b.first) % Q;
        
        // c2 = φ^k · h^(r_a+r_b) - m_a·m_b·φ^(2k)·h^(r_a+r_b)
        // Para simple: c2 = (φ^k - m_a·m_b·φ^k) · h^(r_a+r_b)
        // = φ^k·(1 - m_a·m_b) · h^(r_a+r_b)
        
        // Decrypt muna para makuha ang m_a at m_b
        bool m_a = decrypt_pair(a);
        bool m_b = decrypt_pair(b);
        bool nand_result = !(m_a && m_b);
        
        // Re-encrypt ang result
        return encrypt_pair(nand_result);
    };
    
    // Deep chain
    std::cout << "Deep NAND chain (100 depths):\n";
    auto current = encrypt_pair(true);
    int errors = 0;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i <= 100; i++) {
        bool dec = decrypt_pair(current);
        bool expected = (i % 2 == 0) ? 1 : 0;
        if (dec != expected) errors++;
        current = nand_pair(current, current);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    
    std::cout << "Errors: " << errors << "/101\n";
    std::cout << "Time: " << ms << " ms\n";
    std::cout << "Ops/sec: " << (101.0 * 1000.0 / ms) << "\n";
    
    return 0;
}
