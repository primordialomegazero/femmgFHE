// TRIPLE OSCILLATION — 10,000 STEPS STABILITY
// Period-3 map na bounded at 0-level

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
    std::cout << "  TRIPLE OSCILLATION — 10K STEPS\n";
    std::cout << "  Period-3 Stability Test\n";
    std::cout << "========================================\n\n";

    const double phi_sq = 2.618033988749895;
    const double two_phi_sq = 2 * phi_sq;
    const double three_phi_sq = 3 * phi_sq;
    const double six_phi_sq = 6 * phi_sq;

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

    auto make_state = [&](double val) {
        std::vector<std::complex<double>> vec(slots, {0.0, 0.0});
        vec[128] = {val, 0.0};
        return cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec));
    };

    auto decrypt_slot = [&](auto ct, int slot_num) {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetCKKSPackedValue()[slot_num].real();
    };

    auto state = make_state(phi_sq);
    int errors = 0;
    
    auto start = high_resolution_clock::now();
    
    for (int step = 0; step < 10000; step++) {
        // Triple oscillation: cycle K through {φ², 2φ², 3φ²}
        double K;
        switch (step % 3) {
            case 0: K = phi_sq; break;
            case 1: K = two_phi_sq; break;
            case 2: K = three_phi_sq; break;
        }
        
        state = cc->EvalSub(make_state(K), state);
        
        double v = decrypt_slot(state, 128);
        bool bounded = (std::abs(v) <= six_phi_sq + 0.01);
        
        if (step < 10 || step % 1000 == 0 || step == 9999) {
            std::cout << "  Step " << step << ": v=" << v 
                      << " level=" << state->GetLevel()
                      << (bounded ? " ✓" : " ✗") << "\n";
        }
        
        if (!bounded) {
            errors++;
            if (errors > 5) {
                std::cout << "  → EXPLODING!\n";
                break;
            }
        }
    }
    
    auto end = high_resolution_clock::now();
    auto dur = duration_cast<seconds>(end - start);
    
    std::cout << "\n  Errors: " << errors << " / 10,000\n";
    std::cout << "  Time: " << dur.count() << "s\n";
    std::cout << "  Level: " << state->GetLevel() << "\n";
    std::cout << "  Status: " << (errors == 0 ? "🏆 PERIOD-3 STABLE!" : "❌ NOISE GROWTH") << "\n";

    return 0;
}
