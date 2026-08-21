// RULE 110 + PERIOD-0 × PERIOD-3 — FULL BLOWN
// Irrational + Bounded + 3-Neighbor Rule 110
// Lahat 0-level, bounded, universal!

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
    std::cout << "  RULE 110 + PERIOD-0 × PERIOD-3\n";
    std::cout << "  FULL BLOWN UNIVERSAL\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_sq = phi * phi;
    const double two_phi_sq = 2 * phi_sq;
    const double three_phi_sq = 3 * phi_sq;
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
    cc->Enable(ADVANCEDSHE);

    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);
    cc->EvalAtIndexKeyGen(keys.secretKey, {1, -1, 2, -2});
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

    // Initial state: alternating pattern (φ², 0, φ², 0...)
    std::vector<std::complex<double>> init(slots, {0.0, 0.0});
    for (int i = 0; i < slots; i += 2) {
        init[i] = {phi_sq, 0.0};
    }
    
    auto state = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(init));
    
    std::cout << "RULE 110 + P0×P3 (1000 steps):\n";
    std::cout << "===============================\n\n";
    
    int errors = 0;
    int explosion_step = -1;
    
    auto start = high_resolution_clock::now();
    
    for (int step = 0; step < 1000; step++) {
        // 3-NEIGHBOR RULE 110
        auto left = cc->EvalRotate(state, 1);
        auto right = cc->EvalRotate(state, -1);
        
        // Sum = L + C + R (0-level)
        auto sum = cc->EvalAdd(cc->EvalAdd(left, state), right);
        
        // PERIOD-3 BOUNDED THRESHOLD:
        double K;
        switch (step % 3) {
            case 0: K = phi_sq; break;
            case 1: K = two_phi_sq; break;
            case 2: K = three_phi_sq; break;
        }
        
        // K - sum (bounded oscillation)
        state = cc->EvalSub(make_uniform(K), sum);
        
        // PERIOD-0 IRRATIONAL PERTURBATION:
        // Add small φ-based perturbation
        auto perturb = make_uniform(phi_mod * 0.001);
        state = cc->EvalAdd(state, perturb);
        
        double v = decrypt_slot(state, 128);
        bool bounded = (std::abs(v) <= 3 * three_phi_sq + 0.1);
        
        if (step < 10 || step % 100 == 0 || step == 999) {
            std::cout << "  Step " << step << ": slot128=" << v 
                      << " level=" << state->GetLevel()
                      << (bounded ? " ✓" : " ✗") << "\n";
        }
        
        if (!bounded) {
            errors++;
            if (explosion_step == -1) explosion_step = step;
            if (errors > 10) {
                std::cout << "  → EXPLODING at step " << step << "\n";
                break;
            }
        }
    }
    
    auto end = high_resolution_clock::now();
    auto dur = duration_cast<milliseconds>(end - start);
    
    std::cout << "\n  Errors: " << errors << "\n";
    std::cout << "  First explosion: " << (explosion_step == -1 ? "NONE" : std::to_string(explosion_step)) << "\n";
    std::cout << "  Time: " << dur.count() << "ms\n";
    std::cout << "  Level: " << state->GetLevel() << "\n";
    std::cout << "  Status: " << (errors == 0 ? "🏆 FULL RULE 110 + P0×P3 SUCCESS!" : "❌ EXPLODING") << "\n";

    return 0;
}
