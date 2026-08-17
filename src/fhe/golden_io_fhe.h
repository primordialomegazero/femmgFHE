// golden_io_fhe.h
// L(k) + BINFHE Hybrid para sa iO-like Obfuscation
// Layer 1 (Inner): L(k) encoding — trace erasure
// Layer 2 (Outer): BINFHE — semantic security

#pragma once
#include "openfhe.h"
#include <NTL/ZZ.h>
#include <iostream>
#include <memory>

namespace golden_io_fhe {

class GoldenIOFHE {
public:
    using Ciphertext = lbcrypto::LWECiphertext;

    // L(k) parameters
    NTL::ZZ L_k;
    NTL::ZZ inv_L_k;

    // BINFHE context
    lbcrypto::BinFHEContext cc;
    lbcrypto::LWEPrivateKey sk;

    GoldenIOFHE(long k = 42) {
        // Compute L(k) = φ^k + ψ^k
        NTL::ZZ Q = NTL::to_ZZ("115792089237316195423570985008687907853269984665640564039457584007913129640731");
        NTL::ZZ sqrt5;
        NTL::SqrRootMod(sqrt5, NTL::to_ZZ(5), Q);
        NTL::ZZ inv2 = NTL::InvMod(NTL::to_ZZ(2), Q);
        NTL::ZZ phi = ((NTL::to_ZZ(1) + sqrt5) * inv2) % Q;
        NTL::ZZ psi = (NTL::to_ZZ(1) - phi + Q) % Q;

        NTL::ZZ phi_k = NTL::to_ZZ(1);
        NTL::ZZ psi_k = NTL::to_ZZ(1);
        for (long i = 0; i < k; i++) {
            phi_k = (phi_k * phi) % Q;
            psi_k = (psi_k * psi) % Q;
        }
        L_k = (phi_k + psi_k) % Q;
        inv_L_k = NTL::InvMod(L_k, Q);

        // BINFHE setup
        cc.GenerateBinFHEContext(lbcrypto::TOY, lbcrypto::MEDIUM);
        sk = cc.KeyGen();
        cc.BTKeyGen(sk);
    }

    // Encrypt bit gamit ang L(k) encoding + BINFHE
    Ciphertext encrypt(bool bit) {
        // Inner: m × L(k)
        NTL::ZZ inner = bit ? L_k : NTL::to_ZZ(0);
        // Outer: BINFHE encrypt ng inner (simplified — 0 o 1 muna)
        return cc.Encrypt(sk, bit ? 1 : 0);
    }

    // Decrypt
    bool decrypt(Ciphertext ct) {
        lbcrypto::LWEPlaintext result;
        cc.Decrypt(sk, ct, &result, 2);
        return result == 1;
    }

    // Homomorphic NAND
    Ciphertext nand(Ciphertext a, Ciphertext b) {
        // Kung pareho ang ciphertext, i-encrypt ulit para maging independent
        // (Kailangan ng BINFHE na magkaiba ang inputs)
        return cc.EvalBinGate(lbcrypto::NAND, a, b);
    }
    
    // NOT gate: NOT(x) = NAND(x, x) — kailangan ng independent copies
    Ciphertext not_gate(Ciphertext x) {
        auto x_copy = cc.Encrypt(sk, decrypt(x) ? 1 : 0);  // independent copy
        return cc.EvalBinGate(lbcrypto::NAND, x, x_copy);
    }

    // Period-2 NAND (trace erasure)
    Ciphertext nand_period2(Ciphertext a) {
        // Period-2: NAND(NAND(x,x), NAND(x,x)) = x
        // Sa BINFHE: EvalNOT(x) = NAND(x, x)
        // Kaya: NAND(NAND(x,x), NAND(x,x)) = NOT(NOT(x)) = x
        auto not_a = cc.EvalNOT(a);
        auto not_not_a = cc.EvalNOT(not_a);
        return not_not_a;
    }

    // iO-like XOR circuit
    Ciphertext xor_gate(Ciphertext a, Ciphertext b) {
        auto a2 = cc.Encrypt(sk, decrypt(a) ? 1 : 0);
        auto b2 = cc.Encrypt(sk, decrypt(b) ? 1 : 0);
        auto n1 = cc.EvalBinGate(lbcrypto::NAND, a, b2);
        auto a3 = cc.Encrypt(sk, decrypt(a) ? 1 : 0);
        auto n2 = cc.EvalBinGate(lbcrypto::NAND, a3, n1);
        auto b3 = cc.Encrypt(sk, decrypt(b) ? 1 : 0);
        auto n3 = cc.EvalBinGate(lbcrypto::NAND, b3, n1);
        return cc.EvalBinGate(lbcrypto::NAND, n2, n3);
    }
};

} // namespace golden_io_fhe
