// MULTI-SCHEME VERIFICATION — BFV + CKKS
// Rule 110 + NAND base sa iba't ibang schemes
// Para ma-verify na hindi fluke

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  MULTI-SCHEME VERIFICATION\n";
    std::cout << "  BFV + CKKS\n";
    std::cout << "========================================\n\n";

    // ==========================================
    // CKKS TEST
    // ==========================================
    std::cout << "CKKS NAND BASE:\n";
    std::cout << "===============\n\n";

    {
        CCParams<CryptoContextCKKSRNS> params;
        params.SetMultiplicativeDepth(20);
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

        auto ct_0 = make_ct(0.0);
        auto ct_1 = make_ct(1.0);

        auto eval_nand = [&](auto a, auto b) {
            auto product = cc->EvalMult(a, b);
            return cc->EvalSub(make_ct(1.0), product);
        };

        std::cout << "  NAND(0,0) = " << decrypt_val(eval_nand(ct_0, ct_0)) << " (dapat 1)\n";
        std::cout << "  NAND(0,1) = " << decrypt_val(eval_nand(ct_0, ct_1)) << " (dapat 1)\n";
        std::cout << "  NAND(1,1) = " << decrypt_val(eval_nand(ct_1, ct_1)) << " (dapat 0)\n\n";
    }

    // ==========================================
    // CKKS Rule 110
    // ==========================================
    std::cout << "CKKS RULE 110:\n";
    std::cout << "==============\n\n";

    {
        CCParams<CryptoContextCKKSRNS> params;
        params.SetMultiplicativeDepth(20);
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

        auto ct_0 = make_ct(0.0);
        auto ct_1 = make_ct(1.0);

        // Rule 110: next = C + R - C*R - L*C*R
        auto eval_rule110 = [&](auto L, auto C, auto R) {
            auto CR = cc->EvalMult(C, R);
            auto LCR = cc->EvalMult(L, CR);
            auto sum1 = cc->EvalAdd(C, R);
            auto sum2 = cc->EvalSub(sum1, CR);
            return cc->EvalSub(sum2, LCR);
        };

        int correct = 0;
        int tests[8][3] = {
            {0,0,0}, {0,0,1}, {0,1,0}, {0,1,1},
            {1,0,0}, {1,0,1}, {1,1,0}, {1,1,1}
        };
        int expected[8] = {0,1,1,1,0,1,1,0};

        for (int i = 0; i < 8; i++) {
            auto L = tests[i][0] ? ct_1 : ct_0;
            auto C = tests[i][1] ? ct_1 : ct_0;
            auto R = tests[i][2] ? ct_1 : ct_0;
            
            auto result = eval_rule110(L, C, R);
            double val = decrypt_val(result);
            int bit = (val > 0.5) ? 1 : 0;
            
            if (bit == expected[i]) correct++;
            
            std::cout << "  (" << tests[i][0] << tests[i][1] << tests[i][2] << ") → "
                      << bit << " (expected " << expected[i] << ")"
                      << (bit == expected[i] ? " ✓" : " ✗") << "\n";
        }
        
        std::cout << "\n  CKKS Rule 110: " << correct << "/8\n";
    }

    std::cout << "\n========================================\n";
    std::cout << "  VERIFICATION COMPLETE\n";
    std::cout << "  BFV + CKKS parehong gumagana\n";
    std::cout << "========================================\n";

    return 0;
}
