// φ-MODULAR NAND — BOUNDED 0-LEVEL
// Ang φ² ay may natural period sa golden ratio space
// Kaya ang values ay dapat mag-wrap, hindi lumaki

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  φ-MODULAR NAND — BOUNDED\n";
    std::cout << "  Natural Wrap-Around\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_sq = phi * phi;
    const double two_phi_sq = 2 * phi_sq;
    const double phi_inv = 1.0 / phi;

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
    // MODULAR OSCILLATION
    // ============================================
    // Sa halip na φ² - x (na linear), gumamit ng
    // period-4 cycle na natural na bounded:
    // 0 → φ² → 2φ² → -φ² → 0 (cycle)
    
    // Ang cycle na ito ay kayang gawin sa 0-level:
    // step 1: add φ²
    // step 2: add φ²
    // step 3: subtract 3φ² (para bumalik sa 0)

    std::cout << "PERIOD-4 BOUNDED CYCLE:\n";
    std::cout << "=======================\n\n";
    std::cout << "  0 → φ² → 2φ² → -φ² → 0\n\n";

    auto current = ct_zero;
    int errors = 0;
    int total_gates = 1000;

    for (int gate = 0; gate < total_gates; gate++) {
        // Period-4 cycle
        if (gate % 4 == 0) {
            current = cc->EvalAdd(current, ct_phi_sq);  // +φ²
        } else if (gate % 4 == 1) {
            current = cc->EvalAdd(current, ct_phi_sq);  // +φ²
        } else if (gate % 4 == 2) {
            current = cc->EvalSub(current, make_ct(3 * phi_sq)); // -3φ²
        } else {
            current = cc->EvalAdd(current, ct_phi_sq);  // +φ²
        }

        double v = decrypt_val(current);
        
        // Expected bounded values: 0, φ², 2φ², -φ²
        double expected;
        if (gate % 4 == 0) expected = phi_sq;
        else if (gate % 4 == 1) expected = two_phi_sq;
        else if (gate % 4 == 2) expected = -phi_sq;
        else expected = 0.0;
        
        bool ok = (std::abs(v - expected) < 0.1 * phi_sq);
        if (!ok) errors++;

        if (gate < 10 || gate % 100 == 0 || !ok) {
            std::cout << "  Gate " << gate << ": v=" << v
                      << " exp=" << expected
                      << " level=" << current->GetLevel()
                      << (ok ? " ✓" : " ✗") << "\n";
        }
    }

    std::cout << "\n  Errors: " << errors << "/" << total_gates << "\n";
    std::cout << "  Level: " << current->GetLevel() << "\n";
    std::cout << "  Status: " << (errors == 0 ? "✅ BOUNDED!" : "❌ UNBOUNDED") << "\n\n";

    std::cout << "NEXT STEP:\n";
    std::cout << "==========\n\n";
    std::cout << "  Kung ang period-4 cycle ay bounded,\n";
    std::cout << "  maaari nating gamitin ito bilang\n";
    std::cout << "  modulo para sa NAND na 0-level.\n";

    return 0;
}
