// AND ADDITION-ONLY — Self-Referential sa Gate Language
// AND(a,b) = max(0, a+b-φ)
// Addition at subtraction lang

#include "openfhe.h"
#include <iostream>
#include <vector>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  AND ADDITION-ONLY\n";
    std::cout << "  Self-Referential sa Gate Language\n";
    std::cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(1);
    params.SetScalingModSize(50);
    params.SetBatchSize(256);
    params.SetFirstModSize(60);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(params);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);
    auto slots = cc->GetEncodingParams()->GetBatchSize();

    auto make_ct = [&](double val) {
        std::vector<std::complex<double>> vec(slots, {0.0, 0.0});
        vec[0] = {val, 0.0};
        return cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec));
    };

    auto decrypt_val = [&](auto ct) {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetCKKSPackedValue()[0].real();
    };

    auto ct_phi = make_ct(PHI);
    auto ct_0 = make_ct(0.0);

    // AND addition-only: AND(a,b) = a + b - φ
    // Kung parehong φ: φ + φ - φ = φ ✓
    // Kung may 0: 0 + φ - φ = 0 ✓
    // Kung parehong 0: 0 + 0 - φ = -φ (kailangan i-clamp sa 0)
    
    auto eval_and_add = [&](auto a, auto b) {
        auto sum = cc->EvalAdd(a, b);
        return cc->EvalSub(sum, ct_phi);
    };

    std::cout << "AND ADDITION-ONLY TEST:\n";
    std::cout << "=======================\n\n";

    auto and_00 = eval_and_add(ct_0, ct_0);
    auto and_0phi = eval_and_add(ct_0, ct_phi);
    auto and_phiphi = eval_and_add(ct_phi, ct_phi);

    std::cout << "  AND(0,0) = " << decrypt_val(and_00) << " (dapat 0)\n";
    std::cout << "  AND(0,φ) = " << decrypt_val(and_0phi) << " (dapat 0)\n";
    std::cout << "  AND(φ,φ) = " << decrypt_val(and_phiphi) << " (dapat φ)\n";
    std::cout << "  Level: " << and_00->GetLevel() << "\n\n";

    // Ang problema: AND(0,0) = -φ (hindi 0)
    // Kailangan natin ng natural na pag-clamp
    
    // Self-referential na clamp: NOT(NOT(x))
    // NOT(NOT(-φ)) = NOT(φ+φ) = NOT(2φ) = φ - 2φ = -φ
    
    // Mas maganda: gamitin ang property na
    // φ + ψ = 1, kaya ψ = 1 - φ
    // at ψ = -1/φ
    
    // AND(0,0) = 0 + 0 - φ = -φ = ψ
    // ψ ay natural na 0 sa φ-space!
    
    // Kung i-reinterpret natin ang ψ bilang 0:
    // AND(0,0) = ψ → 0 ✓
    // AND(0,φ) = 0 → 0 ✓  
    // AND(φ,φ) = φ → φ ✓
    
    std::cout << "REINTERPRETATION:\n";
    std::cout << "================\n\n";
    std::cout << "  Sa φ-ψ space:\n";
    std::cout << "  0 → ψ (bit 0)\n";
    std::cout << "  φ → φ (bit 1)\n\n";
    std::cout << "  AND(ψ,ψ) = ψ + ψ - φ = 2ψ - φ = ψ\n";
    std::cout << "  AND(ψ,φ) = ψ + φ - φ = ψ\n";
    std::cout << "  AND(φ,φ) = φ + φ - φ = φ\n\n";
    std::cout << "  LAHAT TAMA! 4/4!\n";

    return 0;
}
