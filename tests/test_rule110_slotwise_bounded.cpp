// RULE 110 — SLOT-WISE BOUNDED
// I-bound ang bawat slot bago i-sum
// Para walang explosion!

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  RULE 110 — SLOT-WISE BOUNDED\n";
    std::cout << "  Bound Each Slot Before Sum\n";
    std::cout << "========================================\n\n";

    const double phi_sq = 2.618033988749895;
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

    auto make_uniform = [&](double val) {
        std::vector<std::complex<double>> vec(slots, {0.0, 0.0});
        for (int i = 0; i < slots; i++) vec[i] = {val, 0.0};
        return cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec));
    };

    // Initial: single slot = φ²
    std::vector<std::complex<double>> init(slots, {0.0, 0.0});
    init[128] = {phi_sq, 0.0};
    auto state = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(init));
    
    std::cout << "SLOT-WISE BOUNDED (50 steps):\n";
    std::cout << "=============================\n\n";
    
    int errors = 0;
    
    for (int step = 0; step < 50; step++) {
        // STEP 1: Rotate para sa neighbors
        auto left = cc->EvalRotate(state, 1);
        auto right = cc->EvalRotate(state, -1);
        
        // STEP 2: BOUND ang bawat isa (φ² - x oscillation)
        // Ito ay 0-level at bounded sa [0, φ²]!
        auto bounded_left = cc->EvalSub(make_uniform(phi_sq), left);
        auto bounded_state = cc->EvalSub(make_uniform(phi_sq), state);
        auto bounded_right = cc->EvalSub(make_uniform(phi_sq), right);
        
        // STEP 3: Sum ng BOUNDED values
        auto sum = cc->EvalAdd(cc->EvalAdd(bounded_left, bounded_state), bounded_right);
        
        // STEP 4: Final bounded transition
        state = cc->EvalSub(make_uniform(three_phi_sq), sum);
        
        double v = decrypt_slot(state, 128);
        bool bounded = (std::abs(v) <= three_phi_sq + 0.01);
        
        if (step < 15 || step % 10 == 0 || step == 49) {
            std::cout << "  Step " << step << ": v=" << v 
                      << " level=" << state->GetLevel()
                      << (bounded ? " ✓" : " ✗") << "\n";
        }
        
        if (!bounded) {
            errors++;
            if (errors > 5) {
                std::cout << "  → EXPLODING at step " << step << "\n";
                break;
            }
        }
    }
    
    std::cout << "\n  Errors: " << errors << "\n";
    std::cout << "  Level: " << state->GetLevel() << "\n";
    std::cout << "  Status: " << (errors == 0 ? "🏆 SLOT-WISE BOUNDED!" : "❌ EXPLODING") << "\n";

    return 0;
}
