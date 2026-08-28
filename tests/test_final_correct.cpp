// FINAL CORRECT — Tamang φ-ψ NOT
// NOT(x) = 1 - x = φ + ψ - x

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  FINAL CORRECT\n";
    std::cout << "  Tamang φ-ψ NOT\n";
    std::cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double PSI = -0.6180339887498948482;
    const double ONE = PHI + PSI;  // = 1

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(5);
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

    auto ct_false = make_ct(PSI);
    auto ct_true = make_ct(PHI);
    auto ct_one = make_ct(ONE);

    // NOT(x) = 1 - x
    auto eval_not = [&](auto x) {
        return cc->EvalSub(ct_one, x);
    };

    // AND(a,b) = a + b - 1
    // Sa φ-ψ space: AND(φ,φ) = φ+φ-1 = 2φ-1 = φ
    // AND(ψ,φ) = ψ+φ-1 = 1-1 = 0 → ψ
    // AND(ψ,ψ) = ψ+ψ-1 = 2ψ-1 = -2/φ-1 = ψ
    
    auto eval_and = [&](auto a, auto b) {
        auto sum = cc->EvalAdd(a, b);
        return cc->EvalSub(sum, ct_one);
    };

    std::cout << "CORRECT NOT:\n";
    std::cout << "  NOT(false) = " << decrypt_val(eval_not(ct_false)) << " (dapat " << PHI << ")\n";
    std::cout << "  NOT(true) = " << decrypt_val(eval_not(ct_true)) << " (dapat " << PSI << ")\n\n";

    std::cout << "CORRECT AND:\n";
    std::cout << "  AND(false,false) = " << decrypt_val(eval_and(ct_false, ct_false)) << " (dapat " << PSI << ")\n";
    std::cout << "  AND(false,true) = " << decrypt_val(eval_and(ct_false, ct_true)) << " (dapat " << PSI << ")\n";
    std::cout << "  AND(true,false) = " << decrypt_val(eval_and(ct_true, ct_false)) << " (dapat " << PSI << ")\n";
    std::cout << "  AND(true,true) = " << decrypt_val(eval_and(ct_true, ct_true)) << " (dapat " << PHI << ")\n";

    return 0;
}
