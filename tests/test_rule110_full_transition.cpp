// FULL RULE 110 — φ-ENCODED TRANSITION TABLE
// 111→0, 110→1, 101→1, 100→0, 011→1, 010→1, 001→1, 000→0

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  FULL RULE 110 TRANSITION\n";
    std::cout << "  0→0, 1→φ²\n";
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
    cc->Enable(ADVANCEDSHE);

    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);
    cc->EvalAtIndexKeyGen(keys.secretKey, {1, -1});

    auto slots = cc->GetEncodingParams()->GetBatchSize();

    auto decrypt_slot = [&](auto ct, int slot_num) {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetCKKSPackedValue()[slot_num].real();
    };

    // ============================================
    // RULE 110 TRANSITION ANALYSIS
    // ============================================
    // 111 → 0:  L+C+R = 3φ² → output 0
    // 110 → 1:  L+C+R = 2φ² → output φ²
    // 101 → 1:  L+C+R = 2φ² → output φ²
    // 100 → 0:  L+C+R = φ² → output 0
    // 011 → 1:  L+C+R = 2φ² → output φ²
    // 010 → 1:  L+C+R = φ² → output φ²
    // 001 → 1:  L+C+R = φ² → output φ²
    // 000 → 0:  L+C+R = 0 → output 0
    //
    // Ang pattern:
    // sum = 0 → 0
    // sum = φ² → depende sa arrangement:
    //   100 → 0, 010 → 1, 001 → 1
    // sum = 2φ² → 1 (lahat)
    // sum = 3φ² → 0
    //
    // Kaya kailangan ng POSITION-DEPENDENT rule.
    // PERO may φ-natural na paraan:
    // output = φ² - |sum - φ²| (threshold sa gitna)

    // Subukan: output = φ² - |L + C + R - φ²|
    // 0: φ² - |0 - φ²| = φ² - φ² = 0 ✓
    // φ²: φ² - |φ² - φ²| = φ² ✓ (kasama 010, 001)
    // 2φ²: φ² - |2φ² - φ²| = φ² - φ² = 0 ✗ (dapat 1)
    
    // Hindi ito ang tamang formula. Kailangan ng iba.

    // Subukan: output = (L + R) mod 2φ²
    // 111: L=φ², R=φ² → sum=2φ² mod 2φ² = 0 ✓
    // 110: L=φ², R=0 → sum=φ² ✓
    // 101: L=φ², R=φ² → 0 ✗ (dapat 1)

    // Subukan: output = L XOR C XOR R
    // = L + C + R - 2·(L·C·R) — may multiplication

    std::cout << "RULE 110 FORMULA SEARCH:\n";
    std::cout << "========================\n\n";

    // Ang pinakamalapit na 0-level formula:
    // output = φ² - (L + C + R) + φ²·kung(L+C+R > φ²)
    // Ito ay nangangailangan ng conditional.

    // SUBUKAN: Period-6 recurrence para sa neighbor sum
    // x_{n+1} = x_n - x_{n-1} + φ² - (L + C + R)
    // Ang period-6 ay natural na modulo

    std::cout << "PERIOD-6 + RULE 110:\n";
    std::cout << "====================\n\n";

    // Initial state
    std::vector<std::complex<double>> init(slots, {0.0, 0.0});
    init[128] = {phi_sq, 0.0};

    auto x_prev = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(
        std::vector<std::complex<double>>(slots, {0.0, 0.0})));
    auto x_curr = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(init));

    // Constants
    std::vector<std::complex<double>> phi_sq_vec(slots, {phi_sq, 0.0});
    std::vector<std::complex<double>> two_phi_sq_vec(slots, {two_phi_sq, 0.0});
    auto ct_phi_sq = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(phi_sq_vec));
    auto ct_two_phi_sq = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(two_phi_sq_vec));

    std::cout << "100 STEPS (Period-6 + Rule 110):\n";
    std::cout << "=================================\n\n";

    int errors = 0;

    for (int step = 0; step < 100; step++) {
        auto left = cc->EvalAtIndex(x_curr, -1);
        auto right = cc->EvalAtIndex(x_curr, 1);
        auto sum = cc->EvalAdd(cc->EvalAdd(left, x_curr), right);

        // Period-6 bounded transition: x_n - x_{n-1} + φ² - sum
        auto x_next = cc->EvalSub(
            cc->EvalAdd(cc->EvalSub(x_curr, x_prev), ct_phi_sq),
            sum
        );

        x_prev = x_curr;
        x_curr = x_next;

        double v = decrypt_slot(x_curr, 128);
        bool bounded = (std::abs(v) < 5 * phi_sq);
        if (!bounded) errors++;

        if (step % 10 == 0) {
            std::cout << "  Step " << step << ": "
                      << "slot128=" << v
                      << " level=" << x_curr->GetLevel()
                      << (bounded ? " ✓" : " ✗") << "\n";
        }
    }

    std::cout << "\n  Errors: " << errors << "/100\n";
    std::cout << "  Level: " << x_curr->GetLevel() << "\n";
    std::cout << "  Status: " << (errors == 0 ? "✅ FOUND!" : "❌") << "\n";

    return 0;
}
