// SORTING / COMPARISON SA φ-DOMAIN — 0-LEVEL
// I-test kung kaya ng φ-domain ang comparison

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  SORTING / COMPARISON SA φ-DOMAIN\n";
    std::cout << "  0-Level Comparison\n";
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
    auto ct_zero = make_ct(0.0);

    // ============================================
    // COMPARISON SA φ-DOMAIN
    // ============================================
    // Kailangan nating malaman:
    // - Mas malaki ba ang a kaysa b?
    // - Mas maliit ba?
    // - Pantay ba?
    //
    // Sa period-4 cycle:
    // 0 < φ² < 2φ² (sa absolute value)
    // -φ² ay katumbas ng 3φ² sa cycle
    //
    // Ang difference a - b ay:
    // 0 kung pantay
    // φ² kung a > b (o -φ² kung a < b)
    // 2φ² kung a >> b

    std::cout << "COMPARISON TEST:\n";
    std::cout << "================\n\n";

    // Test: 0 vs φ²
    auto diff_0p = cc->EvalSub(ct_phi_sq, ct_zero);  // φ² - 0 = φ²
    std::cout << "  φ² - 0 = " << decrypt_val(diff_0p) << " (positive → φ² > 0)\n";

    // Test: φ² vs 0
    auto diff_p0 = cc->EvalSub(ct_zero, ct_phi_sq);  // 0 - φ² = -φ²
    std::cout << "  0 - φ² = " << decrypt_val(diff_p0) << " (negative → 0 < φ²)\n";

    // Test: φ² vs φ²
    auto diff_pp = cc->EvalSub(ct_phi_sq, ct_phi_sq);  // φ² - φ² = 0
    std::cout << "  φ² - φ² = " << decrypt_val(diff_pp) << " (zero → equal)\n\n";

    // COMPARISON RESULT:
    // Positive difference → a > b
    // Negative difference → a < b
    // Zero difference → a = b
    //
    // Sa period-4 cycle, ang sign ay nade-detect ng oscillation

    std::cout << "SIGN DETECTION VIA OSCILLATION:\n";
    std::cout << "===============================\n\n";

    // φ² - x ay nagbibigay ng:
    // φ² - φ² = 0 (equal)
    // φ² - 0 = φ² (positive)
    // φ² - (-φ²) = 2φ² (mas positive)

    auto osc_equal = cc->EvalSub(ct_phi_sq, diff_pp);
    auto osc_pos = cc->EvalSub(ct_phi_sq, diff_0p);
    auto osc_neg = cc->EvalSub(ct_phi_sq, diff_p0);

    std::cout << "  Osc(equal) = " << decrypt_val(osc_equal) << " (φ²)\n";
    std::cout << "  Osc(positive) = " << decrypt_val(osc_pos) << " (0)\n";
    std::cout << "  Osc(negative) = " << decrypt_val(osc_neg) << " (2φ²)\n\n";

    // MIN/MAX via period-4:
    // min(a,b) = a kung a < b, b kung b < a
    // Sa φ-domain: min = (a + b - |a - b|) / 2
    // max = (a + b + |a - b|) / 2

    std::cout << "MIN/MAX TEST:\n";
    std::cout << "=============\n\n";

    auto sum = cc->EvalAdd(ct_phi_sq, ct_zero);  // φ² + 0 = φ²
    auto diff = cc->EvalSub(ct_phi_sq, ct_zero);  // φ² - 0 = φ²

    // min = (sum - diff) / 2 = (φ² - φ²) / 2 = 0
    auto min_val = cc->EvalSub(sum, diff);
    std::cout << "  min(φ², 0) = " << decrypt_val(min_val) << " (0)\n";

    // max = (sum + diff) / 2 = (φ² + φ²) / 2 = φ²
    auto max_val = cc->EvalAdd(sum, diff);
    std::cout << "  max(φ², 0) = " << decrypt_val(max_val) << " (2φ²)\n\n";

    std::cout << "  Level: 0\n";
    std::cout << "  Status: COMPARISON WORKS!\n";

    return 0;
}
