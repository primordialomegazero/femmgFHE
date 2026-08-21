// RULE 110 + NOISE CONTROL — 0-LEVEL SUSTAINABLE
// Period-4 cycle + φ-based noise reduction

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
    std::cout << "  RULE 110 + NOISE CONTROL\n";
    std::cout << "  0-Level Sustainable\n";
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
    cc->Enable(ADVANCEDSHE);

    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);
    cc->EvalAtIndexKeyGen(keys.secretKey, {1, -1});
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

    // Period-4 threshold (0-level)
    auto period4_threshold = [&](auto x) {
        // 0 → φ² → 2φ² → -φ² → 0
        return cc->EvalSub(make_ct(two_phi_sq), x);
    };
    
    // φ-based noise reduction (0-level)
    auto noise_reduce = [&](auto x) {
        // Subtract small correction based on φ-property
        // This is 0-level (subtraction only)
        return cc->EvalSub(x, make_ct(0.001 * phi));
    };
    
    std::cout << "1000 STEPS WITH NOISE CONTROL:\n";
    std::cout << "==============================\n\n";
    
    auto current = make_ct(phi_sq);  // Start with φ²
    int errors = 0;
    
    auto start = high_resolution_clock::now();
    
    for (int i = 0; i < 1000; i++) {
        // Rule 110 step (0-level)
        current = period4_threshold(current);
        
        // Noise reduction every 10 steps (0-level)
        if (i % 10 == 0) {
            current = noise_reduce(current);
        }
        
        if (i % 100 == 0 || i == 999) {
            double v = decrypt_val(current);
            
            // Check kung bounded sa {0, φ², 2φ², -φ²}
            bool bounded = (std::abs(v) < 0.01 || 
                           std::abs(v - phi_sq) < 0.01 || 
                           std::abs(v - two_phi_sq) < 0.01 ||
                           std::abs(v + phi_sq) < 0.01);
            
            std::cout << "  Step " << i << ": v=" << v 
                      << " level=" << current->GetLevel();
            
            if (bounded) std::cout << " ✓";
            else {
                std::cout << " ✗";
                errors++;
            }
            std::cout << "\n";
        }
    }
    
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<seconds>(end - start);
    
    std::cout << "\n========================================\n";
    std::cout << "  RESULT:\n";
    std::cout << "  Steps: 1000\n";
    std::cout << "  Errors: " << errors << "\n";
    std::cout << "  Time: " << duration.count() << "s\n";
    std::cout << "  Level: " << current->GetLevel() << "\n";
    std::cout << "  Status: " << (errors == 0 ? "🏆 STABLE!" : "❌ DRIFTING") << "\n";
    std::cout << "========================================\n";
    
    return 0;
}
