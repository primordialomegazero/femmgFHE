// NAND VIA PERIOD-4 THRESHOLD
// sum ∈ {0, φ²} → φ², sum = 2φ² → 0
// Period-4 cycle bilang threshold

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  NAND VIA PERIOD-4 THRESHOLD\n";
    std::cout << "  2D Sum → Period-4 States\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_sq = phi * phi;
    const double two_phi_sq = 2 * phi_sq;

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
    auto ct_zero = make_ct(0.0);

    // Period-4 threshold:
    // f(0) = φ², f(φ²) = φ², f(2φ²) = 0
    // Ito ay: f(x) = φ² - |x - φ²| (absolute value sa φ domain)
    // Na sa period-4: f(x) = φ² - (x kung x<φ², 2φ²-x kung x>φ²)

    // Simpleng period-2 oscillation:
    // NAND(0,0): 0 → oscillate → φ² ✓
    // NAND(0,1): φ² → oscillate → φ² (kailangan ng special case)
    // NAND(1,1): 2φ² → oscillate → 0 ✓

    // Sa 2D slots (mula sa naunang test):
    // Ang sum ay may values: 0, φ², 2φ²
    // Period-4 mapping:
    // 0 → +φ² → φ² (state 1, output φ²)
    // φ² → +φ² → 2φ² (state 2, output 0) — MALI
    
    // Kailangan ng ibang mapping:
    // 0 → φ² (direct)
    // φ² → φ² (direct)
    // 2φ² → 0 (direct)

    // Ito ay piecewise — kailangan ng conditional

    std::cout << "PERIOD-4 STATES:\n";
    std::cout << "================\n\n";
    std::cout << "  State 0: 0 → NAND output φ²\n";
    std::cout << "  State 1: φ² → NAND output φ²\n";
    std::cout << "  State 2: 2φ² → NAND output 0\n\n";

    std::cout << "  Ang period-4 cycle: 0 → φ² → 2φ² → -φ² → 0\n";
    std::cout << "  Ay may natural na state transition\n";
    std::cout << "  Na kailangan nating i-adapt para sa NAND\n\n";

    std::cout << "  Level: 0\n";
    std::cout << "  Status: Period-4 threshold analysis\n";

    return 0;
}
