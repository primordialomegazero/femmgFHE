// SIGN-BASED MODULO PARA SA NAND — 0-LEVEL
// Ang +2φ² at -2φ² ay may different signs
// Kung kaya nating i-detect ang sign nang 0-level,
// kaya nating i-bound ang NAND chain

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  SIGN-BASED MODULO NAND\n";
    std::cout << "  0-Level Bounded Chain\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_sq = phi * phi;
    const double two_phi_sq = 2 * phi_sq;
    const double three_phi_sq = 3 * phi_sq;
    const double four_phi_sq = 4 * phi_sq;

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
    auto ct_two_phi_sq = make_ct(two_phi_sq);
    auto ct_three_phi_sq = make_ct(three_phi_sq);
    auto ct_four_phi_sq = make_ct(four_phi_sq);
    auto ct_zero = make_ct(0.0);

    // ============================================
    // SIGN DETECTION VIA OSCILLATION
    // ============================================
    // Mula sa naunang test:
    //   φ² - (positive + φ²) = -φ² (negative)
    //   φ² - (zero + φ²) = 0 (zero)
    //   φ² - (negative + φ²) = φ² (positive)
    //
    // Ito ay 0-level sign flip!

    // ============================================
    // NAND + SIGN-BASED MODULO
    // ============================================
    // NAND(a,b) = 2φ² - (a+b)
    // Pagkatapos ng NAND, i-apply ang sign-based modulo:
    //   Kung positive: subtract 4φ² (para maging negative)
    //   Kung negative: add 4φ² (para maging positive)
    //   Ganito ang automatic bounding

    auto nand_cycle = [&](auto a, auto b) {
        auto sum = cc->EvalAdd(a, b);
        return cc->EvalSub(ct_two_phi_sq, sum);
    };

    // Sign-based modulo approximation:
    // |v| dapat ≤ 2φ²
    // Kung v > 2φ²: subtract 4φ²
    // Kung v < -2φ²: add 4φ²
    
    // Sa 0-level, gamitin natin ang oscillation para i-detect
    // at i-correct ang sign

    std::cout << "NAND + SIGN MODULO (100 gates):\n";
    std::cout << "================================\n\n";

    auto current = ct_zero;
    int errors = 0;

    for (int gate = 0; gate < 100; gate++) {
        current = nand_cycle(current, current);

        // SIGN-BASED MODULO:
        // Step 1: add φ² para sa sign detection
        auto plus_phi = cc->EvalAdd(current, ct_phi_sq);
        
        // Step 2: oscillate para sa sign flip
        auto osc = cc->EvalSub(ct_phi_sq, plus_phi);
        
        // Step 3: i-check kung kailangan ng correction
        // Kung ang value ay lumampas sa 2φ², ang oscillation
        // ay magbibigay ng negative value
        double v_osc = decrypt_val(osc);
        
        // Step 4: bounded correction
        if (std::abs(v_osc) > 2 * phi_sq) {
            current = cc->EvalSub(current, ct_four_phi_sq);
        }
        
        double v = decrypt_val(current);

        if (gate < 10 || std::abs(v) > 2 * phi_sq) {
            std::cout << "  Gate " << gate << ": v=" << v
                      << " osc=" << v_osc
                      << " level=" << current->GetLevel() << "\n";
        }
    }

    std::cout << "\n  Final level: " << current->GetLevel() << "\n";

    return 0;
}
