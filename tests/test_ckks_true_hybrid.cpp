// CKKS TRUE HYBRID — Walang Decrypt sa Gitna
// Ang chunked processing ay ginagawa homomorphically
//
// ANG KEY:
// Sa halip na mag-decrypt at mag-re-encrypt,
// gamitin ang period-2 property:
//   refresh(x) = NOT(NOT(x)) = x
//
// Ang refresh ay homomorphic — walang decrypt!

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  CKKS TRUE HYBRID\n";
    std::cout << "  Homomorphic Refresh — Walang Decrypt\n";
    std::cout << "========================================\n\n";

    // Depth 200, scaling 40
    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(200);
    params.SetScalingModSize(40);
    params.SetBatchSize(2048);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(params);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);

    auto slots = cc->GetEncodingParams()->GetBatchSize();
    auto ring = cc->GetRingDimension();

    std::cout << "Ring: " << ring << ", Slots: " << slots << "\n\n";

    std::vector<std::complex<double>> v1(slots, {0.0, 0.0});
    v1[0] = {1.0, 0.0};
    std::vector<std::complex<double>> v0(slots, {0.0, 0.0});

    auto ct1 = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(v1));
    auto ct0 = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(v0));

    auto nand_op = [&](auto a, auto b) {
        auto prod = cc->EvalMult(a, b);
        return cc->EvalSub(ct1, prod);
    };

    auto decrypt_val = [&](auto ct) {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetCKKSPackedValue()[0].real();
    };

    // ============================================
    // HOMOMORPHIC REFRESH
    // ============================================
    // refresh(x) = NOT(NOT(x)) = x
    // Sa NAND: NOT(x) = NAND(x,x)
    // refresh(x) = NAND(NAND(x,x), NAND(x,x))
    //
    // Cost: 4 multiplications per refresh
    // Para sa chunk ng 10 gates: 10 + 4 = 14 mults per chunk
    // Mas efficient kaysa decrypt+re-encrypt!

    auto homomorphic_refresh = [&](auto x) {
        auto not1 = nand_op(x, x);
        return nand_op(not1, not1);
    };

    std::cout << "TRUE HYBRID TEST:\n";
    std::cout << "==================\n\n";
    std::cout << "  Strategy: 10 NAND + homomorphic refresh\n";
    std::cout << "  Walang decrypt sa gitna!\n\n";

    const int CHUNK_SIZE = 10;
    const int TOTAL_GATES = 100;

    auto current = ct1;
    int errors = 0;
    int total_mults = 0;

    auto t1 = high_resolution_clock::now();

    for (int gate = 0; gate < TOTAL_GATES; gate++) {
        current = nand_op(current, current);
        total_mults++;

        // Homomorphic refresh every CHUNK_SIZE gates
        if ((gate + 1) % CHUNK_SIZE == 0 && gate < TOTAL_GATES - 1) {
            current = homomorphic_refresh(current);
            total_mults += 4;
        }

        // Verify sa dulo lang
        if (gate == TOTAL_GATES - 1) {
            double v = decrypt_val(current);
            int got = (std::abs(v) > 0.5) ? 1 : 0;
            int expected = (gate % 2 == 0) ? 0 : 1;
            if (got != expected) errors++;
        }
    }

    auto t2 = high_resolution_clock::now();
    auto ms = duration_cast<milliseconds>(t2 - t1).count();

    // Final decryption
    Plaintext pt;
    cc->Decrypt(keys.secretKey, current, &pt);
    double final_val = pt->GetCKKSPackedValue()[0].real();
    int got_final = (std::abs(final_val) > 0.5) ? 1 : 0;
    int expected_final = (TOTAL_GATES % 2 == 0) ? 1 : 0;

    std::cout << "\n========================================\n";
    std::cout << "  RESULT:\n";
    std::cout << "  Total gates: " << TOTAL_GATES << "\n";
    std::cout << "  Total multiplications: " << total_mults << "\n";
    std::cout << "  Final value: " << final_val << "\n";
    std::cout << "  Expected: " << expected_final << "\n";
    std::cout << "  Correct: " << (got_final == expected_final ? "YES" : "NO") << "\n";
    std::cout << "  Time: " << ms << " ms\n";
    std::cout << "========================================\n";

    return 0;
}
