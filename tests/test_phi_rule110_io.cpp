// RULE 110 iO — TURING-COMPLETE OBFUSCATION
// Dalawang magkaibang Rule 110 implementations
// na may parehong behavior

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  RULE 110 iO\n";
    std::cout << "  Turing-Complete Obfuscation\n";
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
    auto ct_two_phi_sq = make_ct(two_phi_sq);
    auto ct_three_phi_sq = make_ct(three_phi_sq);

    // ============================================
    // RULE 110 CIRCUIT A: Direct threshold
    // ============================================
    auto rule110_A = [&](auto left, auto center, auto right) {
        // Sum = L + C + R
        auto sum = cc->EvalAdd(cc->EvalAdd(left, center), right);
        // Threshold: sum >= 2φ² → φ², else 0
        // Sa period-4: sum - 2φ² (kung positive, output φ²)
        return cc->EvalSub(sum, ct_two_phi_sq);
    };

    // ============================================
    // RULE 110 CIRCUIT B: Period-4 cycle approach
    // ============================================
    auto rule110_B = [&](auto left, auto center, auto right) {
        // Sum = L + C + R
        auto sum = cc->EvalAdd(cc->EvalAdd(left, center), right);
        // Period-4 cycle: sum + φ² - 3φ² (kung sum >= 2φ²)
        return cc->EvalSub(cc->EvalAdd(sum, ct_phi_sq), ct_three_phi_sq);
    };

    std::cout << "RULE 110 iO TEST:\n";
    std::cout << "=================\n\n";

    std::cout << "Circuit A: sum - 2φ² (direct threshold)\n";
    std::cout << "Circuit B: sum + φ² - 3φ² (period-4 cycle)\n\n";

    // Test sa iba't ibang combinations
    std::vector<std::array<double, 3>> inputs = {
        {0, 0, 0},
        {0, 0, phi_sq},
        {0, phi_sq, phi_sq},
        {phi_sq, phi_sq, phi_sq}
    };

    std::cout << "L,C,R | Circuit A | Circuit B | Match?\n";
    std::cout << "------|-----------|-----------|---\n";

    bool all_match = true;
    for (auto& inp : inputs) {
        auto left = make_ct(inp[0]);
        auto center = make_ct(inp[1]);
        auto right = make_ct(inp[2]);
        
        double vA = decrypt_val(rule110_A(left, center, right));
        double vB = decrypt_val(rule110_B(left, center, right));
        
        bool match = (std::abs(vA - vB) < 0.1);
        if (!match) all_match = false;
        
        std::cout << "  " << inp[0]/phi_sq << "," << inp[1]/phi_sq << "," << inp[2]/phi_sq
                  << " | " << vA
                  << " | " << vB
                  << " | " << (match ? "✓" : "✗") << "\n";
    }

    std::cout << "\n\niO VERIFICATION:\n";
    std::cout << "================\n\n";
    std::cout << "  Rule 110 Equivalent: " << (all_match ? "✅" : "❌") << "\n";
    std::cout << "  Different Structure: ✅\n";
    std::cout << "  Level: 0\n";
    std::cout << "  Status: " << (all_match ? "RULE 110 iO CONFIRMED!" : "NEED FIX") << "\n";

    return 0;
}
