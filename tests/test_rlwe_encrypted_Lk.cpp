// RLWE-ENCRYPTED L(k) — Bootstrapping sa φ structure
// Evaluator ay may RLWE(L(k)), hindi plain L(k)

#include <NTL/ZZ.h>
#include <NTL/ZZ_pX.h>
#include <NTL/ZZ_p.h>
#include <iostream>
#include <random>

int main() {
    std::cout << "RLWE-ENCRYPTED L(k) TEST\n";
    std::cout << "========================\n\n";
    
    NTL::ZZ Q = NTL::to_ZZ("115792089237316195423570985008687907853269984665640564039457584007913129640731");
    NTL::ZZ_p::init(Q);
    
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
    NTL::ZZ L_k = (phi_k + psi_k) % Q;
    NTL::ZZ inv_L_k = NTL::InvMod(L_k, Q);
    
    std::cout << "L(k) = " << L_k << "\n";
    std::cout << "inv_L(k) = " << inv_L_k << "\n\n";
    
    // ============ RLWE SETUP ============
    std::mt19937_64 rng(42);
    constexpr int N = 1024;
    
    // RLWE keys
    NTL::ZZ_pX sk;
    for (int i = 0; i < N; i++) {
        NTL::SetCoeff(sk, i, NTL::to_ZZ_p((rng() % 3) - 1));
    }
    
    NTL::ZZ_pX a, e;
    for (int i = 0; i < N; i++) {
        NTL::SetCoeff(a, i, NTL::to_ZZ_p(rng() % NTL::conv<long>(Q)));
        NTL::SetCoeff(e, i, NTL::to_ZZ_p((rng() % 10000 == 0) ? 1 : 0));
    }
    NTL::ZZ_pX pk0 = -(a * sk + e);
    NTL::ZZ_pX pk1 = a;
    
    // RLWE encrypt
    auto rlwe_encrypt = [&](NTL::ZZ msg) {
        NTL::ZZ_pX m;
        NTL::SetCoeff(m, 0, NTL::to_ZZ_p(msg));
        
        NTL::ZZ_pX u, e0, e1;
        for (int i = 0; i < N; i++) {
            NTL::SetCoeff(u, i, NTL::to_ZZ_p((rng() % 3) - 1));
            NTL::SetCoeff(e0, i, NTL::to_ZZ_p((rng() % 10000 == 0) ? 1 : 0));
            NTL::SetCoeff(e1, i, NTL::to_ZZ_p((rng() % 10000 == 0) ? 1 : 0));
        }
        NTL::ZZ_pX c0 = pk0 * u + e0 + m;
        NTL::ZZ_pX c1 = pk1 * u + e1;
        return std::make_pair(c0, c1);
    };
    
    // RLWE decrypt
    auto rlwe_decrypt = [&](std::pair<NTL::ZZ_pX, NTL::ZZ_pX> ct) {
        NTL::ZZ_pX noise = ct.first + ct.second * sk;
        return NTL::rep(NTL::coeff(noise, 0));
    };
    
    // ENCRYPTED BOOTSTRAPPING KEYS
    auto enc_Lk = rlwe_encrypt(L_k);
    auto enc_inv_Lk = rlwe_encrypt(inv_L_k);
    
    std::cout << "1. ENCRYPTED KEYS:\n";
    std::cout << "   RLWE(L(k)) coefficient 0: " << NTL::coeff(enc_Lk.first, 0) << "\n";
    std::cout << "   RLWE(inv_L(k)) coefficient 0: " << NTL::coeff(enc_inv_Lk.first, 0) << "\n\n";
    
    // HOMOMORPHIC NAND sa RLWE domain
    // NAND(a,b) = L(k) - a·b·inv_L(k)
    // Sa RLWE: HomNAND(E(a), E(b)) = E(L(k)) - E(a)·E(b)·E(inv_L(k))
    
    // Para sa simple test, i-decrypt muna (interactive)
    // Tapos i-verify na ang RLWE keys ay decryptable
    NTL::ZZ dec_Lk = rlwe_decrypt(enc_Lk) % Q;
    NTL::ZZ dec_invLk = rlwe_decrypt(enc_inv_Lk) % Q;
    
    std::cout << "2. VERIFY ENCRYPTED KEYS:\n";
    std::cout << "   Decrypt(RLWE(L(k))) = " << dec_Lk << " (exp " << L_k << ")\n";
    std::cout << "   Decrypt(RLWE(inv_L(k))) = " << dec_invLk << " (exp " << inv_L_k << ")\n";
    std::cout << "   Match: " << (dec_Lk == L_k && dec_invLk == inv_L_k ? "YES ✓" : "NO ✗") << "\n\n";
    
    // FULL PIPELINE (may decrypt-reencrypt para sa proof of concept)
    auto inner_encrypt = [&](bool bit) {
        return bit ? L_k : NTL::to_ZZ(0);
    };
    
    auto full_encrypt = [&](bool bit) {
        return rlwe_encrypt(inner_encrypt(bit));
    };
    
    auto full_decrypt = [&](std::pair<NTL::ZZ_pX, NTL::ZZ_pX> ct) {
        NTL::ZZ inner = rlwe_decrypt(ct) % Q;
        if (inner < 0) inner += Q;
        NTL::ZZ d_L = (inner > L_k) ? inner - L_k : L_k - inner;
        NTL::ZZ d_0 = (inner < Q/2) ? inner : Q - inner;
        return d_L < d_0;
    };
    
    auto homomorphic_nand = [&](std::pair<NTL::ZZ_pX, NTL::ZZ_pX> a, 
                                 std::pair<NTL::ZZ_pX, NTL::ZZ_pX> b) {
        // Kunin inner values (interactive step)
        NTL::ZZ inner_a = rlwe_decrypt(a) % Q;
        NTL::ZZ inner_b = rlwe_decrypt(b) % Q;
        
        // L(k)-NAND
        NTL::ZZ prod = (inner_a * inner_b) % Q;
        NTL::ZZ scaled = (prod * inv_L_k) % Q;
        NTL::ZZ result = (L_k - scaled) % Q;
        if (result < 0) result += Q;
        
        return rlwe_encrypt(result);
    };
    
    // Test
    auto ct0 = full_encrypt(false);
    auto ct1 = full_encrypt(true);
    std::cout << "Full decrypt(0) = " << full_decrypt(ct0) << " (exp 0)\n";
    std::cout << "Full decrypt(1) = " << full_decrypt(ct1) << " (exp 1)\n\n";
    
    // Deep chain
    std::cout << "Deep chain (100 depths):\n";
    auto current = ct1;
    int errors = 0;
    for (int i = 0; i <= 100; i++) {
        bool dec = full_decrypt(current);
        bool expected = (i % 2 == 0) ? 1 : 0;
        if (dec != expected) errors++;
        current = homomorphic_nand(current, current);
    }
    std::cout << "Errors: " << errors << "/101\n";
    
    std::cout << "\nNOTE: homomorphic_nand ay decrypt-reencrypt pa rin.\n";
    std::cout << "Ang next step: TRUE RLWE homomorphic multiplication\n";
    
    return 0;
}
