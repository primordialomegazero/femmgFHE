// RULE 110 + P0×3 — 10,000 STEPS STRESS TEST
// I-verify ang long-term stability

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
    std::cout << "  RULE 110 + P0×3 — 10K STRESS\n";
    std::cout << "  Long-Term Stability Test\n";
    std::cout << "========================================\n\n";

    const double phi_sq = 2.618033988749895;
    const double two_phi_sq = 2 * phi_sq;
    const double three_phi_sq = 3 * phi_sq;
    const double phi_mod = phi_sq - 2.0;

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

    // Initial: non-trivial pattern
    std::vector<std::complex<double>> init(slots, {0.0, 0.0});
    for (int i = 0; i < slots; i++) {
        bool bit = ((i * 7 + 3) % 11) > 5;
        init[i] = {bit ? phi_sq : 0.0, 0.0};
    }
    auto state = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(init));
    
    std::cout << "10,000 STEPS TEST:\n";
    std::cout << "==================\n\n";
    
    int errors = 0;
    auto start = high_resolution_clock::now();
    
    for (int step = 0; step < 10000; step++) {
        auto left = cc->EvalRotate(state, 1);
        auto right = cc->EvalRotate(state, -1);
        
        // K_i from neighbors (period-3)
        auto sum_neighbors = cc->EvalAdd(left, right);
        auto K = cc->EvalSub(make_uniform(three_phi_sq), sum_neighbors);
        
        // P0×3: state = K - (state + φ_mod)
        auto temp = cc->EvalAdd(state, make_uniform(phi_mod));
        state = cc->EvalSub(K, temp);
        
        double v = decrypt_slot(state, 128);
        bool bounded = (std::abs(v) <= 3 * three_phi_sq + 0.1);
        
        if (step < 5 || step % 1000 == 0 || step == 9999) {
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
    
    auto end = high_resolution_clock::now();
    auto dur = duration_cast<seconds>(end - start);
    
    std::cout << "\n  Errors: " << errors << " / 10,000\n";
    std::cout << "  Time: " << dur.count() << "s\n";
    std::cout << "  Level: " << state->GetLevel() << "\n";
    std::cout << "  Status: " << (errors == 0 ? "🏆 P0×3 STABLE!" : "❌ EXPLODING") << "\n";

    return 0;
}
