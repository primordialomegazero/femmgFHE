// EXACT PERIOD-4 CYCLE — BOUNDED NAND
// Eksaktong 4-state cycle: 0 → φ² → 2φ² → -φ² → 0

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  EXACT PERIOD-4 CYCLE\n";
    std::cout << "  Bounded NAND Foundation\n";
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
    auto ct_zero = make_ct(0.0);
    auto ct_three_phi_sq = make_ct(three_phi_sq);
    auto ct_four_phi_sq = make_ct(four_phi_sq);

    // Eksaktong period-4 cycle:
    // State 0: 0
    // State 1: φ²
    // State 2: 2φ²
    // State 3: -φ²
    // Balik sa State 0: 0
    //
    // Transitions:
    // 0 + φ² = φ²
    // φ² + φ² = 2φ²
    // 2φ² - 3φ² = -φ²
    // -φ² + φ² = 0

    auto exact_cycle_step = [&](auto current, int step_num) {
        if (step_num % 4 == 2) {
            // Sa 2φ² state, subtract 3φ² para sa -φ²
            return cc->EvalSub(current, ct_three_phi_sq);
        } else {
            // Lahat ng iba ay add φ²
            return cc->EvalAdd(current, ct_phi_sq);
        }
    };

    std::cout << "EXACT 4-CYCLE TEST (1000 gates):\n";
    std::cout << "================================\n\n";

    auto current = ct_zero;
    int errors = 0;

    for (int gate = 0; gate < 1000; gate++) {
        current = exact_cycle_step(current, gate);
        
        double v = decrypt_val(current);
        double expected;
        if (gate % 4 == 0) expected = phi_sq;
        else if (gate % 4 == 1) expected = two_phi_sq;
        else if (gate % 4 == 2) expected = -phi_sq;
        else expected = 0.0;
        
        bool ok = (std::abs(v - expected) < 0.1 * phi_sq);
        if (!ok) errors++;

        if (gate < 20 || !ok) {
            std::cout << "  Gate " << gate << ": v=" << v
                      << " exp=" << expected
                      << " level=" << current->GetLevel()
                      << (ok ? " ✓" : " ✗") << "\n";
        }
    }

    std::cout << "\n  Errors: " << errors << "/1000\n";
    std::cout << "  Level: " << current->GetLevel() << "\n";
    std::cout << "  Status: " << (errors == 0 ? "✅ EXACT CYCLE!" : "❌ BROKEN") << "\n";

    return 0;
}
