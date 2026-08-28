// PHI MIN AND — Tamang AND sa φ-Space
// min(a,b) = (a + b - |a - b|) / 2
// |x| = x kung x ≥ 0, -x kung x < 0

#include "openfhe.h"
#include <iostream>
#include <vector>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  PHI MIN AND\n";
    std::cout << "  Tamang AND sa φ-Space\n";
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

    // Sa φ-space, ang |a-b| ay:
    // Kung pareho: 0
    // Kung magkaiba: ±φ (need positive φ)
    
    // Natural na |x| sa φ-space:
    // |x| = NOT(x) kung x < 0
    // |x| = x kung x ≥ 0
    
    // Pero hindi natin kaya ang conditional sa FHE
    // Kaya subukan ang algebraic approach:
    
    // min(a,b) = a kung a at b ay pareho at φ
    // min(a,b) = 0 kung magkaiba o parehong 0
    
    // Sa φ-space: AND(a,b) = (a/φ) * (b/φ) * φ
    // Ito ay multiplication — level cost
    
    // Alternative: AND = NOR(NOT(a), NOT(b))
    // NOR(a,b) = NOT(OR(a,b))
    // OR(a,b) = NOT(AND(NOT(a), NOT(b)))
    
    // Ang pinakasimple: sa φ-space, ang AND ay:
    // AND(a,b) = φ kung a=b=φ, 0 kung hindi
    
    // Ito ay maaaring ma-encode bilang:
    // AND(a,b) = φ * (a == φ && b == φ)
    
    // Sa algebraic form:
    // AND(a,b) = φ * ((a/φ) * (b/φ))
    // = a * b / φ
    
    // Ito ay multiplication sa a at b, division sa φ
    
    // Subukan: AND = a * b / φ
    // Sa FHE: multiplication lang (walang division)
    // Kaya: AND ≈ a * b (kung φ² = φ + 1)
    
    auto eval_and_mult = [&](auto a, auto b) {
        return cc->EvalMult(a, b);
    };

    std::cout << "AND VIA MULTIPLICATION:\n";
    std::cout << "======================\n\n";
    std::cout << "  AND(0,0) = " << decrypt_val(eval_and_mult(ct_0, ct_0)) << " (dapat 0)\n";
    std::cout << "  AND(0,φ) = " << decrypt_val(eval_and_mult(ct_0, ct_phi)) << " (dapat 0)\n";
    std::cout << "  AND(φ,φ) = " << decrypt_val(eval_and_mult(ct_phi, ct_phi)) << " (dapat φ)\n";
    std::cout << "  Level: " << eval_and_mult(ct_phi, ct_phi)->GetLevel() << "\n";

    return 0;
}
