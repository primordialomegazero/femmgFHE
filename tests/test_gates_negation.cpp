// ALL GATES — Correct Negation NOT
// NOT(x) = -x (hindi +π)
// Pentagonal encoding ±2π/5
//
// AND = NAND(NOT(a), NOT(b)) = NAND(-a, -b)
// OR = NOT(NAND(a,b)) = -NAND(a,b)
//
// Lahat FHE, walang decrypt sa gitna

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  ALL GATES — Correct Negation\n";
    std::cout << "  NOT(x) = -x (natural)\n";
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

    // NOT(x) = -x (negation)
    auto eval_not = [&](auto ct_x) {
        return cc->EvalNegate(ct_x);
    };

    std::cout << "DIRECT: XOR, NAND, NOR\n";
    std::cout << "COMPOSED: AND, OR\n";
    std::cout << "NOT(x) = -x\n\n";

    int total = 0;

    // XOR direct
    int xor_c = 0;
    std::cout << "XOR (direct):\n";
    for (int a = 0; a <= 1; a++) {
        for (int b = 0; b <= 1; b++) {
            auto ct_a = make_ct(a ? ENC_1 : -ENC_1);
            auto ct_b = make_ct(b ? ENC_1 : -ENC_1);
            auto result = eval_xor(ct_a, ct_b);
            double val = decrypt_val(result);
            int got = (val > 0) ? 1 : 0;
            int expected = a ^ b;
            if (got == expected) xor_c++;
            std::cout << "  XOR(" << a << "," << b << ") = " << expected 
                      << " → " << got << " (cos=" << val << ")"
                      << (got == expected ? " ✓" : " ✗") << "\n";
        }
    }
    std::cout << "  XOR: " << xor_c << "/4\n\n";
    total += xor_c;

    // NAND direct
    int nand_c = 0;
    std::cout << "NAND (direct):\n";
    for (int a = 0; a <= 1; a++) {
        for (int b = 0; b <= 1; b++) {
            auto ct_a = make_ct(a ? ENC_1 : -ENC_1);
            auto ct_b = make_ct(b ? ENC_1 : -ENC_1);
            auto result = eval_nand(ct_a, ct_b);
            double val = decrypt_val(result);
            int got = (val > 0) ? 1 : 0;
            int expected = !(a && b);
            if (got == expected) nand_c++;
            std::cout << "  NAND(" << a << "," << b << ") = " << expected 
                      << " → " << got << " (cos=" << val << ")"
                      << (got == expected ? " ✓" : " ✗") << "\n";
        }
    }
    std::cout << "  NAND: " << nand_c << "/4\n\n";
    total += nand_c;

    // NOR direct
    int nor_c = 0;
    std::cout << "NOR (direct):\n";
    for (int a = 0; a <= 1; a++) {
        for (int b = 0; b <= 1; b++) {
            auto ct_a = make_ct(a ? ENC_1 : -ENC_1);
            auto ct_b = make_ct(b ? ENC_1 : -ENC_1);
            auto result = eval_nor(ct_a, ct_b);
            double val = decrypt_val(result);
            int got = (val > 0) ? 1 : 0;
            int expected = !(a || b);
            if (got == expected) nor_c++;
            std::cout << "  NOR(" << a << "," << b << ") = " << expected 
                      << " → " << got << " (cos=" << val << ")"
                      << (got == expected ? " ✓" : " ✗") << "\n";
        }
    }
    std::cout << "  NOR: " << nor_c << "/4\n\n";
    total += nor_c;

    // AND = NAND(NOT(a), NOT(b)) = NAND(-a, -b)
    int and_c = 0;
    std::cout << "AND (via NAND(-a,-b)):\n";
    for (int a = 0; a <= 1; a++) {
        for (int b = 0; b <= 1; b++) {
            auto ct_a = make_ct(a ? ENC_1 : -ENC_1);
            auto ct_b = make_ct(b ? ENC_1 : -ENC_1);
            auto not_a = eval_not(ct_a);
            auto not_b = eval_not(ct_b);
            auto result = eval_nand(not_a, not_b);
            double val = decrypt_val(result);
            int got = (val > 0) ? 1 : 0;
            int expected = a && b;
            if (got == expected) and_c++;
            std::cout << "  AND(" << a << "," << b << ") = " << expected 
                      << " → " << got << " (cos=" << val << ")"
                      << (got == expected ? " ✓" : " ✗") << "\n";
        }
    }
    std::cout << "  AND: " << and_c << "/4\n\n";
    total += and_c;

    // OR = NOT(NAND(a,b)) = -NAND(a,b)
    int or_c = 0;
    std::cout << "OR (via -NAND(a,b)):\n";
    for (int a = 0; a <= 1; a++) {
        for (int b = 0; b <= 1; b++) {
            auto ct_a = make_ct(a ? ENC_1 : -ENC_1);
            auto ct_b = make_ct(b ? ENC_1 : -ENC_1);
            auto nand_result = eval_nand(ct_a, ct_b);
            auto result = eval_not(nand_result);
            double val = decrypt_val(result);
            int got = (val > 0) ? 1 : 0;
            int expected = a || b;
            if (got == expected) or_c++;
            std::cout << "  OR(" << a << "," << b << ") = " << expected 
                      << " → " << got << " (cos=" << val << ")"
                      << (got == expected ? " ✓" : " ✗") << "\n";
        }
    }
    std::cout << "  OR: " << or_c << "/4\n\n";
    total += or_c;

    std::cout << "========================================\n";
    std::cout << "  TOTAL: " << total << "/20\n";
    std::cout << "  STATUS: " << (total == 20 ? "✅ LAHAT PERFECT!" : "⚠️ NEEDS TUNING") << "\n";
    std::cout << "========================================\n";

    return 0;
}
