// PERIOD-0 NAND SEARCH
// Hanapin ang natural na NAND pattern sa period-0 space
// Subukan iba't ibang φ-derived combinations

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  PERIOD-0 NAND SEARCH\n";
    std::cout << "  Natural φ Pattern Hunt\n";
    std::cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double PHI_MOD = 0.6180339887498949;
    const double PHI2_MOD = 0.38196601125010515;
    const double PHI3_MOD = 0.2360679774997897;
    const double PHI4_MOD = 0.8541019662496845;

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(1);
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

    // Base encodings
    auto ct_0 = make_ct(0.0);
    auto ct_1 = make_ct(PHI_MOD);

    // Constant ciphertexts
    auto ct_phi_mod = make_ct(PHI_MOD);
    auto ct_phi2_mod = make_ct(PHI2_MOD);
    auto ct_phi3_mod = make_ct(PHI3_MOD);
    auto ct_phi4_mod = make_ct(PHI4_MOD);

    std::cout << "FUNCTIONS SA PERIOD-0 SPACE:\n";
    std::cout << "============================\n\n";

    // Subukan iba't ibang operations
    struct TestResult {
        std::string name;
        std::vector<double> values;
    };

    std::vector<TestResult> results;

    // 1. Simple sum: a + b
    {
        std::vector<double> vals;
        for (int a = 0; a <= 1; a++) {
            for (int b = 0; b <= 1; b++) {
                auto ct_a = a ? ct_1 : ct_0;
                auto ct_b = b ? ct_1 : ct_0;
                auto sum = cc->EvalAdd(ct_a, ct_b);
                vals.push_back(decrypt_val(sum));
            }
        }
        results.push_back({"a + b", vals});
    }

    // 2. Sum minus φ: a + b - φ
    {
        std::vector<double> vals;
        for (int a = 0; a <= 1; a++) {
            for (int b = 0; b <= 1; b++) {
                auto ct_a = a ? ct_1 : ct_0;
                auto ct_b = b ? ct_1 : ct_0;
                auto sum = cc->EvalAdd(ct_a, ct_b);
                auto result = cc->EvalSub(sum, ct_phi_mod);
                vals.push_back(decrypt_val(result));
            }
        }
        results.push_back({"a + b - φ", vals});
    }

    // 3. φ - (a + b): NAND attempt
    {
        std::vector<double> vals;
        for (int a = 0; a <= 1; a++) {
            for (int b = 0; b <= 1; b++) {
                auto ct_a = a ? ct_1 : ct_0;
                auto ct_b = b ? ct_1 : ct_0;
                auto sum = cc->EvalAdd(ct_a, ct_b);
                auto result = cc->EvalSub(ct_phi_mod, sum);
                vals.push_back(decrypt_val(result));
            }
        }
        results.push_back({"φ - (a+b)", vals});
    }

    // 4. φ² - (a+b)
    {
        std::vector<double> vals;
        for (int a = 0; a <= 1; a++) {
            for (int b = 0; b <= 1; b++) {
                auto ct_a = a ? ct_1 : ct_0;
                auto ct_b = b ? ct_1 : ct_0;
                auto sum = cc->EvalAdd(ct_a, ct_b);
                auto result = cc->EvalSub(ct_phi2_mod, sum);
                vals.push_back(decrypt_val(result));
            }
        }
        results.push_back({"φ² - (a+b)", vals});
    }

    // 5. φ³ - (a+b)
    {
        std::vector<double> vals;
        for (int a = 0; a <= 1; a++) {
            for (int b = 0; b <= 1; b++) {
                auto ct_a = a ? ct_1 : ct_0;
                auto ct_b = b ? ct_1 : ct_0;
                auto sum = cc->EvalAdd(ct_a, ct_b);
                auto result = cc->EvalSub(ct_phi3_mod, sum);
                vals.push_back(decrypt_val(result));
            }
        }
        results.push_back({"φ³ - (a+b)", vals});
    }

    // 6. φ⁴ - (a+b)
    {
        std::vector<double> vals;
        for (int a = 0; a <= 1; a++) {
            for (int b = 0; b <= 1; b++) {
                auto ct_a = a ? ct_1 : ct_0;
                auto ct_b = b ? ct_1 : ct_0;
                auto sum = cc->EvalAdd(ct_a, ct_b);
                auto result = cc->EvalSub(ct_phi4_mod, sum);
                vals.push_back(decrypt_val(result));
            }
        }
        results.push_back({"φ⁴ - (a+b)", vals});
    }

    // I-print ang lahat
    for (auto& result : results) {
        std::cout << result.name << ": ";
        for (int i = 0; i < 4; i++) {
            std::cout << result.values[i];
            if (i < 3) std::cout << ", ";
        }
        std::cout << "\n";
    }

    // Tingnan kung may pattern na (1,1,1,0) para sa NAND
    std::cout << "\nNAND TARGET: (1, 1, 1, 0)\n";
    std::cout << "================\n\n";

    for (auto& result : results) {
        // Hanapin kung may threshold na nagbibigay ng (1,1,1,0)
        for (double threshold : {0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8}) {
            std::vector<int> pattern;
            for (double v : result.values) {
                pattern.push_back(v > threshold ? 1 : 0);
            }
            if (pattern == std::vector<int>{1,1,1,0}) {
                std::cout << "✓ " << result.name << " na may threshold " << threshold << " → NAND!\n";
            }
        }
    }

    return 0;
}
