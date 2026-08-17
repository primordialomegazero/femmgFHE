// L(k) FHE + RLWE OUTER LAYER
// Inner: L(k)-based NAND (non-interactive, unlimited)
// Outer: RLWE (semantic security)

#include <NTL/ZZ.h>
#include <NTL/ZZ_pX.h>
#include <NTL/ZZ_p.h>
#include <iostream>
#include <random>

int main() {
    std::cout << "L(k) FHE + RLWE OUTER TEST\n";
    std::cout << "==========================\n\n";
    
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
    
    // ============ RLWE KEYGEN ============
    std::mt19937_64 rng(42);
    constexpr int N = 1024;
    
    // RLWE secret key (small)
    NTL::ZZ_pX sk;
    for (int i = 0; i < N; i++) {
        NTL::SetCoeff(sk, i, NTL::to_ZZ_p((rng() % 3) - 1));
    }
    
    // RLWE public key: pk = (a·sk + e, a)
    NTL::ZZ_pX a, e;
    for (int i = 0; i < N; i++) {
        NTL::SetCoeff(a, i, NTL::to_ZZ_p(rng() % NTL::conv<long>(Q)));
        NTL::SetCoeff(e, i, NTL::to_ZZ_p((rng() % 10000 == 0) ? 1 : 0));
    }
    NTL::ZZ_pX pk0 = -(a * sk + e);
    NTL::ZZ_pX pk1 = a;
    
    // ============ INNER L(k) ENCRYPT ============
    auto inner_encrypt = [&](bool bit) {
        return bit ? L_k : NTL::to_ZZ(0);
    };
    
    // ============ OUTER RLWE ENCRYPT ============
    auto outer_encrypt = [&](NTL::ZZ inner) {
        NTL::ZZ_pX m;
        NTL::SetCoeff(m, 0, NTL::to_ZZ_p(inner));
        
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
    
    // ============ RLWE DECRYPT ============
    auto outer_decrypt = [&](std::pair<NTL::ZZ_pX, NTL::ZZ_pX> ct) {
        NTL::ZZ_pX noise = ct.first + ct.second * sk;
        return NTL::rep(NTL::coeff(noise, 0));
    };
    
    // ============ HOMOMORPHIC NAND sa RLWE ============
    // Kailangan: NAND sa L(k)-space na naka-RLWE-encrypt
    // NAND_L(a,b) = L(k) - a·b·inv_L(k)
    // Sa RLWE: homomorphic multiply at subtract
    
    // Para sa RLWE multiplication, kailangan ng relinearization
    // Simplification: gawin sa coefficient 0 lang (degree-0)
    auto homomorphic_nand = [&](std::pair<NTL::ZZ_pX, NTL::ZZ_pX> a, 
                                 std::pair<NTL::ZZ_pX, NTL::ZZ_pX> b) {
        // Kunin ang inner values (decrypt)
        NTL::ZZ inner_a = outer_decrypt(a) % Q;
        NTL::ZZ inner_b = outer_decrypt(b) % Q;
        
        // L(k)-NAND
        NTL::ZZ prod = (inner_a * inner_b) % Q;
        NTL::ZZ scaled = (prod * inv_L_k) % Q;
        NTL::ZZ result = (L_k - scaled) % Q;
        if (result < 0) result += Q;
        
        // Re-encrypt
        return outer_encrypt(result);
    };
    
    // ============ FULL PIPELINE ============
    auto full_encrypt = [&](bool bit) {
        NTL::ZZ inner = inner_encrypt(bit);
        return outer_encrypt(inner);
    };
    
    auto full_decrypt = [&](std::pair<NTL::ZZ_pX, NTL::ZZ_pX> ct) {
        NTL::ZZ inner = outer_decrypt(ct) % Q;
        if (inner < 0) inner += Q;
        NTL::ZZ d_L = (inner > L_k) ? inner - L_k : L_k - inner;
        NTL::ZZ d_0 = (inner < Q/2) ? inner : Q - inner;
        return d_L < d_0;
    };
    
    // Test
    auto ct0 = full_encrypt(false);
    auto ct1 = full_encrypt(true);
    
    std::cout << "Full decrypt(0) = " << full_decrypt(ct0) << " (exp 0)\n";
    std::cout << "Full decrypt(1) = " << full_decrypt(ct1) << " (exp 1)\n\n";
    
    // Deep chain — may decrypt-reencrypt (hindi pa non-interactive sa RLWE)
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
    
    std::cout << "\nNOTE: Ang homomorphic_nand ay decrypt-reencrypt pa rin.\n";
    std::cout << "Para sa tunay na non-interactive, kailangan ng RLWE homomorphic multiply.\n";
    
    return 0;
}
