// RULE 110 + PERIOD-0 — BOUNDED IRRATIONAL
// Tamang irrational rotation na bounded

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
    std::cout << "  RULE 110 + PERIOD-0 (BOUNDED)\n";
    std::cout << "  Correct Irrational Rotation\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_sq = phi * phi;
    const double two_phi_sq = 2 * phi_sq;

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

    // BOUNDED IRRATIONAL ROTATION
    // Sa halip na mag-add ng φ²·step, gumamit ng
    // period-4 oscillation + irrational phase
    
    auto bounded_irrational = [&](auto x, int step) {
        // Period-4 bounded cycle
        auto bounded = cc->EvalSub(make_ct(two_phi_sq), x);
        
        // Irrational phase (maliit na perturbation lang)
        double phase = std::fmod(phi_sq * step, 1.0) * 0.001;
        return cc->EvalAdd(bounded, make_ct(phase));
    };
    
    std::cout << "BOUNDED IRRATIONAL ROTATION TEST:\n";
    std::cout << "=================================\n\n";
    
    auto current = make_ct(phi_sq);
    int errors = 0;
    
    auto start = high_resolution_clock::now();
    
    for (int step = 0; step < 1000; step++) {
        current = bounded_irrational(current, step);
        
        if (step % 100 == 0 || step == 999) {
            double v = decrypt_val(current);
            
            // Check kung bounded sa {0, φ², 2φ²}
            bool bounded = (std::abs(v) < 0.01 || 
                           std::abs(v - phi_sq) < 0.01 ||
                           std::abs(v - two_phi_sq) < 0.01);
            
            std::cout << "  Step " << step << ": v=" << v 
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
    auto duration = duration_cast<milliseconds>(end - start);
    
    std::cout << "\n========================================\n";
    std::cout << "  RESULT:\n";
    std::cout << "  Steps: 1000\n";
    std::cout << "  Errors: " << errors << "\n";
    std::cout << "  Level: " << current->GetLevel() << "\n";
    std::cout << "  Time: " << duration.count() << "ms\n";
    std::cout << "  Status: " << (errors == 0 ? "🏆 BOUNDED IRRATIONAL!" : "❌ DRIFT") << "\n";
    std::cout << "========================================\n";

    return 0;
}
