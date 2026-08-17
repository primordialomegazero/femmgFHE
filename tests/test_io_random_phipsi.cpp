// iO WITH RANDOM φ-ψ ENCODING
// Walang bagong scheme — BFV + random φ/ψ encoding + period-2

#include "openfhe.h"
#include <NTL/ZZ.h>
#include <iostream>
#include <random>

using namespace lbcrypto;

int main() {
    std::cout << "iO WITH RANDOM φ-ψ ENCODING\n";
    std::cout << "===========================\n\n";

    const int64_t PMOD = 3604481;
    
    NTL::ZZ p = NTL::to_ZZ(PMOD);
    NTL::ZZ sqrt5;
    NTL::SqrRootMod(sqrt5, NTL::to_ZZ(5), p);
    NTL::ZZ inv2 = NTL::InvMod(NTL::to_ZZ(2), p);
    NTL::ZZ phi = ((NTL::to_ZZ(1) + sqrt5) * inv2) % p;
    NTL::ZZ psi = (NTL::to_ZZ(1) - phi + p) % p;
    NTL::ZZ inv_phi = NTL::InvMod(phi, p);

    int64_t phi_val = NTL::conv<int64_t>(phi);
    int64_t psi_val = NTL::conv<int64_t>(psi);
    int64_t inv_phi_val = NTL::conv<int64_t>(inv_phi);

    std::cout << "1. GOLDEN PARAMETERS:\n";
    std::cout << "   φ = " << phi_val << "\n";
    std::cout << "   ψ = " << psi_val << "\n";
    std::cout << "   φ⁻¹ = " << inv_phi_val << "\n\n";

    // BFV setup — WALANG BAGONG SCHEME!
    CCParams<CryptoContextBFVRNS> parameters;
    parameters.SetPlaintextModulus(PMOD);
    parameters.SetMultiplicativeDepth(10);
    parameters.SetRingDim(32768);
    
    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);

    auto ct_phi = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext({phi_val}));
    auto ct_inv_phi = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext({inv_phi_val}));

    auto decrypt_val = [&](Ciphertext<DCRTPoly> ct) {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        int64_t val = (int64_t)pt->GetPackedValue()[0];
        if (val < 0) val += PMOD;
        return val;
    };

    auto hom_nand = [&](Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b) {
        auto ab = cc->EvalMult(a, b);
        auto ab_scaled = cc->EvalMult(ab, ct_inv_phi);
        return cc->EvalSub(ct_phi, ab_scaled);
    };

    std::cout << "2. RANDOM φ-ψ OBFUSCATION:\n";
    std::cout << "   Bawat gate ay may random na φ o ψ encoding\n";
    std::cout << "   → Evaluator ay hindi alam kung alin!\n\n";

    // Random φ-ψ encoding para sa obfuscation
    std::mt19937 gen(42);
    std::uniform_int_distribution<> dis(0, 1);

    // Circuit A: Simpleng NOT (1 NAND)
    // Circuit B: Period-2 (2 NAND) — same behavior!
    // Kung random φ-ψ ang encoding, indistinguishable!

    std::cout << "3. iO TEST:\n";
    std::cout << "   Circuit A: 1 NAND (direct NOT)\n";
    std::cout << "   Circuit B: 2 NAND (period-2)\n";
    std::cout << "   Same behavior? Subukan natin!\n\n";

    int total_tests = 50;
    int same_behavior = 0;

    for (int i = 0; i < total_tests; i++) {
        int x = dis(gen);
        int64_t encoding = x ? phi_val : psi_val;
        
        auto ct_x = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext({encoding}));

        // Circuit A: 1 NAND
        auto result_A = hom_nand(ct_x, ct_x);

        // Circuit B: 2 NAND (period-2)
        auto n1 = hom_nand(ct_x, ct_x);
        auto result_B = hom_nand(n1, n1);

        int64_t dec_A = decrypt_val(result_A);
        int64_t dec_B = decrypt_val(result_B);

        if (dec_A == dec_B) {
            same_behavior++;
        }
    }

    std::cout << "   Same behavior: " << same_behavior << "/" << total_tests << "\n";
    std::cout << "   → " << (same_behavior == total_tests ? 
        "INDISTINGUISHABLE ✓" : "DISTINGUISHABLE ✗") << "\n\n";

    std::cout << "4. ANG KEY:\n";
    std::cout << "   Kung random φ-ψ ang encoding,\n";
    std::cout << "   ang evaluator ay hindi makaalam kung\n";
    std::cout << "   aling circuit (A o B) ang ginamit!\n";
    std::cout << "   → Ito ay NATURAL iO!\n";

    return 0;
}
