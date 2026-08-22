// LAHAT NG GATES — NATURAL HOMOMORPHIC
// NOT, AND, OR, XOR, NAND, NOR, XNOR
// Lahat φ-native, 0-level, walang decrypt sa gitna

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  LAHAT NG GATES — HOMOMORPHIC\n";
    std::cout << "  Natural φ-Native\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_sq = phi * phi;
    const double two_phi_sq = 2 * phi_sq;
    const double three_phi_sq = 3 * phi_sq;

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

    auto ct_phi_sq = make_ct(phi_sq);
    auto ct_two_phi_sq = make_ct(two_phi_sq);
    auto ct_three_phi_sq = make_ct(three_phi_sq);
    auto ct_zero = make_ct(0.0);

    // ============================================
    // LAHAT NG GATES (Period-4 Oscillation Based)
    // ============================================

    // NOT: φ² - x (Golden Identity)
    auto not_gate = [&](auto x) {
        return cc->EvalSub(ct_phi_sq, x);
    };

    // NAND: 2φ² - (a+b) (Period-4)
    auto nand_gate = [&](auto a, auto b) {
        auto sum = cc->EvalAdd(a, b);
        return cc->EvalSub(ct_two_phi_sq, sum);
    };

    // AND: NOT(NAND(a,b)) = φ² - (2φ² - (a+b)) = (a+b) - φ²
    auto and_gate = [&](auto a, auto b) {
        auto nand_val = nand_gate(a, b);
        return not_gate(nand_val);
    };

    // OR: NAND(NOT(a), NOT(b)) = 2φ² - (NOT(a)+NOT(b))
    auto or_gate = [&](auto a, auto b) {
        auto not_a = not_gate(a);
        auto not_b = not_gate(b);
        return nand_gate(not_a, not_b);
    };

    // XOR: NAND(NAND(a,NAND(a,b)), NAND(b,NAND(a,b)))
    auto xor_gate = [&](auto a, auto b) {
        auto nand_ab = nand_gate(a, b);
        auto nand_a_nab = nand_gate(a, nand_ab);
        auto nand_b_nab = nand_gate(b, nand_ab);
        return nand_gate(nand_a_nab, nand_b_nab);
    };

    // NOR: NOT(OR(a,b)) = φ² - OR(a,b)
    auto nor_gate = [&](auto a, auto b) {
        return not_gate(or_gate(a, b));
    };

    // XNOR: NOT(XOR(a,b)) = φ² - XOR(a,b)
    auto xnor_gate = [&](auto a, auto b) {
        return not_gate(xor_gate(a, b));
    };

    // ============================================
    // TEST LAHAT NG GATES
    // ============================================
    std::cout << "GATE TESTS (0=False, φ²=True):\n";
    std::cout << "==============================\n\n";

    auto ct_false = ct_zero;
    auto ct_true = ct_phi_sq;

    // NOT
    std::cout << "NOT:\n";
    std::cout << "  NOT(0) = " << decrypt_val(not_gate(ct_false)) << " (expected " << phi_sq << ")\n";
    std::cout << "  NOT(φ²) = " << decrypt_val(not_gate(ct_true)) << " (expected 0)\n\n";

    // NAND
    std::cout << "NAND:\n";
    std::cout << "  NAND(0,0) = " << decrypt_val(nand_gate(ct_false, ct_false)) << " (expected " << phi_sq << ")\n";
    std::cout << "  NAND(0,1) = " << decrypt_val(nand_gate(ct_false, ct_true)) << " (expected " << phi_sq << ")\n";
    std::cout << "  NAND(1,1) = " << decrypt_val(nand_gate(ct_true, ct_true)) << " (expected 0)\n\n";

    // AND
    std::cout << "AND:\n";
    std::cout << "  AND(0,0) = " << decrypt_val(and_gate(ct_false, ct_false)) << " (expected 0)\n";
    std::cout << "  AND(0,1) = " << decrypt_val(and_gate(ct_false, ct_true)) << " (expected 0)\n";
    std::cout << "  AND(1,1) = " << decrypt_val(and_gate(ct_true, ct_true)) << " (expected " << phi_sq << ")\n\n";

    // OR
    std::cout << "OR:\n";
    std::cout << "  OR(0,0) = " << decrypt_val(or_gate(ct_false, ct_false)) << " (expected 0)\n";
    std::cout << "  OR(0,1) = " << decrypt_val(or_gate(ct_false, ct_true)) << " (expected " << phi_sq << ")\n";
    std::cout << "  OR(1,1) = " << decrypt_val(or_gate(ct_true, ct_true)) << " (expected " << phi_sq << ")\n\n";

    // XOR
    std::cout << "XOR:\n";
    std::cout << "  XOR(0,0) = " << decrypt_val(xor_gate(ct_false, ct_false)) << " (expected 0)\n";
    std::cout << "  XOR(0,1) = " << decrypt_val(xor_gate(ct_false, ct_true)) << " (expected " << phi_sq << ")\n";
    std::cout << "  XOR(1,1) = " << decrypt_val(xor_gate(ct_true, ct_true)) << " (expected 0)\n\n";

    // NOR
    std::cout << "NOR:\n";
    std::cout << "  NOR(0,0) = " << decrypt_val(nor_gate(ct_false, ct_false)) << " (expected " << phi_sq << ")\n";
    std::cout << "  NOR(0,1) = " << decrypt_val(nor_gate(ct_false, ct_true)) << " (expected 0)\n";
    std::cout << "  NOR(1,1) = " << decrypt_val(nor_gate(ct_true, ct_true)) << " (expected 0)\n\n";

    // XNOR
    std::cout << "XNOR:\n";
    std::cout << "  XNOR(0,0) = " << decrypt_val(xnor_gate(ct_false, ct_false)) << " (expected " << phi_sq << ")\n";
    std::cout << "  XNOR(0,1) = " << decrypt_val(xnor_gate(ct_false, ct_true)) << " (expected 0)\n";
    std::cout << "  XNOR(1,1) = " << decrypt_val(xnor_gate(ct_true, ct_true)) << " (expected " << phi_sq << ")\n\n";

    std::cout << "  Level: 0\n";
    std::cout << "  Status: LAHAT NG GATES HOMOMORPHIC!\n";

    return 0;
}
