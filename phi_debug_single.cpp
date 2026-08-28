// ============================================
// DEBUG: SINGLE SLOT CKKS BEHAVIOR
// Alamin kung bakit mali ang results
// ============================================

#include <iostream>
#include <vector>
#include <iomanip>
#include <complex>
#include <cmath>
#include "pke/openfhe.h"

using namespace lbcrypto;
using namespace std;

int main() {
    cout << "========================================\n";
    cout << "  DEBUG: SINGLE SLOT CKKS\n";
    cout << "========================================\n\n";
    
    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(30);
    parameters.SetScalingModSize(50);
    parameters.SetBatchSize(1);  // Single slot
    
    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    
    auto keyPair = cc->KeyGen();
    cc->EvalMultKeyGen(keyPair.secretKey);
    
    // Test: Encrypt(7), multiply by 11 via repeated addition
    vector<double> val(1, 7.0);
    Plaintext pt = cc->MakeCKKSPackedPlaintext(val);
    auto ct = cc->Encrypt(keyPair.publicKey, pt);
    
    cout << "  Encrypted 7\n\n";
    
    // Manual multiply by 11: 7×8 + 7×2 + 7×1 = 56 + 14 + 7 = 77
    auto ct2 = cc->EvalAdd(ct, ct);       // 14
    auto ct4 = cc->EvalAdd(ct2, ct2);     // 28
    auto ct8 = cc->EvalAdd(ct4, ct4);     // 56
    auto ct11 = cc->EvalAdd(ct8, ct2);    // 70
    ct11 = cc->EvalAdd(ct11, ct);          // 77
    
    // Decrypt
    Plaintext result_pt;
    cc->Decrypt(keyPair.secretKey, ct11, &result_pt);
    result_pt->SetLength(1);
    
    double result = result_pt->GetCKKSPackedValue()[0].real();
    
    cout << "  Expected: 77\n";
    cout << "  Got: " << result << "\n\n";
    
    // Check other values
    vector<double> test_vals = {2.0, 3.0, 5.0, 10.0, 15.0};
    
    for (double v : test_vals) {
        vector<double> val_vec(1, v);
        Plaintext p = cc->MakeCKKSPackedPlaintext(val_vec);
        auto c = cc->Encrypt(keyPair.publicKey, p);
        
        Plaintext rp;
        cc->Decrypt(keyPair.secretKey, c, &rp);
        rp->SetLength(1);
        
        double got = rp->GetCKKSPackedValue()[0].real();
        
        cout << "  Encrypt(" << v << ") → Decrypt = " << got 
             << " | " << (abs(got - v) < 0.01 ? "✅" : "❌") << "\n";
    }
    
    cout << "\n  DEBUG COMPLETE\n";
    return 0;
}
