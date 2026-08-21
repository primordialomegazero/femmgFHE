// 100,000 STEPS — MASSIVE SCALE TEST
// Period-4 cycle sa napakahabang computation

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
    std::cout << "  100,000 STEPS — MASSIVE SCALE\n";
    std::cout << "  Period-4 Cycle 0-Level\n";
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

    auto ct_two_phi_sq = make_ct(two_phi_sq);
    auto current = make_ct(phi_sq);
    int errors = 0;
    
    std::cout << "RUNNING 100,000 STEPS...\n";
    std::cout << "=========================\n\n";
    
    auto start = high_resolution_clock::now();
    
    for (int i = 0; i < 100000; i++) {
        current = cc->EvalSub(ct_two_phi_sq, current);
        
        if (i % 10000 == 0 || i == 99999) {
            double v = decrypt_val(current);
            bool valid = (std::abs(v - phi_sq) < 0.01 || 
                         std::abs(v - two_phi_sq) < 0.01 ||
                         std::abs(v + phi_sq) < 0.01 ||
                         std::abs(v) < 0.01);
            
            std::cout << "  Step " << i << ": v=" << v 
                      << " level=" << current->GetLevel();
            
            if (valid) std::cout << " ✓";
            else {
                std::cout << " ✗";
                errors++;
            }
            std::cout << "\n";
        }
    }
    
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<minutes>(end - start);
    
    std::cout << "\n========================================\n";
    std::cout << "  RESULT:\n";
    std::cout << "  Steps: 100,000\n";
    std::cout << "  Errors: " << errors << "\n";
    std::cout << "  Time: " << duration.count() << " minutes\n";
    std::cout << "  Level: " << current->GetLevel() << "\n";
    std::cout << "  Status: " << (errors == 0 ? "🏆 MASSIVE SCALE PERFECT!" : "❌ FAILED") << "\n";
    std::cout << "========================================\n";
    
    return 0;
}
