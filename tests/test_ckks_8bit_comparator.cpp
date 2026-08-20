// CKKS 8-BIT ENCRYPTED COMPARATOR
// Mas malaking circuit — 8-bit comparison
// True FHE, walang decrypt sa gitna
//
// GAMIT ANG SIMD: i-encode ang LAHAT ng 8 bits sa slots
// Para sa 8-bit: 8 slots para sa A, 8 slots para sa B
// Sa 4096 slots, may malaking room pa

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  8-BIT ENCRYPTED COMPARATOR\n";
    std::cout << "  True FHE — Real World\n";
    std::cout << "========================================\n\n";

    // CKKS Setup
    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(50);
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
    // ENCODING: I-encode ang bits sa magkaibang slots
    // Slots 0-7: A bits (A7..A0)
    // Slots 8-15: B bits (B7..B0)
    // ============================================
    auto make_bit_ct = [&](int slot, int bit) {
        std::vector<std::complex<double>> vec(slots, {0.0, 0.0});
        if (bit == 1) vec[slot] = {1.0, 0.0};
        return cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec));
    };

    auto ct1 = make_bit_ct(0, 1);  // Constant 1 para sa NAND
    auto ct0 = make_bit_ct(0, 0);  // Constant 0

    // Gates
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

    auto decrypt_slot = [&](auto ct, int slot) {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetCKKSPackedValue()[slot].real();
    };

    std::cout << "ALL GATES READY\n\n";

    // ============================================
    // TEST: A=200 (11001000) > B=150 (10010110)?
    // ============================================
    // A=200: 11001000
    // B=150: 10010110
    // 200 > 150 → TRUE

    std::cout << "TEST: A=200 > B=150?\n";
    std::cout << "Expected: TRUE\n\n";

    int A_bits[8] = {1, 1, 0, 0, 1, 0, 0, 0};  // A7..A0
    int B_bits[8] = {1, 0, 0, 1, 0, 1, 1, 0};  // B7..B0

    auto A7 = make_bit_ct(0, A_bits[0]);
    auto A6 = make_bit_ct(1, A_bits[1]);
    auto A5 = make_bit_ct(2, A_bits[2]);
    auto A4 = make_bit_ct(3, A_bits[3]);
    auto A3 = make_bit_ct(4, A_bits[4]);
    auto A2 = make_bit_ct(5, A_bits[5]);
    auto A1 = make_bit_ct(6, A_bits[6]);
    auto A0 = make_bit_ct(7, A_bits[7]);

    auto B7 = make_bit_ct(8, B_bits[0]);
    auto B6 = make_bit_ct(9, B_bits[1]);
    auto B5 = make_bit_ct(10, B_bits[2]);
    auto B4 = make_bit_ct(11, B_bits[3]);
    auto B3 = make_bit_ct(12, B_bits[4]);
    auto B2 = make_bit_ct(13, B_bits[5]);
    auto B1 = make_bit_ct(14, B_bits[6]);
    auto B0 = make_bit_ct(15, B_bits[7]);

    auto t_start = high_resolution_clock::now();

    // Compute comparisons
    auto diff7 = xor_op(A7, B7);
    auto diff6 = xor_op(A6, B6);
    auto diff5 = xor_op(A5, B5);
    auto diff4 = xor_op(A4, B4);
    auto diff3 = xor_op(A3, B3);
    auto diff2 = xor_op(A2, B2);
    auto diff1 = xor_op(A1, B1);

    auto notB7 = not_op(B7);
    auto notB6 = not_op(B6);
    auto notB5 = not_op(B5);
    auto notB4 = not_op(B4);
    auto notB3 = not_op(B3);
    auto notB2 = not_op(B2);
    auto notB1 = not_op(B1);
    auto notB0 = not_op(B0);

    auto gt7 = and_op(A7, notB7);
    auto gt6 = and_op(A6, notB6);
    auto gt5 = and_op(A5, notB5);
    auto gt4 = and_op(A4, notB4);
    auto gt3 = and_op(A3, notB3);
    auto gt2 = and_op(A2, notB2);
    auto gt1 = and_op(A1, notB1);
    auto gt0 = and_op(A0, notB0);

    // Cascade OR chain
    auto result = gt7;
    result = or_op(result, and_op(diff7, gt6));
    result = or_op(result, and_op(and_op(diff7, diff6), gt5));
    result = or_op(result, and_op(and_op(and_op(diff7, diff6), diff5), gt4));
    result = or_op(result, and_op(and_op(and_op(and_op(diff7, diff6), diff5), diff4), gt3));
    result = or_op(result, and_op(and_op(and_op(and_op(and_op(diff7, diff6), diff5), diff4), diff3), gt2));
    result = or_op(result, and_op(and_op(and_op(and_op(and_op(and_op(diff7, diff6), diff5), diff4), diff3), diff2), gt1));
    result = or_op(result, and_op(and_op(and_op(and_op(and_op(and_op(and_op(diff7, diff6), diff5), diff4), diff3), diff2), diff1), gt0));

    auto t_end = high_resolution_clock::now();

    double val = decrypt_slot(result, 0);
    int got = std::abs(val) > 0.5 ? 1 : 0;

    std::cout << "  Result: " << got << " (1=TRUE, 0=FALSE)\n";
    std::cout << "  Correct: " << (got == 1 ? "YES ✓" : "NO ✗") << "\n";
    std::cout << "  Time: " << duration_cast<milliseconds>(t_end - t_start).count() << " ms\n";

    return 0;
}
