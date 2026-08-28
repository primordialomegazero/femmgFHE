// ALL GATES — Complete with Dual Encoding
// XOR/NAND/AND: ±φ/2 encoding
// OR/NOR: ±φ encoding (mas malaking separation)
//
// OR:  shift=0, threshold=-0.5
// OR(0,0) = cos(-2φ) = cos(-3.236) = -0.996 → 0 ✓
// OR(0,1) = cos(0) = 1 → 1 ✓
// OR(1,0) = cos(0) = 1 → 1 ✓
// OR(1,1) = cos(2φ) = cos(3.236) = -0.996 → 1 ✗
//
// HINDI — kailangan ng shift
//
// OR: shift=φ, threshold=-0.5
// OR(0,0) = cos(-φ) = -0.047 → 0 ✓ (threshold -0.5)
// OR(0,1) = cos(φ) = -0.047 → 0 ✗ (dapat 1)
//
// ANG TUNAY NA SOLUSYON:
// OR = 1 kung may kahit isang 1
// Sa ±φ/2 encoding:
// OR(0,0) = cos(-φ) = -0.047 → dapat 0
// OR(0,1) = cos(0) = 1 → dapat 1
// OR(1,1) = cos(φ) = -0.047 → dapat 1
//
// Ang problema: (0,0) at (1,1) ay parehong -0.047
// Kailangan natin ng asymmetric encoding!
//
// ASYMMETRIC ENCODING:
// 0 → -φ/2
// 1 → +3φ/2
//
// OR(0,0) = cos(-φ) = -0.047 → 0
// OR(0,1) = cos(φ) = -0.047 → 0 ✗
//
// HINDI RIN
//
// ANG PINAKA-SIMPLENG SOLUSYON:
// OR = NAND(NOT(a), NOT(b))
// De Morgan's Law!
//
// Kung may NAND na tayo, ang OR ay:
// OR(a,b) = NAND(NOT(a), NOT(b))
//
// At ang NOR ay:
// NOR(a,b) = NOT(OR(a,b))
//
// LAHAT AY PWEDENG I-EXPRESS SA NAND!
// NAND ay functionally complete!

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  ALL GATES — Via NAND Completeness\n";
    std::cout << "  XOR, NAND, AND Direct\n";
    std::cout << "  OR, NOR via De Morgan\n";
    std::cout << "========================================\n\n";

    const double phi = 1.618033988749895;
    const double PI = 3.14159265358979323846;
    const double HALF_PHI = phi / 2;

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

    // NOT(x) = x + π (phase inversion)
    auto eval_not = [&](auto ct_input) {
        auto ct_pi = make_ct(PI);
        return cc->EvalAdd(ct_input, ct_pi);
    };

    // NAND(a,b) = cos(a + b + φ/2) > -0.5
    auto eval_nand = [&](auto ct_a, auto ct_b) {
        auto sum = cc->EvalAdd(ct_a, ct_b);
        auto shift = make_ct(HALF_PHI);
        auto shifted = cc->EvalAdd(sum, shift);
        return cc->EvalCos(shifted, -3.0, 3.0, 15);
    };

    std::cout << "DIRECT GATES (XOR, NAND, AND):\n";
    std::cout << "==============================\n\n";

    int total = 0;

    // XOR direct
    std::cout << "XOR (direct):\n";
    int xor_correct = 0;
    for (int a = 0; a <= 1; a++) {
        for (int b = 0; b <= 1; b++) {
            double angle_a = a ? HALF_PHI : -HALF_PHI;
            double angle_b = b ? HALF_PHI : -HALF_PHI;
            auto ct_a = make_ct(angle_a);
            auto ct_b = make_ct(angle_b);
            auto sum = cc->EvalAdd(ct_a, ct_b);
            auto cos_result = cc->EvalCos(sum, -3.0, 3.0, 15);
            double cos_val = decrypt_val(cos_result);
            int got = (cos_val > 0) ? 1 : 0;
            int expected = a ^ b;
            if (got == expected) xor_correct++;
        }
    }
    std::cout << "  XOR: " << xor_correct << "/4\n";
    total += xor_correct;

    // NAND direct
    std::cout << "NAND (direct):\n";
    int nand_correct = 0;
    for (int a = 0; a <= 1; a++) {
        for (int b = 0; b <= 1; b++) {
            double angle_a = a ? HALF_PHI : -HALF_PHI;
            double angle_b = b ? HALF_PHI : -HALF_PHI;
            auto ct_a = make_ct(angle_a);
            auto ct_b = make_ct(angle_b);
            auto cos_result = eval_nand(ct_a, ct_b);
            double cos_val = decrypt_val(cos_result);
            int got = (cos_val > -0.5) ? 1 : 0;
            int expected = !(a && b);
            if (got == expected) nand_correct++;
        }
    }
    std::cout << "  NAND: " << nand_correct << "/4\n";
    total += nand_correct;

    // AND via NAND + NOT
    std::cout << "AND (via NAND + NOT):\n";
    int and_correct = 0;
    for (int a = 0; a <= 1; a++) {
        for (int b = 0; b <= 1; b++) {
            double angle_a = a ? HALF_PHI : -HALF_PHI;
            double angle_b = b ? HALF_PHI : -HALF_PHI;
            auto ct_a = make_ct(angle_a);
            auto ct_b = make_ct(angle_b);
            auto nand_result = eval_nand(ct_a, ct_b);
            auto not_result = eval_not(nand_result);
            double cos_val = decrypt_val(not_result);
            int got = (cos_val > -0.5) ? 1 : 0;
            int expected = a && b;
            if (got == expected) and_correct++;
        }
    }
    std::cout << "  AND: " << and_correct << "/4\n";
    total += and_correct;

    // OR via NAND(NOT(a), NOT(b))
    std::cout << "OR (via De Morgan):\n";
    int or_correct = 0;
    for (int a = 0; a <= 1; a++) {
        for (int b = 0; b <= 1; b++) {
            double angle_a = a ? HALF_PHI : -HALF_PHI;
            double angle_b = b ? HALF_PHI : -HALF_PHI;
            auto ct_a = make_ct(angle_a);
            auto ct_b = make_ct(angle_b);
            auto not_a = eval_not(ct_a);
            auto not_b = eval_not(ct_b);
            auto or_result = eval_nand(not_a, not_b);
            double cos_val = decrypt_val(or_result);
            int got = (cos_val > -0.5) ? 1 : 0;
            int expected = a || b;
            if (got == expected) or_correct++;
        }
    }
    std::cout << "  OR: " << or_correct << "/4\n";
    total += or_correct;

    // NOR via NOT(OR)
    std::cout << "NOR (via NOT OR):\n";
    int nor_correct = 0;
    for (int a = 0; a <= 1; a++) {
        for (int b = 0; b <= 1; b++) {
            double angle_a = a ? HALF_PHI : -HALF_PHI;
            double angle_b = b ? HALF_PHI : -HALF_PHI;
            auto ct_a = make_ct(angle_a);
            auto ct_b = make_ct(angle_b);
            auto not_a = eval_not(ct_a);
            auto not_b = eval_not(ct_b);
            auto or_result = eval_nand(not_a, not_b);
            auto nor_result = eval_not(or_result);
            double cos_val = decrypt_val(nor_result);
            int got = (cos_val > -0.5) ? 1 : 0;
            int expected = !(a || b);
            if (got == expected) nor_correct++;
        }
    }
    std::cout << "  NOR: " << nor_correct << "/4\n";
    total += nor_correct;

    std::cout << "\n========================================\n";
    std::cout << "  TOTAL: " << total << "/20\n";
    std::cout << "  STATUS: " << (total == 20 ? "✅ LAHAT PERFECT!" : "⚠️ NEEDS TUNING") << "\n";
    std::cout << "========================================\n";
    std::cout << "\n  DECOMPOSITION:\n";
    std::cout << "  NOT(x) = x + π\n";
    std::cout << "  NAND(a,b) = cos(a+b+φ/2) > -0.5\n";
    std::cout << "  AND = NOT(NAND)\n";
    std::cout << "  OR = NAND(NOT(a), NOT(b))\n";
    std::cout << "  NOR = NOT(OR)\n";

    return 0;
}
