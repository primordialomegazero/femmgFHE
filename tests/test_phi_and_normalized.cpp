// PHI AND NORMALIZED — Tamang AND sa φ-Space
// AND(a,b) = a * b / φ
// Ito ay nagbibigay ng eksaktong φ values

#include "openfhe.h"
#include <iostream>
#include <vector>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  PHI AND NORMALIZED\n";
    std::cout << "  AND = a * b / φ\n";
    std::cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double PHI_INV = 1.0 / PHI;

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(2);
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
    auto ct_phi_inv = make_ct(PHI_INV);

    // AND = a * b * (1/φ)
    auto eval_and = [&](auto a, auto b) {
        auto mult = cc->EvalMult(a, b);
        return cc->EvalMult(mult, ct_phi_inv);
    };

    // NOT = φ - x
    auto eval_not = [&](auto x) {
        auto ct_phi_local = make_ct(PHI);
        return cc->EvalSub(ct_phi_local, x);
    };

    // NAND = NOT(AND)
    auto eval_nand = [&](auto a, auto b) {
        auto and_ab = eval_and(a, b);
        return eval_not(and_ab);
    };

    // OR = NOT(AND(NOT(a), NOT(b)))
    auto eval_or = [&](auto a, auto b) {
        auto not_a = eval_not(a);
        auto not_b = eval_not(b);
        auto and_nots = eval_and(not_a, not_b);
        return eval_not(and_nots);
    };

    std::cout << "AND TEST:\n";
    std::cout << "=========\n\n";
    std::cout << "  AND(0,0) = " << decrypt_val(eval_and(ct_0, ct_0)) << " (dapat 0)\n";
    std::cout << "  AND(0,φ) = " << decrypt_val(eval_and(ct_0, ct_phi)) << " (dapat 0)\n";
    std::cout << "  AND(φ,φ) = " << decrypt_val(eval_and(ct_phi, ct_phi)) << " (dapat " << PHI << ")\n\n";

    std::cout << "NAND TEST:\n";
    std::cout << "==========\n\n";
    std::cout << "  NAND(0,0) = " << decrypt_val(eval_nand(ct_0, ct_0)) << " (dapat φ)\n";
    std::cout << "  NAND(0,φ) = " << decrypt_val(eval_nand(ct_0, ct_phi)) << " (dapat φ)\n";
    std::cout << "  NAND(φ,φ) = " << decrypt_val(eval_nand(ct_phi, ct_phi)) << " (dapat 0)\n\n";

    std::cout << "OR TEST:\n";
    std::cout << "========\n\n";
    std::cout << "  OR(0,0) = " << decrypt_val(eval_or(ct_0, ct_0)) << " (dapat 0)\n";
    std::cout << "  OR(0,φ) = " << decrypt_val(eval_or(ct_0, ct_phi)) << " (dapat φ)\n";
    std::cout << "  OR(φ,φ) = " << decrypt_val(eval_or(ct_phi, ct_phi)) << " (dapat φ)\n";

    return 0;
}
