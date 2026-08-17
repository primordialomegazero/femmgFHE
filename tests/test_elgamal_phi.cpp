// ELGAMAL-PHI HYBRID
// ct = m·φ^k · g^r + r'·Q

#include <NTL/ZZ.h>
#include <iostream>
#include <random>

int main() {
    std::cout << "ELGAMAL-PHI HYBRID TEST\n";
    std::cout << "======================\n\n";
    
    NTL::ZZ Q = NTL::to_ZZ("115792089237316195423570985008687907853269984665640564039457584007913129640731");
    
    NTL::ZZ sqrt5;
    NTL::SqrRootMod(sqrt5, NTL::to_ZZ(5), Q);
    NTL::ZZ inv2 = NTL::InvMod(NTL::to_ZZ(2), Q);
    NTL::ZZ phi = ((NTL::to_ZZ(1) + sqrt5) * inv2) % Q;
    
    // φ^k
    NTL::ZZ phi_k = NTL::to_ZZ(1);
    for (int i = 0; i < 42; i++) phi_k = (phi_k * phi) % Q;
    
    // ElGamal keys
    NTL::ZZ g = NTL::to_ZZ(2);  // generator (simplified)
    NTL::ZZ k = NTL::to_ZZ(42); // secret
    NTL::ZZ h = NTL::to_ZZ(1);
    for (int i = 0; i < 42; i++) h = (h * g) % Q;
    
    std::mt19937_64 rng(42);
    
    // Encrypt: ct = (m·φ^k) · g^r + r'·Q
    auto encrypt = [&](bool bit) {
        NTL::ZZ r = NTL::to_ZZ(rng() % 1000 + 1);
        NTL::ZZ g_r = NTL::to_ZZ(1);
        for (int i = 0; i < NTL::conv<long>(r); i++) g_r = (g_r * g) % Q;
        
        NTL::ZZ m_val = bit ? phi_k : NTL::to_ZZ(0);
        NTL::ZZ inner = (m_val * g_r) % Q;
        
        NTL::ZZ r_prime = NTL::RandomBnd(Q);
        return inner + r_prime * Q;
    };
    
    // Decrypt: ct mod Q → inner = m·φ^k·g^r
    // Divide by g^r (kailangan malaman ang r)
    // Pero hindi alam ang r! Kailangan ng ElGamal pair.
    
    std::cout << "PROBLEM: Ang g^r ay nagma-mask ng φ^k,\n";
    std::cout << "pero hindi natin ma-recover kung walang r.\n\n";
    
    std::cout << "SOLUTION: ElGamal PAIR (c1, c2):\n";
    std::cout << "  c1 = g^r\n";
    std::cout << "  c2 = m·φ^k · h^r\n";
    std::cout << "  kung saan h = g^k\n\n";
    std::cout << "  Decrypt: c2 / c1^k = m·φ^k\n";
    std::cout << "  → Pagkatapos, NAND sa m·φ^k\n\n";
    
    // Implement ElGamal-PHI pair
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
        for (int i = 0; i < NTL::conv<long>(k); i++) {
            c1_k = (c1_k * ct.first) % Q;
        }
        NTL::ZZ c1_k_inv = NTL::InvMod(c1_k, Q);
        NTL::ZZ m_phi = (ct.second * c1_k_inv) % Q;
        
        // m_phi = m·φ^k → check dist sa 0 o φ^k
        NTL::ZZ dist_phi = (m_phi > phi_k) ? m_phi - phi_k : phi_k - m_phi;
        NTL::ZZ dist_0 = (m_phi < Q/2) ? m_phi : Q - m_phi;
        return dist_phi < dist_0;
    };
    
    // Test
    auto ct0 = encrypt_pair(false);
    auto ct1 = encrypt_pair(true);
    
    std::cout << "Decrypt(ct0) = " << decrypt_pair(ct0) << " (exp 0)\n";
    std::cout << "Decrypt(ct1) = " << decrypt_pair(ct1) << " (exp 1)\n";
    
    return 0;
}
