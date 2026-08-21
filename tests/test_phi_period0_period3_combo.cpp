// PERIOD-0 × PERIOD-3 COMBINATION
// Irrational rotation + bounded oscillation
// x_{n+1} = K_i - (x_n + φ_mod)
// Kung saan K_i cycles sa {φ², 2φ², 3φ²}

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
    std::cout << "  PERIOD-0 × PERIOD-3 COMBINATION\n";
    std::cout << "  Irrational + Bounded Oscillation\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_sq = phi * phi;
    const double two_phi_sq = 2 * phi_sq;
    const double three_phi_sq = 3 * phi_sq;
    const double phi_mod = phi_sq - 2.0;  // 0.618...

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

    // ============================================
    // COMBINATION MAP:
    // x_{n+1} = K_i - (x_n + φ_mod)
    // K_i cycles: φ², 2φ², 3φ²
    // φ_mod = 0.618 (irrational perturbation)
    // ============================================
    
    std::cout << "COMBINATION MAP:\n";
    std::cout << "  x_{n+1} = K_i - (x_n + φ_mod)\n";
    std::cout << "  K_i ∈ {φ², 2φ², 3φ²}\n";
    std::cout << "  φ_mod = 0.618034\n\n";
    
    auto state = make_state(0.0);
    int errors = 0;
    std::vector<double> trajectory;
    
    std::cout << "PERIOD-0 × PERIOD-3 TEST (100 steps):\n";
    std::cout << "=====================================\n\n";
    
    for (int step = 0; step < 100; step++) {
        // Add φ_mod (irrational)
        auto temp = cc->EvalAdd(state, make_state(phi_mod));
        
        // K_i - temp (bounded oscillation)
        double K;
        switch (step % 3) {
            case 0: K = phi_sq; break;
            case 1: K = two_phi_sq; break;
            case 2: K = three_phi_sq; break;
        }
        
        state = cc->EvalSub(make_state(K), temp);
        
        double v = decrypt_slot(state, 128);
        trajectory.push_back(v);
        
        bool bounded = (std::abs(v) <= three_phi_sq + phi_mod + 0.01);
        
        if (step < 20) {
            std::cout << "  Step " << step << ": v=" << v 
                      << " level=" << state->GetLevel()
                      << (bounded ? " ✓" : " ✗") << "\n";
        }
        
        if (!bounded) {
            errors++;
            if (errors > 5) break;
        }
    }
    
    // Check for periodicity
    bool has_period = false;
    for (int p = 1; p < 50; p++) {
        bool is_periodic = true;
        for (int i = 0; i < 30; i++) {
            if (std::abs(trajectory[i] - trajectory[i + p]) > 0.001) {
                is_periodic = false;
                break;
            }
        }
        if (is_periodic) {
            has_period = true;
            std::cout << "\n  PERIOD FOUND: " << p << " steps\n";
            break;
        }
    }
    
    if (!has_period) {
        std::cout << "\n  WALANG PERIOD — IRRATIONAL! (Period-0 ✓)\n";
    }
    
    std::cout << "\n  Bounded: " << (errors == 0 ? "✓" : "✗") << "\n";
    std::cout << "  Level: " << state->GetLevel() << "\n\n";
    
    // ============================================
    // 10,000 STEPS LONG TEST
    // ============================================
    std::cout << "LONG TEST (10,000 steps):\n";
    std::cout << "=========================\n\n";
    
    auto state_long = make_state(0.0);
    int errors_long = 0;
    std::vector<double> traj_long;
    
    auto start = high_resolution_clock::now();
    
    for (int step = 0; step < 10000; step++) {
        auto temp = cc->EvalAdd(state_long, make_state(phi_mod));
        
        double K;
        switch (step % 3) {
            case 0: K = phi_sq; break;
            case 1: K = two_phi_sq; break;
            case 2: K = three_phi_sq; break;
        }
        
        state_long = cc->EvalSub(make_state(K), temp);
        
        double v = decrypt_slot(state_long, 128);
        traj_long.push_back(v);
        
        if (std::abs(v) > three_phi_sq + phi_mod + 0.01) {
            errors_long++;
            if (errors_long > 5) {
                std::cout << "  → EXPLODING at step " << step << "\n";
                break;
            }
        }
        
        if (step < 5 || step % 1000 == 0 || step == 9999) {
            std::cout << "  Step " << step << ": v=" << v 
                      << " level=" << state_long->GetLevel() << "\n";
        }
    }
    
    auto end = high_resolution_clock::now();
    auto dur = duration_cast<milliseconds>(end - start);
    
    // Check for repeats
    bool has_repeat_long = false;
    for (int i = 0; i < (int)traj_long.size(); i++) {
        for (int j = i + 1; j < (int)traj_long.size(); j++) {
            if (std::abs(traj_long[i] - traj_long[j]) < 0.001) {
                has_repeat_long = true;
                std::cout << "  REPEAT: step " << i << " = step " << j << "\n";
                break;
            }
        }
        if (has_repeat_long) break;
    }
    
    std::cout << "\n  Errors: " << errors_long << " / 10,000\n";
    std::cout << "  Time: " << dur.count() << "ms\n";
    std::cout << "  Repeat: " << (has_repeat_long ? "PERIODIC" : "PERIOD-0 (irrational!)") << "\n";
    std::cout << "  Level: " << state_long->GetLevel() << "\n";
    std::cout << "  Status: " << (errors_long == 0 && !has_repeat_long 
              ? "🏆 PERIOD-0 × PERIOD-3 SUCCESS!" : "❌ MAY ISSUE") << "\n";

    return 0;
}
