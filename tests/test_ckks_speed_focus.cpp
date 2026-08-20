// CKKS SPEED FOCUS — True FHE, Maximum Speed
// Depth 60, iba't ibang optimization strategies
//
// ANG MGA OPTIMIZATION:
// 1. Decrypt lang sa DULO (hindi sa bawat gate)
// 2. Scaling modulus na pinakamababa
// 3. Batch size na pinakamaliit
// 4. Walang relinearization kung pwede

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  CKKS SPEED FOCUS\n";
    std::cout << "  True FHE, Maximum Speed\n";
    std::cout << "========================================\n\n";

    // ============================================
    // TEST A: BASELINE (scaling=40, batch=2048)
    // ============================================
    std::cout << "TEST A: Baseline (scaling=40, batch=2048)\n";
    std::cout << "=============================================\n\n";

    {
        CCParams<CryptoContextCKKSRNS> params;
        params.SetMultiplicativeDepth(60);
        params.SetScalingModSize(40);
        params.SetBatchSize(2048);

        CryptoContext<DCRTPoly> cc = GenCryptoContext(params);
        cc->Enable(PKE);
        cc->Enable(KEYSWITCH);
        cc->Enable(LEVELEDSHE);

        auto t_keygen_start = high_resolution_clock::now();
        auto keys = cc->KeyGen();
        cc->EvalMultKeyGen(keys.secretKey);
        auto t_keygen_end = high_resolution_clock::now();
        std::cout << "  KeyGen: " << duration_cast<milliseconds>(t_keygen_end - t_keygen_start).count() << " ms\n";

        auto slots = cc->GetEncodingParams()->GetBatchSize();
        std::vector<std::complex<double>> v1(slots, {0.0, 0.0});
        v1[0] = {1.0, 0.0};

        auto t_enc_start = high_resolution_clock::now();
        auto ct1 = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(v1));
        auto t_enc_end = high_resolution_clock::now();
        std::cout << "  Encrypt: " << duration_cast<milliseconds>(t_enc_end - t_enc_start).count() << " ms\n";

        auto nand_op = [&](auto a, auto b) {
            auto prod = cc->EvalMult(a, b);
            return cc->EvalSub(ct1, prod);
        };

        // 55 gates — WALANG DECRYPT SA GITNA
        auto t_eval_start = high_resolution_clock::now();
        auto current = ct1;
        for (int i = 0; i < 55; i++) {
            current = nand_op(current, current);
        }
        auto t_eval_end = high_resolution_clock::now();
        std::cout << "  55 NANDs (walang decrypt): " << duration_cast<milliseconds>(t_eval_end - t_eval_start).count() << " ms\n";

        Plaintext pt;
        auto t_dec_start = high_resolution_clock::now();
        cc->Decrypt(keys.secretKey, current, &pt);
        auto t_dec_end = high_resolution_clock::now();
        std::cout << "  1 Decrypt: " << duration_cast<milliseconds>(t_dec_end - t_dec_start).count() << " ms\n";

        auto val = pt->GetCKKSPackedValue()[0].real();
        int expected = (55 % 2 == 0) ? 1 : 0;
        int got = (std::abs(val) > 0.5) ? 1 : 0;
        std::cout << "  Result: " << (got == expected ? "CORRECT" : "WRONG") << "\n\n";
    }

    // ============================================
    // TEST B: SCALING=30, BATCH=1024
    // ============================================
    std::cout << "TEST B: scaling=30, batch=1024\n";
    std::cout << "================================\n\n";

    {
        CCParams<CryptoContextCKKSRNS> params;
        params.SetMultiplicativeDepth(60);
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
        std::cout << "  KeyGen: " << duration_cast<milliseconds>(t_keygen_end - t_keygen_start).count() << " ms\n";

        auto slots = cc->GetEncodingParams()->GetBatchSize();
        std::vector<std::complex<double>> v1(slots, {0.0, 0.0});
        v1[0] = {1.0, 0.0};

        auto t_enc_start = high_resolution_clock::now();
        auto ct1 = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(v1));
        auto t_enc_end = high_resolution_clock::now();
        std::cout << "  Encrypt: " << duration_cast<milliseconds>(t_enc_end - t_enc_start).count() << " ms\n";

        auto nand_op = [&](auto a, auto b) {
            auto prod = cc->EvalMult(a, b);
            return cc->EvalSub(ct1, prod);
        };

        auto t_eval_start = high_resolution_clock::now();
        auto current = ct1;
        for (int i = 0; i < 55; i++) {
            current = nand_op(current, current);
        }
        auto t_eval_end = high_resolution_clock::now();
        std::cout << "  55 NANDs: " << duration_cast<milliseconds>(t_eval_end - t_eval_start).count() << " ms\n";

        Plaintext pt;
        auto t_dec_start = high_resolution_clock::now();
        cc->Decrypt(keys.secretKey, current, &pt);
        auto t_dec_end = high_resolution_clock::now();
        std::cout << "  1 Decrypt: " << duration_cast<milliseconds>(t_dec_end - t_dec_start).count() << " ms\n";

        auto val = pt->GetCKKSPackedValue()[0].real();
        int expected = (55 % 2 == 0) ? 1 : 0;
        int got = (std::abs(val) > 0.5) ? 1 : 0;
        std::cout << "  Result: " << (got == expected ? "CORRECT" : "WRONG") << "\n\n";
    }

    // ============================================
    // TEST C: SCALING=26, BATCH=512
    // ============================================
    std::cout << "TEST C: scaling=26, batch=512\n";
    std::cout << "===============================\n\n";

    {
        CCParams<CryptoContextCKKSRNS> params;
        params.SetMultiplicativeDepth(60);
        params.SetScalingModSize(26);
        params.SetBatchSize(512);

        CryptoContext<DCRTPoly> cc = GenCryptoContext(params);
        cc->Enable(PKE);
        cc->Enable(KEYSWITCH);
        cc->Enable(LEVELEDSHE);

        auto t_keygen_start = high_resolution_clock::now();
        auto keys = cc->KeyGen();
        cc->EvalMultKeyGen(keys.secretKey);
        auto t_keygen_end = high_resolution_clock::now();
        std::cout << "  KeyGen: " << duration_cast<milliseconds>(t_keygen_end - t_keygen_start).count() << " ms\n";

        auto slots = cc->GetEncodingParams()->GetBatchSize();
        std::vector<std::complex<double>> v1(slots, {0.0, 0.0});
        v1[0] = {1.0, 0.0};

        auto t_enc_start = high_resolution_clock::now();
        auto ct1 = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(v1));
        auto t_enc_end = high_resolution_clock::now();
        std::cout << "  Encrypt: " << duration_cast<milliseconds>(t_enc_end - t_enc_start).count() << " ms\n";

        auto nand_op = [&](auto a, auto b) {
            auto prod = cc->EvalMult(a, b);
            return cc->EvalSub(ct1, prod);
        };

        auto t_eval_start = high_resolution_clock::now();
        auto current = ct1;
        for (int i = 0; i < 55; i++) {
            current = nand_op(current, current);
        }
        auto t_eval_end = high_resolution_clock::now();
        std::cout << "  55 NANDs: " << duration_cast<milliseconds>(t_eval_end - t_eval_start).count() << " ms\n";

        Plaintext pt;
        auto t_dec_start = high_resolution_clock::now();
        cc->Decrypt(keys.secretKey, current, &pt);
        auto t_dec_end = high_resolution_clock::now();
        std::cout << "  1 Decrypt: " << duration_cast<milliseconds>(t_dec_end - t_dec_start).count() << " ms\n";

        auto val = pt->GetCKKSPackedValue()[0].real();
        int expected = (55 % 2 == 0) ? 1 : 0;
        int got = (std::abs(val) > 0.5) ? 1 : 0;
        std::cout << "  Result: " << (got == expected ? "CORRECT" : "WRONG") << "\n\n";
    }

    return 0;
}
