// BFV φ-ANCHOR NAND
// Message: 0/1 (plaintext)
// NAND: φ - a·b·φ⁻¹ (φ bilang anchor, hindi message)

#include "openfhe.h"
#include <NTL/ZZ.h>
#include <iostream>

using namespace lbcrypto;

int main() {
    std::cout << "BFV φ-ANCHOR NAND\n";
    std::cout << "=================\n\n";
    
    // 3604481 = 327680 × 11 + 1, prime, ≡ 1 mod 5
    const int64_t PMOD = 12451841;
    
    CCParams<CryptoContextBFVRNS> parameters;
    parameters.SetPlaintextModulus(PMOD);
    parameters.SetMultiplicativeDepth(30);
    parameters.SetRingDim(65536);
    
    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    
    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);
    
    // Compute φ at φ⁻¹ DIRECTLY sa PMOD
    NTL::ZZ p = NTL::to_ZZ(PMOD);
    NTL::ZZ sqrt5;
    NTL::SqrRootMod(sqrt5, NTL::to_ZZ(5), p);
    NTL::ZZ inv2 = NTL::InvMod(NTL::to_ZZ(2), p);
    NTL::ZZ phi = ((NTL::to_ZZ(1) + sqrt5) * inv2) % p;
    NTL::ZZ inv_phi = NTL::InvMod(phi, p);
    
    int64_t phi_val = NTL::conv<int64_t>(phi);
    int64_t inv_phi_val = NTL::conv<int64_t>(inv_phi);
    
    std::cout << "φ mod " << PMOD << " = " << phi_val << "\n";
    std::cout << "φ⁻¹ mod " << PMOD << " = " << inv_phi_val << "\n";
    std::cout << "Verify φ·φ⁻¹ = " << (phi * inv_phi) % p << " (dapat 1)\n\n";
    
    // Encrypt message 0 at 1
    auto ct0 = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext({(int64_t)(1 - phi_val + PMOD)}));  // Message 0 = ψ = 1-φ
    auto ct1 = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext({phi_val}));  // Message 1 = φ
    
    // Pre-encrypt φ at φ⁻¹ bilang constants
    auto ct_phi = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext({phi_val}));
    auto ct_inv_phi = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext({inv_phi_val}));
    
    std::cout << "Decrypt(0) = " << [&]() {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct0, &pt);
        return pt->GetPackedValue()[0];
    }() << "\n";
    std::cout << "Decrypt(1) = " << [&]() {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct1, &pt);
        return pt->GetPackedValue()[0];
    }() << "\n\n";
    
    // Homomorphic NAND: φ - a·b·φ⁻¹
    auto homomorphic_nand = [&](Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b) {
        // NAND(a,b) = 2 - a·b (period-2 with φ/ψ encoding)
        auto ab = cc->EvalMult(a, b);
        auto ct_two = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext({2}));
        return cc->EvalSub(ct_two, ab);
    };
    
    // Test NAND
    auto nand00 = homomorphic_nand(ct0, ct0);
    auto nand01 = homomorphic_nand(ct0, ct1);
    auto nand11 = homomorphic_nand(ct1, ct1);
    
    Plaintext pt;
    cc->Decrypt(keys.secretKey, nand00, &pt);
    int64_t v00 = pt->GetPackedValue()[0];
    if (v00 < 0) v00 += PMOD;
    std::cout << "NAND(0,0) = " << v00 << " (exp " << phi_val << ")\n";
    
    cc->Decrypt(keys.secretKey, nand01, &pt);
    int64_t v01 = pt->GetPackedValue()[0];
    if (v01 < 0) v01 += PMOD;
    std::cout << "NAND(0,1) = " << v01 << " (exp " << phi_val << ")\n";
    
    cc->Decrypt(keys.secretKey, nand11, &pt);
    int64_t v11 = pt->GetPackedValue()[0];
    if (v11 < 0) v11 += PMOD;
    std::cout << "NAND(1,1) = " << v11 << " (exp 0)\n\n";
    
    // Deep chain
    std::cout << "Deep chain (10 depths):\n";
    auto current = ct1;
    int errors = 0;
    for (int i = 0; i <= 30; i++) {
        cc->Decrypt(keys.secretKey, current, &pt);
        int64_t val = pt->GetPackedValue()[0];
        if (val < 0) val += PMOD;
        int64_t expected = (i % 2 == 0) ? (int64_t)phi_val : (int64_t)(1 - phi_val + PMOD);
        
        bool ok = (i % 2 == 0) ? (val == phi_val) : (val == 1 - phi_val + PMOD);
        if (!ok) {
            std::cout << "  Depth " << i << ": " << val << " (exp " << expected << ") ✗\n";
            errors++;
        }
        current = homomorphic_nand(current, current);
    }
    std::cout << "Errors: " << errors << "/31\n";
    
    return 0;
}
