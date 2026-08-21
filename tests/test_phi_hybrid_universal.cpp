// φ²-DOMAIN HYBRID UNIVERSAL NAND
// 1 multiplication per NAND + AddSub optimization
// Mindset: tanggalin lahat ng assumption

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  φ²-DOMAIN HYBRID UNIVERSAL NAND\n";
    std::cout << "  Breaking All Assumptions\n";
    std::cout << "========================================\n\n";

    const double phi_sq = 2.618033988749895;
    const double phi = 1.6180339887498948482;

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(10);
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
    auto ct_zero = make_ct(0.0);
    auto ct_one = make_ct(1.0);

    // ============================================
    // HYBRID NAND: 1 mult + AddSub
    // ============================================
    // NAND(a,b) = φ² - (a·b) — 1 mult
    // Tapos gamitin ang AddSub para sa alternating parts

    auto nand_hybrid = [&](auto a, auto b) {
        auto prod = cc->EvalMult(a, b);  // 1 mult, 1 level
        return cc->EvalSub(ct_phi_sq, prod);  // 0 levels
    };

    // ============================================
    // TEST TRUTH TABLE
    // ============================================
    std::cout << "TRUTH TABLE (φ²-domain, hybrid):\n";
    std::cout << "=================================\n\n";

    auto t00 = nand_hybrid(ct_zero, ct_zero);
    auto t0p = nand_hybrid(ct_zero, ct_phi_sq);
    auto tpp = nand_hybrid(ct_phi_sq, ct_phi_sq);

    std::cout << "  NAND(0,0) = " << decrypt_val(t00) << " (expected " << phi_sq << ")\n";
    std::cout << "  NAND(0,φ²) = " << decrypt_val(t0p) << " (expected " << phi_sq << ")\n";
    std::cout << "  NAND(φ²,φ²) = " << decrypt_val(tpp) << " (expected 0)\n\n";

    // ============================================
    // UNIVERSAL COMPUTATION TEST
    // ============================================
    std::cout << "UNIVERSAL COMPUTATION TEST:\n";
    std::cout << "===========================\n\n";

    // Subukan ang AND, OR, XOR gamit ang hybrid NAND
    // Sa φ²-domain:
    // NAND(a,b) = φ² - a·b
    // NOT(x) = NAND(x,x) = φ² - x²
    // AND(a,b) = NOT(NAND(a,b)) = φ² - (NAND(a,b))²

    auto NOT_phi = [&](auto x) {
        return nand_hybrid(x, x);  // φ² - x²
    };

    auto AND_phi = [&](auto a, auto b) {
        auto nand_ab = nand_hybrid(a, b);
        return NOT_phi(nand_ab);  // φ² - (NAND(a,b))²
    };

    auto OR_phi = [&](auto a, auto b) {
        auto not_a = NOT_phi(a);
        auto not_b = NOT_phi(b);
        return nand_hybrid(not_a, not_b);  // NAND(NOT(a), NOT(b))
    };

    auto XOR_phi = [&](auto a, auto b) {
        auto nand_ab = nand_hybrid(a, b);
        auto and_ab = AND_phi(a, b);
        auto nand_and = nand_hybrid(nand_ab, and_ab);
        return nand_and;
    };

    std::cout << "AND GATE:\n";
    auto and_00 = AND_phi(ct_zero, ct_zero);
    auto and_0p = AND_phi(ct_zero, ct_phi_sq);
    auto and_pp = AND_phi(ct_phi_sq, ct_phi_sq);
    std::cout << "  AND(0,0) = " << decrypt_val(and_00) << "\n";
    std::cout << "  AND(0,φ²) = " << decrypt_val(and_0p) << "\n";
    std::cout << "  AND(φ²,φ²) = " << decrypt_val(and_pp) << "\n\n";

    std::cout << "OR GATE:\n";
    auto or_00 = OR_phi(ct_zero, ct_zero);
    auto or_0p = OR_phi(ct_zero, ct_phi_sq);
    auto or_pp = OR_phi(ct_phi_sq, ct_phi_sq);
    std::cout << "  OR(0,0) = " << decrypt_val(or_00) << "\n";
    std::cout << "  OR(0,φ²) = " << decrypt_val(or_0p) << "\n";
    std::cout << "  OR(φ²,φ²) = " << decrypt_val(or_pp) << "\n\n";

    // ============================================
    // FULL ADDER TEST
    // ============================================
    std::cout << "FULL ADDER:\n";
    std::cout << "===========\n\n";

    // Full adder: sum = a XOR b XOR cin, carry = (a AND b) OR (cin AND (a XOR b))
    auto a = ct_phi_sq;
    auto b = ct_phi_sq;
    auto cin = ct_zero;

    auto sum = XOR_phi(XOR_phi(a, b), cin);
    auto carry = OR_phi(AND_phi(a, b), AND_phi(cin, XOR_phi(a, b)));

    std::cout << "  Sum = " << decrypt_val(sum) << "\n";
    std::cout << "  Carry = " << decrypt_val(carry) << "\n\n";

    std::cout << "  Full adder level: " << sum->GetLevel() << "\n";
    std::cout << "  (Kung mababa, hybrid ay efficient!)\n";

    return 0;
}
