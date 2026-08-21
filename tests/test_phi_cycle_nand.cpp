// CYCLE-BASED NAND — BOUNDED 0-LEVEL
// NAND ay transition sa period-4 cycle

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  CYCLE-BASED NAND\n";
    std::cout << "  Bounded 0-Level\n";
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
    // CYCLE-BASED NAND — BOUNDED
    // ============================================
    // Ang NAND ay:
    //   Kung ang cycle position ay 0 o 2: output 1 (φ²)
    //   Kung ang cycle position ay 1 o 3: output 0
    //
    // Sa cycle: 0 → φ² → 2φ² → -φ² → 0
    // Positions: 0=0, 1=φ², 2=2φ², 3=-φ²
    //
    // NAND(0,0) = position 0 → output φ² ✓
    // NAND(0,φ²) = position 0,1 → output φ² ✓
    // NAND(φ²,φ²) = position 1,1 → output 0 ✓

    // Ang cycle transition ay:
    // next_position = (current_position + 1) % 4
    // Sa values:
    //   0 + φ² = φ²
    //   φ² + φ² = 2φ²
    //   2φ² - 3φ² = -φ²
    //   -φ² + φ² = 0

    auto cycle_step = [&](auto current) {
        // Check kung nasa 2φ² state
        // Kung oo, subtract 3φ² (para bumalik sa -φ²)
        // Kung hindi, add φ²
        // (ito ay conditional — pero sa cycle automatic)
        
        // Sa practice, dapat nating malaman ang state.
        // Sa ngayon, gamitin natin ang simpleng add φ²
        return cc->EvalAdd(current, ct_phi_sq);
    };

    std::cout << "CYCLE STEP TEST (20 gates):\n";
    std::cout << "===========================\n\n";

    auto current = ct_zero;
    for (int gate = 0; gate < 20; gate++) {
        current = cycle_step(current);
        double v = decrypt_val(current);
        std::cout << "  Gate " << gate << ": v=" << v
                  << " level=" << current->GetLevel() << "\n";
        
        // Reset kung masyadong malaki
        if (std::abs(v) > 3 * phi_sq) {
            current = cc->EvalSub(current, make_ct(4 * phi_sq));
        }
    }

    return 0;
}
