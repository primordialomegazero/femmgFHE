// PURE PERIOD-0 — CORRECTED MODULO
// x_n = (φ²·n) mod 1 — with correct modulo handling

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
    std::cout << "  PURE PERIOD-0 — CORRECTED\n";
    std::cout << "  x_n = (φ²·n) mod 1\n";
    std::cout << "========================================\n\n";

    const double phi_sq = 2.618033988749895;
    const double phi_mod = phi_sq - 2.0;  // 0.618033988749895

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

    // CORRECT PURE PERIOD-0:
    // x_n = (φ_mod · n) mod 1
    // kung saan φ_mod = φ² - 2 = 0.618033988749895
    
    auto state = make_ct(0.0);
    auto ct_phi_mod = make_ct(phi_mod);
    auto ct_one = make_ct(1.0);
    
    std::cout << "PURE PERIOD-0 (CORRECTED):\n";
    std::cout << "==========================\n\n";
    
    int errors = 0;
    std::vector<double> trajectory;
    
    for (int i = 0; i < 100; i++) {
        // Add φ_mod (0.618...)
        state = cc->EvalAdd(state, ct_phi_mod);
        
        // Modulo 1: kung ≥ 1, subtract 1
        double v = decrypt_val(state);
        if (v >= 1.0) {
            state = cc->EvalSub(state, ct_one);
            v = decrypt_val(state);
        }
        
        trajectory.push_back(v);
        
        bool bounded = (v >= 0.0 && v < 1.0);
        if (!bounded) errors++;
        
        if (i < 20) {
            std::cout << "  Step " << i << ": v=" << v 
                      << " level=" << state->GetLevel()
                      << (bounded ? " ✓" : " ✗") << "\n";
        }
    }
    
    std::cout << "\n  Bounded: " << (errors == 0 ? "✓ Lahat sa [0,1)" : "✗ May lumabas") << "\n";
    std::cout << "  Level: " << state->GetLevel() << "\n\n";
    
    // ============================================
    // 10,000 STEPS LONG TEST
    // ============================================
    std::cout << "LONG TEST (10,000 steps):\n";
    std::cout << "=========================\n\n";
    
    auto state_long = make_ct(0.0);
    int errors_long = 0;
    bool has_repeat = false;
    std::vector<double> traj_long;
    
    auto start = high_resolution_clock::now();
    
    for (int i = 0; i < 10000; i++) {
        state_long = cc->EvalAdd(state_long, ct_phi_mod);
        
        double v = decrypt_val(state_long);
        if (v >= 1.0) {
            state_long = cc->EvalSub(state_long, ct_one);
            v = decrypt_val(state_long);
        }
        
        traj_long.push_back(v);
        
        if (v < 0.0 || v >= 1.0) errors_long++;
        
        if (i < 10 || i % 1000 == 0 || i == 9999) {
            std::cout << "  Step " << i << ": v=" << v 
                      << " level=" << state_long->GetLevel()
                      << (v >= 0 && v < 1 ? " ✓" : " ✗") << "\n";
        }
    }
    
    auto end = high_resolution_clock::now();
    auto dur = duration_cast<milliseconds>(end - start);
    
    // Check for repeats
    for (int i = 0; i < (int)traj_long.size(); i++) {
        for (int j = i + 1; j < (int)traj_long.size(); j++) {
            if (std::abs(traj_long[i] - traj_long[j]) < 0.001) {
                has_repeat = true;
                std::cout << "  REPEAT: step " << i << " = step " << j << "\n";
                break;
            }
        }
        if (has_repeat) break;
    }
    
    std::cout << "\n  Result: " << errors_long << " errors / 10,000 steps\n";
    std::cout << "  Time: " << dur.count() << "ms\n";
    std::cout << "  Repeat: " << (has_repeat ? "PERIODIC" : "PERIOD-0 (irrational!)") << "\n";
    std::cout << "  Level: " << state_long->GetLevel() << "\n";
    std::cout << "  Status: " << (errors_long == 0 && !has_repeat ? "🏆 PURE PERIOD-0!" : "❌ MAY ISSUE") << "\n";

    return 0;
}
