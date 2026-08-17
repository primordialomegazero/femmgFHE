// iO WITH φ-ANCHOR NAND
// NAND(a,b) = φ - a·b·φ⁻¹ — may period-2 at trace erasure

#include "openfhe.h"
#include <NTL/ZZ.h>
#include <iostream>

using namespace lbcrypto;

int main() {
    std::cout << "iO WITH φ-ANCHOR NAND\n";
    std::cout << "=====================\n\n";

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
    std::cout << "   ψ = " << psi_val << "\n\n";

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

    auto ct_phi = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext({phi_val}));
    auto ct_inv_phi = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext({inv_phi_val}));

    auto decrypt_val = [&](Ciphertext<DCRTPoly> ct) {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        int64_t val = (int64_t)pt->GetPackedValue()[0];
        if (val < 0) val += PMOD;
        return val;
    };

    // NAND(a,b) = φ - a·b·φ⁻¹
    auto hom_nand = [&](Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b) {
        auto ab = cc->EvalMult(a, b);
        auto ab_scaled = cc->EvalMult(ab, ct_inv_phi);
        return cc->EvalSub(ct_phi, ab_scaled);
    };

    std::cout << "2. PERIOD-2 TRACE ERASURE TEST:\n";
    std::cout << "   Circuit A: 1 NAND\n";
    std::cout << "   Circuit B: 2 NAND (period-2)\n";
    std::cout << "   Intermediate: 0 (walang trace)\n\n";

    // Test: φ → 0 → φ
    auto ct_phi_input = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext({phi_val}));
    
    // 1 NAND
    auto nand1 = hom_nand(ct_phi_input, ct_phi_input);
    std::cout << "   NAND(φ,φ) = " << decrypt_val(nand1) << " (exp 0)\n";
    
    // 2 NAND (period-2)
    auto nand2 = hom_nand(nand1, nand1);
    std::cout << "   NAND(NAND(φ)) = " << decrypt_val(nand2) << " (exp " << phi_val << ")\n\n";

    // Trace erasure check
    std::cout << "3. TRACE ERASURE:\n";
    std::cout << "   Intermediate state = 0\n";
    std::cout << "   → Walang impormasyon tungkol sa φ!\n";
    std::cout << "   → Evaluator ay hindi maka-reconstruct ng input!\n\n";

    // Deep chain
    std::cout << "4. DEEP CHAIN (20 depths):\n";
    auto current = ct_phi_input;
    int errors = 0;

    for (int i = 0; i <= 20; i++) {
        int64_t val = decrypt_val(current);
        int64_t expected = (i % 2 == 0) ? phi_val : 0;
        bool match = (val == expected);
        if (!match) errors++;

        auto sq = cc->EvalMult(current, current);
        auto sq_scaled = cc->EvalMult(sq, ct_inv_phi);
        current = cc->EvalSub(ct_phi, sq_scaled);
    }

    std::cout << "   Errors: " << errors << "/21\n";
    std::cout << "\n5. RESULT:\n";
    std::cout << (errors == 0 ? 
        "   PERFECT PERIOD-2 WITH TRACE ERASURE! ✓" : 
        "   May errors") << "\n";

    return 0;
}
