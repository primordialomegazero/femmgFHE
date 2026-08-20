// CKKS REAL WORLD — ENCRYPTED COMPARATOR
// 4-bit encrypted comparison
// Client encrypts two numbers, server compares homomorphically
// Walang decrypt sa gitna — True FHE
//
// USE CASE: 
// - Patient data comparison (hindi exposed sa hospital)
// - Credit score check (hindi exposed sa bank)
// - Age verification (hindi exposed sa website)

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  ENCRYPTED 4-BIT COMPARATOR\n";
    std::cout << "  True FHE — Real World Application\n";
    std::cout << "========================================\n\n";

    // CKKS Setup
    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(30);
    params.SetScalingModSize(30);
    params.SetBatchSize(4096);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(params);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);

    auto slots = cc->GetEncodingParams()->GetBatchSize();
    std::cout << "CKKS: slots=" << slots << "\n\n";

    // ============================================
    // ENCODING: 0 → 0, 1 → 1 (binary sa slots)
    // ============================================
    std::vector<std::complex<double>> v1(slots, {0.0, 0.0});
    v1[0] = {1.0, 0.0};
    std::vector<std::complex<double>> v0(slots, {0.0, 0.0});

    auto ct1 = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(v1));
    auto ct0 = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(v0));

    // GATES
    auto nand_op = [&](auto a, auto b) {
        auto prod = cc->EvalMult(a, b);
        return cc->EvalSub(ct1, prod);
    };
    auto not_op = [&](auto a) { return nand_op(a, a); };
    auto and_op = [&](auto a, auto b) {
        auto nand_ab = nand_op(a, b);
        return nand_op(nand_ab, nand_ab);
    };
    auto or_op = [&](auto a, auto b) {
        auto not_a = not_op(a);
        auto not_b = not_op(b);
        return nand_op(not_a, not_b);
    };
    auto xor_op = [&](auto a, auto b) {
        auto nand_ab = nand_op(a, b);
        auto nand_a_nab = nand_op(a, nand_ab);
        auto nand_b_nab = nand_op(b, nand_ab);
        return nand_op(nand_a_nab, nand_b_nab);
    };

    auto decrypt_bit = [&](auto ct) {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetCKKSPackedValue()[0].real();
    };

    auto to_bool = [&](double v) {
        return std::abs(v) > 0.5 ? 1 : 0;
    };

    std::cout << "ALL GATES READY\n\n";

    // ============================================
    // 4-BIT COMPARATOR
    // A > B? kung saan A = a3 a2 a1 a0, B = b3 b2 b1 b0
    // ============================================
    // Algorithm:
    // diff_i = A_i XOR B_i (kung magkaiba)
    // greater_i = A_i AND NOT(B_i) (kung A=1 at B=0)
    // A > B kung:
    //   greater_3 OR (diff_3 AND greater_2) OR (diff_3 AND diff_2 AND greater_1) OR ...
    
    std::cout << "4-BIT COMPARATOR TEST\n";
    std::cout << "======================\n\n";

    // Test cases: A > B
    // A=10 (1010), B=7 (0111) → 10 > 7 = TRUE
    // A=5 (0101), B=12 (1100) → 5 > 12 = FALSE

    // Encoding A=10: a3=1, a2=0, a1=1, a0=0
    auto a3 = ct1, a2 = ct0, a1 = ct1, a0 = ct0;
    // Encoding B=7: b3=0, b2=1, b1=1, b0=1
    auto b3 = ct0, b2 = ct1, b1 = ct1, b0 = ct1;

    std::cout << "  Test: A=10 (1010) > B=7 (0111)?\n";
    std::cout << "  Expected: YES (TRUE)\n\n";

    auto t_start = high_resolution_clock::now();

    // Compute XORs (difference bits)
    auto diff3 = xor_op(a3, b3);
    auto diff2 = xor_op(a2, b2);
    auto diff1 = xor_op(a1, b1);
    auto diff0 = xor_op(a0, b0);

    // Compute greater bits (A=1 AND B=0)
    auto not_b3 = not_op(b3);
    auto not_b2 = not_op(b2);
    auto not_b1 = not_op(b1);
    auto not_b0 = not_op(b0);

    auto gt3 = and_op(a3, not_b3);
    auto gt2 = and_op(a2, not_b2);
    auto gt1 = and_op(a1, not_b1);
    auto gt0 = and_op(a0, not_b0);

    // A > B = gt3 OR (diff3 AND gt2) OR (diff3 AND diff2 AND gt1) OR (diff3 AND diff2 AND diff1 AND gt0)
    auto d3_gt2 = and_op(diff3, gt2);
    auto or1 = or_op(gt3, d3_gt2);

    auto d3_d2 = and_op(diff3, diff2);
    auto d3_d2_gt1 = and_op(d3_d2, gt1);
    auto or2 = or_op(or1, d3_d2_gt1);

    auto d3_d2_d1 = and_op(d3_d2, diff1);
    auto d3_d2_d1_gt0 = and_op(d3_d2_d1, gt0);
    auto result = or_op(or2, d3_d2_d1_gt0);

    auto t_end = high_resolution_clock::now();

    double val = decrypt_bit(result);
    int got = to_bool(val);

    std::cout << "  Result: " << got << " (1=TRUE, 0=FALSE)\n";
    std::cout << "  Correct: " << (got == 1 ? "YES ✓" : "NO ✗") << "\n";
    std::cout << "  Time: " << duration_cast<milliseconds>(t_end - t_start).count() << " ms\n\n";

    // Test 2: A=5, B=12
    std::cout << "  Test: A=5 (0101) > B=12 (1100)?\n";
    std::cout << "  Expected: NO (FALSE)\n\n";

    a3 = ct0; a2 = ct1; a1 = ct0; a0 = ct1;  // A=5
    b3 = ct1; b2 = ct1; b1 = ct0; b0 = ct0;  // B=12

    t_start = high_resolution_clock::now();

    diff3 = xor_op(a3, b3);
    diff2 = xor_op(a2, b2);
    diff1 = xor_op(a1, b1);

    not_b3 = not_op(b3);
    not_b2 = not_op(b2);
    not_b1 = not_op(b1);
    not_b0 = not_op(b0);

    gt3 = and_op(a3, not_b3);
    gt2 = and_op(a2, not_b2);
    gt1 = and_op(a1, not_b1);
    gt0 = and_op(a0, not_b0);

    d3_gt2 = and_op(diff3, gt2);
    or1 = or_op(gt3, d3_gt2);

    d3_d2 = and_op(diff3, diff2);
    d3_d2_gt1 = and_op(d3_d2, gt1);
    or2 = or_op(or1, d3_d2_gt1);

    d3_d2_d1 = and_op(d3_d2, diff1);
    d3_d2_d1_gt0 = and_op(d3_d2_d1, gt0);
    result = or_op(or2, d3_d2_d1_gt0);

    t_end = high_resolution_clock::now();

    val = decrypt_bit(result);
    got = to_bool(val);

    std::cout << "  Result: " << got << " (1=TRUE, 0=FALSE)\n";
    std::cout << "  Correct: " << (got == 0 ? "YES ✓" : "NO ✗") << "\n";
    std::cout << "  Time: " << duration_cast<milliseconds>(t_end - t_start).count() << " ms\n";

    return 0;
}
