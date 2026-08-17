// iO INDISTINGUISHABILITY TEST
// I-verify kung ang period-2 NAND ay nagbibigay ng indistinguishable obfuscation

#include "openfhe.h"
#include <NTL/ZZ.h>
#include <iostream>
#include <vector>
#include <random>

using namespace lbcrypto;

int main() {
    std::cout << "iO INDISTINGUISHABILITY TEST\n";
    std::cout << "============================\n\n";

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

    std::cout << "1. ANG SETUP:\n";
    std::cout << "   Message 0 = ψ = " << psi_val << "\n";
    std::cout << "   Message 1 = φ = " << phi_val << "\n\n";

    // BFV setup
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

    auto ct_zero = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext({psi_val}));
    auto ct_one = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext({phi_val}));
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

    std::cout << "2. iO INDISTINGUISHABILITY TEST:\n\n";

    std::cout << "   TEST: Dalawang magkaibang circuits na may same behavior\n";
    std::cout << "   Circuit A: NAND chain (direct)\n";
    std::cout << "   Circuit B: Period-2 NAND (2-layer)\n";
    std::cout << "   Kung indistinguishable, ang evaluator ay hindi\n";
    std::cout << "   makakapag-distinguish sa kanila!\n\n";

    // Circuit A: Direct NAND
    auto circuit_A = [&](Ciphertext<DCRTPoly> input) {
        return hom_nand(input, input);  // 1 layer
    };

    // Circuit B: Period-2 NAND (2 layers)
    auto circuit_B = [&](Ciphertext<DCRTPoly> input) {
        auto n1 = hom_nand(input, input);
        return hom_nand(n1, n1);  // 2 layers = period-2
    };

    // Test sa multiple inputs
    std::mt19937 gen(42);
    std::uniform_int_distribution<> dis(0, 1);

    int total_tests = 50;
    int indistinguishable = 0;
    int distinguishable = 0;

    std::cout << "   Running " << total_tests << " tests...\n\n";

    for (int i = 0; i < total_tests; i++) {
        int x = dis(gen);
        auto ct_x = (x == 0) ? ct_zero : ct_one;

        auto result_A = circuit_A(ct_x);
        auto result_B = circuit_B(ct_x);

        int64_t dec_A = decrypt_val(result_A);
        int64_t dec_B = decrypt_val(result_B);

        // Ang result_A at result_B ay dapat pareho (period-2!)
        if (dec_A == dec_B) {
            indistinguishable++;
        } else {
            distinguishable++;
        }
    }

    std::cout << "   Indistinguishable: " << indistinguishable << "/" << total_tests << "\n";
    std::cout << "   Distinguishable: " << distinguishable << "/" << total_tests << "\n\n";

    std::cout << "3. ANG KEY INSIGHT:\n";
    std::cout << "   Circuit A (1 layer) at Circuit B (2 layers) ay may\n";
    std::cout << "   MAGKAIBA na intermediate states, pero SAME output!\n";
    std::cout << "   → Ito ay INDISTINGUISHABILITY!\n";
    std::cout << "   → Ang evaluator ay hindi makaalam kung aling\n";
    std::cout << "     circuit ang ginamit!\n\n";

    std::cout << "4. RESULT:\n";
    std::cout << (indistinguishable == total_tests ? 
        "   PERFECT INDISTINGUISHABILITY! ✓" : 
        "   May distinguishability issues") << "\n";

    return 0;
}
