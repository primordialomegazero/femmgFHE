// ============================================
// φ-CORRECTION FHE
// I-encode ang correction term sa slots
// para sa exact mixed operations
//
// Author: Dan Fernandez / Primordial Omega Zero
// ============================================

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
    cout << "  φ-CORRECTION FHE\n";
    cout << "========================================\n\n";

    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(1);
    parameters.SetScalingModSize(50);
    parameters.SetBatchSize(4);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keyPair = cc->KeyGen();

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    cout << "  ✅ CKKS initialized (depth 1, 4 slots)\n\n";

    // Fibonacci sequence
    vector<long long> fib = {0, 1};
    for (int i = 2; i <= 20; i++) {
        fib.push_back(fib[i-1] + fib[i-2]);
    }

    // ============================================
    // ENCODING: [Normal, Log×φ, Correction, F_n]
    // ============================================

    auto encrypt_corr = [&](double x) {
        vector<double> v(4, 0.0);
        v[0] = x;                                       // Normal
        v[1] = (log(x) / LN_PHI) * PHI;                 // Log×φ
        v[2] = x - (log(x) / LN_PHI) * PHI;             // Correction
        v[3] = fib[(int)floor(log(x) / LN_PHI)];        // F_n reference
        
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_corr = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(4);
        auto results = result_pt->GetCKKSPackedValue();
        return vector<double>{results[0].real(), results[1].real(),
                              results[2].real(), results[3].real()};
    };

    // ============================================
    // TEST: (5 × 7) + 3 = 38
    // ============================================

    cout << "========================================\n";
    cout << "  TEST: (5 × 7) + 3 = 38\n";
    cout << "========================================\n\n";

    auto ct_5 = encrypt_corr(5.0);
    auto ct_7 = encrypt_corr(7.0);
    auto ct_3 = encrypt_corr(3.0);

    // Step 1: 5 × 7
    auto ct_mult = cc->EvalAdd(ct_5, ct_7);
    auto mult_vals = decrypt_corr(ct_mult);
    
    cout << "  After 5 × 7:\n";
    cout << "  Slot 0 (Normal): " << mult_vals[0] << "\n";
    cout << "  Slot 1 (Log×φ): " << mult_vals[1] << "\n";
    cout << "  Slot 2 (Correction): " << mult_vals[2] << "\n";
    cout << "  Slot 3 (F_n): " << mult_vals[3] << "\n\n";

    // Step 2: + 3
    auto ct_final = cc->EvalAdd(ct_mult, ct_3);
    auto final_vals = decrypt_corr(ct_final);
    
    cout << "  After + 3:\n";
    cout << "  Slot 0 (Normal): " << final_vals[0] << "\n";
    cout << "  Slot 1 (Log×φ): " << final_vals[1] << "\n";
    cout << "  Slot 2 (Correction): " << final_vals[2] << "\n";
    cout << "  Slot 3 (F_n): " << final_vals[3] << "\n\n";

    cout << "  Level: " << ct_final->GetLevel() << "\n";
    cout << "  Towers: " << ct_final->GetElements()[0].GetNumOfElements() << "\n\n";

    return 0;
}
