// ALL GATES — Minimal Natural Composition
// XOR, NAND, NOR direct (4/4 na)
// AND, OR via minimal composition
//
// AND = NAND(NOT(a), NOT(b))
// OR = NOT(NAND(a,b))
//
// Sa pentagonal encoding ±2π/5:
// NOT(x) = x + 2π/5 (natural pentagon rotation)

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  ALL GATES — Minimal Composition\n";
    std::cout << "  Pentagonal ±2π/5 Encoding\n";
    std::cout << "========================================\n\n";

    const double phi = 1.618033988749895;
    const double PI = 3.14159265358979323846;
    const double ENC_1 = 2 * PI / 5;

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(20);
    params.SetScalingModSize(50);
    params.SetBatchSize(256);
    params.SetFirstModSize(60);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(params);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    cc->Enable(ADVANCEDSHE);

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

    // Direct gates
    auto eval_xor = [&](auto ct_a, auto ct_b) {
        auto sum = cc->EvalAdd(ct_a, ct_b);
        return cc->EvalCos(sum, -4.0, 4.0, 15);
    };

    auto eval_nand = [&](auto ct_a, auto ct_b) {
        auto sum = cc->EvalAdd(ct_a, ct_b);
        auto shift = make_ct(2*PI/5);
        auto shifted = cc->EvalAdd(sum, shift);
        return cc->EvalCos(shifted, -4.0, 4.0, 15);
    };

    auto eval_nor = [&](auto ct_a, auto ct_b) {
        auto sum = cc->EvalAdd(ct_a, ct_b);
        auto shift = make_ct(3*PI/5);
        auto shifted = cc->EvalAdd(sum, shift);
        return cc->EvalCos(shifted, -4.0, 4.0, 15);
    };

    std::cout << "DIRECT: XOR, NAND, NOR\n";
    std::cout << "COMPOSED: AND, OR\n\n";

    int total = 0;

    // XOR direct
    int xor_c = 0;
    for (int a = 0; a <= 1; a++) {
        for (int b = 0; b <= 1; b++) {
            auto ct_a = make_ct(a ? ENC_1 : -ENC_1);
            auto ct_b = make_ct(b ? ENC_1 : -ENC_1);
            auto result = eval_xor(ct_a, ct_b);
            double val = decrypt_val(result);
            int got = (val > 0) ? 1 : 0;
            if (got == (a ^ b)) xor_c++;
        }
    }
    std::cout << "XOR: " << xor_c << "/4\n";
    total += xor_c;

    // NAND direct
    int nand_c = 0;
    for (int a = 0; a <= 1; a++) {
        for (int b = 0; b <= 1; b++) {
            auto ct_a = make_ct(a ? ENC_1 : -ENC_1);
            auto ct_b = make_ct(b ? ENC_1 : -ENC_1);
            auto result = eval_nand(ct_a, ct_b);
            double val = decrypt_val(result);
            int got = (val > 0) ? 1 : 0;
            if (got == !(a && b)) nand_c++;
        }
    }
    std::cout << "NAND: " << nand_c << "/4\n";
    total += nand_c;

    // NOR direct
    int nor_c = 0;
    for (int a = 0; a <= 1; a++) {
        for (int b = 0; b <= 1; b++) {
            auto ct_a = make_ct(a ? ENC_1 : -ENC_1);
            auto ct_b = make_ct(b ? ENC_1 : -ENC_1);
            auto result = eval_nor(ct_a, ct_b);
            double val = decrypt_val(result);
            int got = (val > 0) ? 1 : 0;
            if (got == !(a || b)) nor_c++;
        }
    }
    std::cout << "NOR: " << nor_c << "/4\n";
    total += nor_c;

    // AND via NAND(NOT(a), NOT(b))
    int and_c = 0;
    for (int a = 0; a <= 1; a++) {
        for (int b = 0; b <= 1; b++) {
            // NOT(x) = x + π (phase inversion)
            auto ct_a = make_ct(a ? ENC_1 : -ENC_1);
            auto ct_b = make_ct(b ? ENC_1 : -ENC_1);
            auto not_a = cc->EvalAdd(ct_a, make_ct(PI));
            auto not_b = cc->EvalAdd(ct_b, make_ct(PI));
            auto result = eval_nand(not_a, not_b);
            double val = decrypt_val(result);
            int got = (val > 0) ? 1 : 0;
            if (got == (a && b)) and_c++;
        }
    }
    std::cout << "AND: " << and_c << "/4\n";
    total += and_c;

    // OR via NOT(NAND(a,b))
    int or_c = 0;
    for (int a = 0; a <= 1; a++) {
        for (int b = 0; b <= 1; b++) {
            auto ct_a = make_ct(a ? ENC_1 : -ENC_1);
            auto ct_b = make_ct(b ? ENC_1 : -ENC_1);
            auto nand_result = eval_nand(ct_a, ct_b);
            auto or_result = cc->EvalAdd(nand_result, make_ct(PI));
            double val = decrypt_val(or_result);
            int got = (val > 0) ? 1 : 0;
            if (got == (a || b)) or_c++;
        }
    }
    std::cout << "OR: " << or_c << "/4\n";
    total += or_c;

    std::cout << "\n========================================\n";
    std::cout << "  TOTAL: " << total << "/20\n";
    std::cout << "  STATUS: " << (total == 20 ? "✅ LAHAT PERFECT!" : "⚠️ NEEDS TUNING") << "\n";
    std::cout << "========================================\n";

    return 0;
}
