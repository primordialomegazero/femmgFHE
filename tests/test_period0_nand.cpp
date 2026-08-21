// PERIOD-0 NAND EXPLORATION
// Irrational rotation para sa NAND
// Hanapin kung may emergent na property

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  PERIOD-0 NAND EXPLORATION\n";
    std::cout << "  Irrational Rotation for NAND\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_sq = phi * phi;
    const double two_phi_sq = 2 * phi_sq;
    const double phi_mod = phi_sq - 2.0;  // 0.618034

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

    auto ct_phi_mod = make_ct(phi_mod);
    auto ct_phi_sq = make_ct(phi_sq);
    auto ct_zero = make_ct(0.0);

    // PERIOD-0 IRRATIONAL ROTATION:
    // x_{n+1} = (x_n + φ_mod) mod 1
    // Ang φ_mod = 0.618 ay irrational — walang period

    std::cout << "PERIOD-0 ROTATION (20 steps):\n";
    std::cout << "============================\n\n";

    auto state = ct_zero;
    std::vector<double> values;

    for (int step = 0; step < 20; step++) {
        state = cc->EvalAdd(state, ct_phi_mod);
        double v = decrypt_val(state);
        v = v - std::floor(v);  // mod 1
        values.push_back(v);

        std::cout << "  Step " << step << ": " << v << "\n";
    }

    std::cout << "\n";

    // NAND TEST VIA PERIOD-0
    std::cout << "NAND VIA PERIOD-0:\n";
    std::cout << "==================\n\n";

    // Subukan: NAND = φ² kung (a + b) mod 1 ∈ {0, φ_mod}
    // NAND = 0 kung (a + b) mod 1 ∈ {2φ_mod}

    auto nand_p0 = [&](auto a, auto b) {
        auto sum = cc->EvalAdd(a, b);
        // I-rotate sa period-0
        auto rotated = cc->EvalAdd(sum, ct_phi_mod);
        double v = decrypt_val(rotated);
        v = v - std::floor(v);
        
        // Threshold sa irrational space
        // Kung v < 0.5 → φ² (true)
        // Kung v >= 0.5 → 0 (false)
        return (v < 0.5) ? phi_sq : 0.0;
    };

    std::cout << "  NAND(0,0) = " << nand_p0(ct_zero, ct_zero) << "\n";
    std::cout << "  NAND(0,1) = " << nand_p0(ct_zero, ct_phi_sq) << "\n";
    std::cout << "  NAND(1,0) = " << nand_p0(ct_phi_sq, ct_zero) << "\n";
    std::cout << "  NAND(1,1) = " << nand_p0(ct_phi_sq, ct_phi_sq) << "\n\n";

    std::cout << "  Expected: φ², φ², φ², 0\n";
    std::cout << "  Level: 0\n";
    std::cout << "  Status: Period-0 NAND exploration\n";

    return 0;
}
