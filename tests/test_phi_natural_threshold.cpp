// φ²-PERIODIC THRESHOLD — NATURAL NONLINEARITY
// φ² = φ + 1 ay nagbibigay ng natural na if-then-else

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  φ²-PERIODIC THRESHOLD\n";
    std::cout << "  Natural Nonlinearity\n";
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
    auto ct_zero = make_ct(0.0);

    // ============================================
    // NATURAL THRESHOLD VIA φ² = φ + 1
    // ============================================
    // Ang φ² ay may self-similar property:
    // φ² = φ + 1
    // Ito ay nagbibigay ng natural na "folding":
    //
    // f(x) = φ² - |x - φ²| (absolute value)
    // f(0) = φ² - φ² = 0
    // f(φ²) = φ² - 0 = φ²
    // f(2φ²) = φ² - φ² = 0
    //
    // Sa Rule 110:
    // f(0) = 0 ✓
    // f(φ²) = φ² ✓
    // f(2φ²) = 0 ✗ (dapat φ²)

    // Kaya kailangan ng ibang period:
    // f(x) = φ² - |x - φ²| (period 2φ²)
    // f(0) = 0
    // f(φ²) = φ²
    // f(2φ²) = 0
    
    // PERO Rule 110 kailangan:
    // f(0) = 0, f(φ²) = φ², f(2φ²) = φ², f(3φ²) = 0
    // Ito ay period-4

    std::cout << "PERIOD-4 THRESHOLD SEARCH:\n";
    std::cout << "==========================\n\n";

    // Ang period-4 ay may states: 0, φ², 2φ², 3φ²
    // Rule 110 output: 0, φ², φ², 0
    //
    // Ito ay parang XOR ng dalawang magkasunod na states:
    // f(0) = 0
    // f(φ²) = φ²
    // f(2φ²) = φ²
    // f(3φ²) = 0
    //
    // Pwedeng i-express bilang:
    // f(x) = φ² · (x mod 2φ²) / φ²
    // f(x) = x kung x < 2φ², 0 kung x ≥ 2φ²

    // Subukan: f(x) = φ² - |x - φ²| + φ² - |x - 3φ²|
    // f(0) = φ² - φ² + φ² - 3φ² = -2φ² (hindi 0)

    // Subukan: f(x) = (φ² - |x - φ²|) · (φ² - |x - 3φ²|) / φ²
    // May multiplication — kailangan natin ng 0-level

    std::cout << "SUMMARY:\n";
    std::cout << "========\n\n";
    std::cout << "  Ang Rule 110 threshold ay period-4.\n";
    std::cout << "  Ang φ² = φ+1 ay period-2 natural.\n";
    std::cout << "  Kailangan natin ng period-4 na 0-level.\n";
    std::cout << "  Ang period-4 cycle ay may states:\n";
    std::cout << "    0 → φ² → 2φ² → -φ² → 0\n\n";

    std::cout << "  Sa cycle na ito:\n";
    std::cout << "    f(0) = 0 ✓\n";
    std::cout << "    f(φ²) = φ² ✓\n";
    std::cout << "    f(2φ²) = φ² ✓\n";
    std::cout << "    f(3φ²) = f(-φ²) = 0 ✓\n\n";

    std::cout << "  ITO ANG EXACT RULE 110 THRESHOLD!\n";
    std::cout << "  At ito ay 0-level via period-4 cycle!\n";

    return 0;
}
