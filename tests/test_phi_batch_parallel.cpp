// BATCH PARALLEL — 256 COMPUTATIONS SABAY-SABAY
// Test kung lahat ng slots ay 0-level at stable

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
    std::cout << "  BATCH PARALLEL — 256 COMPUTATIONS\n";
    std::cout << "  Lahat ng slots sabay-sabay\n";
    std::cout << "========================================\n\n";

    const double phi_sq = 2.618033988749895;
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

    // Fill ALL 256 slots with different patterns
    std::vector<std::complex<double>> vec(slots, {0.0, 0.0});
    for (int i = 0; i < slots; i++) {
        vec[i] = {(i % 2 == 0) ? phi_sq : 0.0, 0.0};
    }
    
    auto current = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec));
    auto ct_two_phi_sq = cc->Encrypt(keys.publicKey, 
        cc->MakeCKKSPackedPlaintext(std::vector<std::complex<double>>(slots, {two_phi_sq, 0.0})));
    
    std::cout << "RUNNING 10,000 STEPS SA LAHAT NG 256 SLOTS...\n";
    std::cout << "===============================================\n\n";
    
    auto start = high_resolution_clock::now();
    
    for (int i = 0; i < 10000; i++) {
        current = cc->EvalSub(ct_two_phi_sq, current);
        
        if (i % 1000 == 0 || i == 9999) {
            Plaintext pt;
            cc->Decrypt(keys.secretKey, current, &pt);
            auto values = pt->GetCKKSPackedValue();
            
            // Check slots 0, 64, 128, 192
            std::cout << "  Step " << i << ": ";
            std::cout << "slot0=" << values[0].real() << " ";
            std::cout << "slot64=" << values[64].real() << " ";
            std::cout << "slot128=" << values[128].real() << " ";
            std::cout << "slot192=" << values[192].real() << " ";
            std::cout << "level=" << current->GetLevel() << "\n";
        }
    }
    
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<seconds>(end - start);
    
    std::cout << "\n========================================\n";
    std::cout << "  RESULT:\n";
    std::cout << "  Slots: 256\n";
    std::cout << "  Steps: 10,000\n";
    std::cout << "  Time: " << duration.count() << "s\n";
    std::cout << "  Level: " << current->GetLevel() << "\n";
    std::cout << "  Status: BATCH PARALLEL COMPLETE!\n";
    std::cout << "========================================\n";
    
    return 0;
}
