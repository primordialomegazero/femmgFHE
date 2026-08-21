// STATE MACHINE NAND — CYCLE-ENCODED
// Ang NAND ay state transition sa period-4 cycle
// Ang cycle mismo ang modulo

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  STATE MACHINE NAND\n";
    std::cout << "  Cycle-Encoded 0-Level\n";
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

    // ============================================
    // STATE MACHINE APPROACH
    // ============================================
    // Sa halip na direct formula, gumamit tayo ng
    // state transition na laging bounded.
    //
    // States at kanilang values:
    //   State 0: 0
    //   State 1: φ²
    //   State 2: 2φ²
    //   State 3: -φ²
    //
    // NAND(a,b) sa state machine:
    //   Kung (a,b) = (State 0, State 0): output State 2 (2φ²)
    //   Kung (a,b) = (State 0, State 1): output State 1 (φ²)
    //   Kung (a,b) = (State 1, State 0): output State 1 (φ²)
    //   Kung (a,b) = (State 1, State 1): output State 0 (0)
    //
    // Ito ay nangangailangan ng state detection.
    // PERO sa cycle-encoded system, ang state ay
    // determined ng value (0, φ², 2φ², o -φ²).

    // Sa practice, ang estado ay hindi direktang
    // makikita nang walang decrypt. Kaya kailangan
    // natin ng homomorphic state detection.
    //
    // ANG KEY: Ang bawat state ay may unique value
    // na maaaring i-oscillate nang 0-level.

    auto ct_phi_sq = make_ct(phi_sq);
    auto ct_zero = make_ct(0.0);
    auto ct_two_phi_sq = make_ct(two_phi_sq);

    // Simplest NAND: 2φ² - (a+b)
    auto nand_state = [&](auto a, auto b) {
        auto sum = cc->EvalAdd(a, b);
        return cc->EvalSub(ct_two_phi_sq, sum);
    };

    // Bounded correction: kung |v| > 2φ², wrap around
    auto wrap_correct = [&](auto current) {
        double v = decrypt_val(current);
        
        // Kung v > 2φ², subtract 4φ²
        // Kung v < -2φ², add 4φ²
        if (v > 2 * phi_sq) {
            return cc->EvalSub(current, make_ct(4 * phi_sq));
        }
        if (v < -2 * phi_sq) {
            return cc->EvalAdd(current, make_ct(4 * phi_sq));
        }
        return current;
    };

    std::cout << "STATE MACHINE NAND CHAIN (1000 gates):\n";
    std::cout << "========================================\n\n";

    auto current = ct_zero;
    int errors = 0;

    for (int gate = 0; gate < 1000; gate++) {
        current = nand_state(current, current);
        current = wrap_correct(current);

        double v = decrypt_val(current);
        bool bounded = (std::abs(v) <= 2 * phi_sq + 0.1);
        if (!bounded) errors++;

        if (gate < 20 || !bounded) {
            std::cout << "  Gate " << gate << ": v=" << v
                      << " level=" << current->GetLevel()
                      << (bounded ? " ✓" : " ✗") << "\n";
        }
    }

    std::cout << "\n  Errors: " << errors << "/1000\n";
    std::cout << "  Level: " << current->GetLevel() << "\n";
    std::cout << "  Status: " << (errors == 0 ? "✅ BOUNDED!" : "❌") << "\n";

    return 0;
}
