// CKKS DEPTH 50 — Speed Test
// Mas mabilis kaysa depth 60, mas maraming gates kaysa depth 30
//
// TARGET: 45 gates sa depth 50, scaling=30, batch=1024

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  CKKS DEPTH 50 SPEED TEST\n";
    std::cout << "  True FHE, 45 gates\n";
    std::cout << "========================================\n\n";

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(50);
    params.SetScalingModSize(30);
    params.SetBatchSize(1024);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(params);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto t_keygen_start = high_resolution_clock::now();
    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);
    auto t_keygen_end = high_resolution_clock::now();
    std::cout << "KeyGen: " << duration_cast<milliseconds>(t_keygen_end - t_keygen_start).count() << " ms\n";

    auto slots = cc->GetEncodingParams()->GetBatchSize();
    auto ring = cc->GetRingDimension();
    std::cout << "Ring: " << ring << ", Slots: " << slots << "\n\n";

    std::vector<std::complex<double>> v1(slots, {0.0, 0.0});
    v1[0] = {1.0, 0.0};

    auto t_enc_start = high_resolution_clock::now();
    auto ct1 = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(v1));
    auto t_enc_end = high_resolution_clock::now();
    std::cout << "Encrypt: " << duration_cast<milliseconds>(t_enc_end - t_enc_start).count() << " ms\n";

    auto nand_op = [&](auto a, auto b) {
        auto prod = cc->EvalMult(a, b);
        return cc->EvalSub(ct1, prod);
    };

    // 45 gates — TRUE FHE, walang decrypt sa gitna
    std::cout << "\n45 NAND gates (True FHE)...\n";
    auto t_eval_start = high_resolution_clock::now();
    auto current = ct1;
    for (int i = 0; i < 45; i++) {
        current = nand_op(current, current);
    }
    auto t_eval_end = high_resolution_clock::now();
    auto eval_ms = duration_cast<milliseconds>(t_eval_end - t_eval_start).count();
    std::cout << "Eval: " << eval_ms << " ms\n";
    std::cout << "Per NAND: " << (double)eval_ms / 45 << " ms\n";

    Plaintext pt;
    auto t_dec_start = high_resolution_clock::now();
    cc->Decrypt(keys.secretKey, current, &pt);
    auto t_dec_end = high_resolution_clock::now();
    std::cout << "Decrypt: " << duration_cast<milliseconds>(t_dec_end - t_dec_start).count() << " ms\n";

    auto val = pt->GetCKKSPackedValue()[0].real();
    int expected = (45 % 2 == 0) ? 1 : 0;
    int got = (std::abs(val) > 0.5) ? 1 : 0;
    std::cout << "\nResult: " << (got == expected ? "CORRECT" : "WRONG") << "\n";
    std::cout << "Final value: " << val << " (expected " << expected << ")\n";

    // ============================================
    // COMPARISON WITH DEPTH 60
    // ============================================
    std::cout << "\n========================================\n";
    std::cout << "  COMPARISON:\n";
    std::cout << "  Depth 50, 45 gates: " << (double)eval_ms / 45 << " ms/NAND\n";
    std::cout << "  Depth 60, 55 gates: 1689 ms/NAND\n";
    std::cout << "========================================\n";

    return 0;
}
