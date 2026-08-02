// BFV φ-CYCLE: Integer FHE with φ^L ≡ 1 mod p
// Plaintext modulus p where φ has known order

#include <iostream>
#include <iomanip>
#include <cmath>
#include <vector>
#include "openfhe.h"
using namespace lbcrypto;

int main() {
    std::cout << "\n";
    std::cout << "  BFV φ-CYCLE: Integer FHE with cyclic modulus\n";
    std::cout << "  Plaintext modulus p=199, φ order=22\n\n";

    // BFV with plaintext modulus 199
    CCParams<CryptoContextBFVRNS> p;
    p.SetPlaintextModulus(199);
    p.SetMultiplicativeDepth(50);
    p.SetBatchSize(2048);
    p.SetRingDim(8192);
    p.SetSecretKeyDist(UNIFORM_TERNARY);
    p.SetSecurityLevel(HEStd_NotSet);
    
    auto cc = GenCryptoContext(p);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    cc->Enable(ADVANCEDSHE);
    auto kp = cc->KeyGen();
    cc->EvalMultKeyGen(kp.secretKey);

    std::cout << "  BFV setup with plaintext modulus 199: SUCCESS\n\n";

    // φ in mod 199 arithmetic
    // φ = (1+√5)/2, but we need √5 mod 199
    // √5 mod 199: find x where x² ≡ 5 mod 199
    int sqrt5 = -1;
    for (int x = 0; x < 199; x++) {
        if ((x * x) % 199 == 5) {
            sqrt5 = x;
            break;
        }
    }
    std::cout << "  √5 mod 199 = " << sqrt5 << "\n";
    
    // φ = (1 + √5) / 2 mod 199
    // Need modular inverse of 2: 2*100 = 200 ≡ 1 mod 199
    int inv2 = 100;
    int phi_mod = ((1 + sqrt5) * inv2) % 199;
    std::cout << "  φ mod 199 = (1 + " << sqrt5 << ") * " << inv2 << " mod 199 = " << phi_mod << "\n";
    
    // ψ = (1 - √5) / 2 mod 199
    int psi_mod = ((1 - sqrt5 + 199) * inv2) % 199;
    std::cout << "  ψ mod 199 = (1 - " << sqrt5 << ") * " << inv2 << " mod 199 = " << psi_mod << "\n\n";

    // Verify: φ^22 ≡ 1 mod 199
    std::cout << "  Verifying φ^22 mod 199:\n";
    long long val = 1;
    for (int i = 0; i < 22; i++) {
        val = (val * phi_mod) % 199;
    }
    std::cout << "  φ^22 mod 199 = " << val << " (should be 1)\n\n";

    // Encode bit 0 = ψ, bit 1 = φ
    auto pt_psi = cc->MakePackedPlaintext(std::vector<int64_t>{psi_mod});
    auto pt_phi = cc->MakePackedPlaintext(std::vector<int64_t>{phi_mod});
    auto pt_one = cc->MakePackedPlaintext(std::vector<int64_t>{1});
    
    auto ct_psi = cc->Encrypt(kp.publicKey, pt_psi);
    auto ct_phi = cc->Encrypt(kp.publicKey, pt_phi);
    auto ct_one = cc->Encrypt(kp.publicKey, pt_one);

    std::cout << "  Testing φ-cycle in BFV encrypted domain:\n";
    std::cout << "  (multiplying by φ repeatedly, tracking value)\n\n";
    
    auto ct_state = ct_phi; // start with φ (bit 1)
    
    std::cout << "  Step | Decrypted value mod 199\n";
    std::cout << "  -----------------------------\n";
    
    for (int step = 0; step <= 25; step++) {
        Plaintext pt;
        cc->Decrypt(kp.secretKey, ct_state, &pt);
        int64_t decrypted = pt->GetPackedValue()[0];
        
        std::cout << "  " << std::setw(4) << step << " | " << std::setw(3) << decrypted;
        
        if (decrypted == phi_mod) {
            std::cout << " = φ";
        } else if (decrypted == psi_mod) {
            std::cout << " = ψ";
        } else if (decrypted == 1) {
            std::cout << " = 1";
        }
        
        if (step == 22 && decrypted == phi_mod) {
            std::cout << " *** CYCLE BACK AT 22!";
        }
        if (step == 22 && decrypted == 1) {
            std::cout << " *** φ^22 = 1 CONFIRMED";
        }
        std::cout << "\n";
        
        // Multiply by φ for next step
        if (step < 25) {
            ct_state = cc->EvalMult(ct_state, ct_phi);
        }
    }
    
    // Now test NAND using BFV with cyclic modulus
    std::cout << "\n  Testing NAND with φ-cycle modulus:\n";
    std::cout << "  NAND(φ, φ) should give ψ (1 NAND 1 = 0)\n";
    
    // NAND via: result = 1 - (a * b) / φ^2? 
    // In φ-ring: ψ AND ψ = ψ, ψ AND φ = ψ, φ AND ψ = ψ, φ AND φ = φ
    // NAND = 1 - AND (in mod 199, 1 is the multiplicative identity)
    // But we need threshold decoding...
    
    auto ct_and = cc->EvalMult(ct_phi, ct_phi);
    Plaintext pt_and;
    cc->Decrypt(kp.secretKey, ct_and, &pt_and);
    int64_t and_val = pt_and->GetPackedValue()[0];
    std::cout << "  φ * φ mod 199 = " << and_val << " (should be φ^2 mod 199)\n";
    
    // NAND = ψ + φ - AND? Or some linear combination
    // Actually: in {ψ, φ} encoding, AND = multiply, NAND = 1 - multiply (in some basis)
    // This needs the φ-ring structure, not just modular arithmetic

    std::cout << "\n  KEY INSIGHT:\n";
    std::cout << "  In BFV with plaintext modulus 199:\n";
    std::cout << "  - φ^22 ≡ 1, so 22 multiplications form a cycle\n";
    std::cout << "  - After 22 EvalMults, value returns to original\n";
    std::cout << "  - Level consumption: 22 levels per cycle\n";
    std::cout << "  - With depth 50, we get 2 full cycles = 44 gates\n";
    std::cout << "  - For infinite depth: need to reset level after each cycle\n";
    std::cout << "  - But the VALUE cycles! The LEVEL doesn't... yet\n\n";

    return 0;
}
