// φ-POWERS THRESHOLD SEARCH
// Hanapin ang natural na threshold gamit ang φ-powers
// NAND = φ⁴ - (a+b) na may natural na separation

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  φ-POWERS THRESHOLD SEARCH\n";
    std::cout << "  Natural Separation\n";
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

    auto ct_zero = make_ct(0.0);
    auto ct_one = make_ct(PHI_MOD);

    // Lahat ng φ-powers as ciphertexts
    auto ct_phi_mod = make_ct(PHI_MOD);
    auto ct_phi2_mod = make_ct(PHI2_MOD);
    auto ct_phi3_mod = make_ct(PHI3_MOD);
    auto ct_phi4_mod = make_ct(PHI4_MOD);

    std::cout << "FUNCTIONS TO TEST:\n";
    std::cout << "==================\n\n";

    struct Function {
        std::string name;
        Ciphertext<DCRTPoly> (*eval)(CryptoContext<DCRTPoly>&, 
                                      Ciphertext<DCRTPoly>, 
                                      Ciphertext<DCRTPoly>,
                                      Ciphertext<DCRTPoly>,
                                      Ciphertext<DCRTPoly>,
                                      Ciphertext<DCRTPoly>,
                                      Ciphertext<DCRTPoly>);
    };

    // Subukan ang iba't ibang operations
    std::cout << "TESTING PHI-POWER COMBINATIONS:\n";
    std::cout << "===============================\n\n";

    // Base NAND: φ⁴ - (a+b)
    std::vector<double> nand_vals;
    for (int a = 0; a <= 1; a++) {
        for (int b = 0; b <= 1; b++) {
            auto ct_a = a ? ct_one : ct_zero;
            auto ct_b = b ? ct_one : ct_zero;
            auto sum = cc->EvalAdd(ct_a, ct_b);
            auto nand = cc->EvalSub(ct_phi4_mod, sum);
            nand_vals.push_back(decrypt_val(nand));
        }
    }
    std::cout << "Base NAND (φ⁴ - sum): ";
    for (double v : nand_vals) std::cout << v << " ";
    std::cout << "\n\n";

    // Subukan: φ⁴ - sum + φ³
    std::cout << "φ⁴ - sum + φ³: ";
    for (int a = 0; a <= 1; a++) {
        for (int b = 0; b <= 1; b++) {
            auto ct_a = a ? ct_one : ct_zero;
            auto ct_b = b ? ct_one : ct_zero;
            auto sum = cc->EvalAdd(ct_a, ct_b);
            auto nand = cc->EvalSub(ct_phi4_mod, sum);
            auto result = cc->EvalAdd(nand, ct_phi3_mod);
            std::cout << decrypt_val(result) << " ";
        }
    }
    std::cout << "\n\n";

    // Subukan: φ⁴ - sum - φ³
    std::cout << "φ⁴ - sum - φ³: ";
    for (int a = 0; a <= 1; a++) {
        for (int b = 0; b <= 1; b++) {
            auto ct_a = a ? ct_one : ct_zero;
            auto ct_b = b ? ct_one : ct_zero;
            auto sum = cc->EvalAdd(ct_a, ct_b);
            auto nand = cc->EvalSub(ct_phi4_mod, sum);
            auto result = cc->EvalSub(nand, ct_phi3_mod);
            std::cout << decrypt_val(result) << " ";
        }
    }
    std::cout << "\n\n";

    // Subukan: φ⁴ - sum + φ²
    std::cout << "φ⁴ - sum + φ²: ";
    for (int a = 0; a <= 1; a++) {
        for (int b = 0; b <= 1; b++) {
            auto ct_a = a ? ct_one : ct_zero;
            auto ct_b = b ? ct_one : ct_zero;
            auto sum = cc->EvalAdd(ct_a, ct_b);
            auto nand = cc->EvalSub(ct_phi4_mod, sum);
            auto result = cc->EvalAdd(nand, ct_phi2_mod);
            std::cout << decrypt_val(result) << " ";
        }
    }
    std::cout << "\n\n";

    // Subukan: φ⁴ - sum + φ
    std::cout << "φ⁴ - sum + φ: ";
    for (int a = 0; a <= 1; a++) {
        for (int b = 0; b <= 1; b++) {
            auto ct_a = a ? ct_one : ct_zero;
            auto ct_b = b ? ct_one : ct_zero;
            auto sum = cc->EvalAdd(ct_a, ct_b);
            auto nand = cc->EvalSub(ct_phi4_mod, sum);
            auto result = cc->EvalAdd(nand, ct_phi_mod);
            std::cout << decrypt_val(result) << " ";
        }
    }
    std::cout << "\n\n";

    // Subukan: - (φ⁴ - sum) = sum - φ⁴
    std::cout << "sum - φ⁴ (negated NAND): ";
    for (int a = 0; a <= 1; a++) {
        for (int b = 0; b <= 1; b++) {
            auto ct_a = a ? ct_one : ct_zero;
            auto ct_b = b ? ct_one : ct_zero;
            auto sum = cc->EvalAdd(ct_a, ct_b);
            auto result = cc->EvalSub(sum, ct_phi4_mod);
            std::cout << decrypt_val(result) << " ";
        }
    }
    std::cout << "\n\n";

    // Hanapin ang threshold na nagbibigay ng NAND (1,1,1,0)
    std::cout << "THRESHOLD SEARCH:\n";
    std::cout << "=================\n\n";

    struct Candidate {
        std::string name;
        std::vector<double> vals;
    };

    std::vector<Candidate> candidates;

    // φ⁴ - sum + φ³
    {
        std::vector<double> vals;
        for (int a = 0; a <= 1; a++) {
            for (int b = 0; b <= 1; b++) {
                auto ct_a = a ? ct_one : ct_zero;
                auto ct_b = b ? ct_one : ct_zero;
                auto sum = cc->EvalAdd(ct_a, ct_b);
                auto nand = cc->EvalSub(ct_phi4_mod, sum);
                auto result = cc->EvalAdd(nand, ct_phi3_mod);
                vals.push_back(decrypt_val(result));
            }
        }
        candidates.push_back({"φ⁴ - sum + φ³", vals});
    }

    // φ⁴ - sum + φ²
    {
        std::vector<double> vals;
        for (int a = 0; a <= 1; a++) {
            for (int b = 0; b <= 1; b++) {
                auto ct_a = a ? ct_one : ct_zero;
                auto ct_b = b ? ct_one : ct_zero;
                auto sum = cc->EvalAdd(ct_a, ct_b);
                auto nand = cc->EvalSub(ct_phi4_mod, sum);
                auto result = cc->EvalAdd(nand, ct_phi2_mod);
                vals.push_back(decrypt_val(result));
            }
        }
        candidates.push_back({"φ⁴ - sum + φ²", vals});
    }

    // φ⁴ - sum + φ
    {
        std::vector<double> vals;
        for (int a = 0; a <= 1; a++) {
            for (int b = 0; b <= 1; b++) {
                auto ct_a = a ? ct_one : ct_zero;
                auto ct_b = b ? ct_one : ct_zero;
                auto sum = cc->EvalAdd(ct_a, ct_b);
                auto nand = cc->EvalSub(ct_phi4_mod, sum);
                auto result = cc->EvalAdd(nand, ct_phi_mod);
                vals.push_back(decrypt_val(result));
            }
        }
        candidates.push_back({"φ⁴ - sum + φ", vals});
    }

    for (auto& cand : candidates) {
        for (double threshold : {0.0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8}) {
            std::vector<int> pattern;
            for (double v : cand.vals) {
                pattern.push_back(v > threshold ? 1 : 0);
            }
            if (pattern == std::vector<int>{1,1,1,0}) {
                std::cout << "✓ " << cand.name << " threshold=" << threshold << " → NAND!\n";
            }
        }
    }

    return 0;
}
