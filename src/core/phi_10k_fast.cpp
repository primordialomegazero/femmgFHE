// 10K FAST: Depth 1, Pre-encrypted, Combined
#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <chrono>

#include "pke/openfhe.h"

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

int main() {
    cout << "========================================\n";
    cout << "  φ-10K FAST — DEPTH 1 OPTIMIZED\n";
    cout << "========================================\n\n";
    
    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(1);    // ADDITION LANG!
    parameters.SetScalingModSize(20);        // MAS MALIIT
    parameters.SetBatchSize(8);              // 8 SHELLS
    parameters.SetSecurityLevel(HEStd_128_classic);
    
    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    
    auto keyPair = cc->KeyGen();
    
    const double PHI = 1.6180339887498948482;
    const double PHI_INV = 0.6180339887498948482;
    const double LN_PHI = log(PHI);
    
    cout << "  ✅ CKKS initialized (Depth 1, 128-bit)\n";
    cout << "  Pre-encrypting 8-shell operation...\n\n";
    
    // COMBINED 8-SHELL OP (isang vector)
    vector<double> combined(8, 0.0);
    combined[0] = 0.01 * pow(PHI_INV, 4);  // φ⁸ Security
    combined[1] = 0.01 * pow(PHI_INV, 3);  // φ⁶ Quantum
    combined[2] = 1.0 / 18.0;               // φ⁵ Fractal
    combined[3] = 0.5;                      // φ⁴ Entangle
    combined[4] = -1.0 / 3.0;               // φ³ Modulo
    combined[5] = PHI_INV;                  // φ² Time
    combined[6] = log(2.0) / LN_PHI;        // φ¹ Compute
    combined[7] = 1.0 / 500.0;              // Progress
    
    Plaintext pt_combined = cc->MakeCKKSPackedPlaintext(combined);
    auto ct_combined = cc->Encrypt(keyPair.publicKey, pt_combined);
    
    // Start sa 0
    vector<double> start(8, 0.0);
    Plaintext pt_start = cc->MakeCKKSPackedPlaintext(start);
    auto ct_result = cc->Encrypt(keyPair.publicKey, pt_start);
    
    cout << "  Starting 10,000 ops...\n\n";
    
    auto start_time = high_resolution_clock::now();
    
    for (int i = 1; i <= 10000; i++) {
        ct_result = cc->EvalAdd(ct_result, ct_combined);
        
        if (i % 500 == 0) {
            auto now = high_resolution_clock::now();
            auto elapsed = duration_cast<milliseconds>(now - start_time).count();
            cout << "  " << setw(7) << i << " | " << elapsed << " ms\n";
        }
    }
    
    auto end_time = high_resolution_clock::now();
    auto total = duration_cast<milliseconds>(end_time - start_time).count();
    
    cout << "\n  ✅ Complete!\n";
    cout << "  Total: " << total << " ms\n";
    cout << "  Level: " << ct_result->GetLevel() << "\n";
    cout << "  Per op: " << (double)total / 10000.0 << " ms\n\n";
    
    return 0;
}
