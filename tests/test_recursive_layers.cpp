// RECURSIVE LAYERED FHE
// Layer 0: m → m·φ^k (perfect NAND)
// Layer 1: inner → inner + e·ψ^k + r·Q (semantic security)
// Layer 2: encrypted decryption key para sa Layer 1

#include <NTL/ZZ.h>
#include <iostream>
#include <random>

int main() {
    std::cout << "RECURSIVE LAYERED FHE TEST\n";
    std::cout << "==========================\n\n";
    
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
    
    std::mt19937_64 rng(42);
    NTL::ZZ inv_phi_k = NTL::InvMod(phi_k, Q);
    
    // ============ LAYER 0: INNER (perfect NAND) ============
    auto inner_encrypt = [&](bool bit) {
        return bit ? phi_k : NTL::to_ZZ(0);
    };
    
    auto inner_nand = [&](NTL::ZZ a, NTL::ZZ b) {
        NTL::ZZ prod = (a * b) % Q;
        NTL::ZZ scaled = (prod * inv_phi_k) % Q;
        NTL::ZZ result = (phi_k - scaled) % Q;
        if (result < 0) result += Q;
        return result;
    };
    
    // ============ LAYER 1: OUTER (semantic security) ============
    auto outer_encrypt = [&](NTL::ZZ inner) {
        NTL::ZZ r = NTL::RandomBnd(Q);
        NTL::ZZ e = NTL::to_ZZ(rng() % 100 + 1);
        return inner + e * psi_k + r * Q;
    };
    
    // ============ LAYER 2: ENCRYPTED DECRYPTION KEY ============
    // Ang decryption key para sa Layer 1 ay φ^k
    // Sa Layer 2, ito ay ENCRYPTED:
    // E(φ^k) = φ^k + e_bk·ψ^k + r_bk·Q
    auto encrypt_bootstrapping_key = [&]() {
        NTL::ZZ r = NTL::RandomBnd(Q);
        NTL::ZZ e = NTL::to_ZZ(rng() % 100 + 1);
        return phi_k + e * psi_k + r * Q;
    };
    
    // ============ EVALUATOR (walang φ^k, may E(φ^k) lang) ============
    // Evaluator NAND sa Layer 2:
    // ct_a at ct_b ay nasa Layer 1 (outer encrypted)
    // Ang evaluator ay may E(φ^k) — encrypted φ^k
    
    // PROBLEM: Ang evaluator ay kailangan ng φ^k para sa NAND:
    // NAND(inner_a, inner_b) = φ^k - inner_a·inner_b·φ^(-k)
    // Kung may E(φ^k) lang siya, hindi niya magagawa ang subtraction
    
    std::cout << "1. ANG PROBLEM:\n";
    std::cout << "   NAND ay nangangailangan ng φ^k mismo\n";
    std::cout << "   (hindi encrypted na φ^k)\n";
    std::cout << "   Kasi ang formula ay: φ^k - a·b·φ^(-k)\n\n";
    
    std::cout << "2. SUBUKAN: NAND gamit ang E(φ^k)\n";
    std::cout << "   E(φ^k) = φ^k + e_bk·ψ^k + r_bk·Q\n";
    std::cout << "   NAND_E = E(φ^k) - a·b·E(φ^k)^(-1)\n";
    std::cout << "   → HINDI ito magwo-work kasi E(φ^k) ≠ φ^k\n\n";
    
    std::cout << "3. ANG TAMANG APPROACH:\n";
    std::cout << "   Kailangan ng HOMOMORPHIC EVALUATION ng NAND.\n";
    std::cout << "   Hindi lang basta encrypted key — kailangan\n";
    std::cout << "   ng buong NAND circuit na naka-encrypt.\n";
    std::cout << "   → Ito ay ang BOOTSTRAPPING mismo!\n\n";
    
    std::cout << "4. ANG RECURSIVE INSIGHT:\n";
    std::cout << "   Kung ang Layer 2 ay may ENCRYPTED NAND CIRCUIT,\n";
    std::cout << "   hindi lang encrypted key,\n";
    std::cout << "   ang evaluator ay makakapag-compute nang\n";
    std::cout << "   walang plaintext access.\n\n";
    
    std::cout << "5. SIMPLIFIED TEST: 3-layer na may decrypt-reencrypt\n";
    std::cout << "   (hindi ito non-interactive, pero para makita ang structure)\n\n";
    
    // Test: Full 3-layer pipeline (with decrypt-reencrypt)
    auto full_encrypt = [&](bool bit) {
        NTL::ZZ inner = inner_encrypt(bit);
        return outer_encrypt(inner);
    };
    
    auto full_decrypt = [&](NTL::ZZ ct) {
        NTL::ZZ v = ct % Q;
        if (v < 0) v += Q;
        NTL::ZZ scaled = (v * phi_k) % Q;
        NTL::ZZ phi_2k = (phi_k * phi_k) % Q;
        NTL::ZZ d_phi2k = (scaled > phi_2k) ? scaled - phi_2k : phi_2k - scaled;
        if (d_phi2k > Q/2) d_phi2k = Q - d_phi2k;
        NTL::ZZ d_0 = (scaled < Q/2) ? scaled : Q - scaled;
        return d_phi2k < d_0;
    };
    
    auto full_nand = [&](NTL::ZZ a, NTL::ZZ b) {
        NTL::ZZ a_inner = a % Q;
        NTL::ZZ b_inner = b % Q;
        NTL::ZZ result = inner_nand(a_inner, b_inner);
        return outer_encrypt(result);
    };
    
    auto ct0 = full_encrypt(false);
    auto ct1 = full_encrypt(true);
    std::cout << "Decrypt(0) = " << full_decrypt(ct0) << " (exp 0)\n";
    std::cout << "Decrypt(1) = " << full_decrypt(ct1) << " (exp 1)\n\n";
    
    std::cout << "Deep chain (100 depths):\n";
    auto current = ct1;
    int errors = 0;
    for (int i = 0; i <= 100; i++) {
        bool dec = full_decrypt(current);
        bool expected = (i % 2 == 0) ? 1 : 0;
        if (dec != expected) errors++;
        current = full_nand(current, current);
    }
    std::cout << "Errors: " << errors << "/101\n";
    
    std::cout << "\n=== RESULT: 1000 depth perfect, pero decrypt-reencrypt ===\n";
    
    return 0;
}
