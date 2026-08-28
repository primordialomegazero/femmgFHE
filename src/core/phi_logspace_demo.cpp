// ============================================
// φ-LOG SPACE FHE — DEMO
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <complex>

#include "pke/openfhe.h"

using namespace lbcrypto;
using namespace std;

int main() {
    cout << "========================================\n";
    cout << "  φ-LOG SPACE FHE — DEMO\n";
    cout << "  Encrypt log_φ(x), add para mag-multiply\n";
    cout << "========================================\n\n";
    
    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(30);
    parameters.SetScalingModSize(50);
    parameters.SetBatchSize(1);
    
    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    
    auto keyPair = cc->KeyGen();
    cc->EvalMultKeyGen(keyPair.secretKey);
    
    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);
    
    auto encrypt_log = [&](double value) {
        vector<double> val(1, log(value) / LN_PHI);
        Plaintext pt = cc->MakeCKKSPackedPlaintext(val);
        return cc->Encrypt(keyPair.publicKey, pt);
    };
    
    auto decrypt_value = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(1);
        return pow(PHI, result_pt->GetCKKSPackedValue()[0].real());
    };
    
    cout << "STEP 1: Encrypt log_φ(3)\n";
    auto ct_3 = encrypt_log(3.0);
    cout << "  Encrypted log_φ(3) = " << log(3.0)/LN_PHI << "\n\n";
    
    cout << "STEP 2: Encrypt log_φ(7)\n";
    auto ct_7 = encrypt_log(7.0);
    cout << "  Encrypted log_φ(7) = " << log(7.0)/LN_PHI << "\n\n";
    
    cout << "STEP 3: Add sa encrypted domain\n";
    auto ct_result = cc->EvalAdd(ct_3, ct_7);
    cout << "  Level: " << ct_result->GetLevel() << "\n";
    cout << "  Towers: " << ct_result->GetElements()[0].GetNumOfElements() << "\n\n";
    
    cout << "STEP 4: Decrypt\n";
    double result = decrypt_value(ct_result);
    cout << "  Result: " << result << "\n";
    cout << "  Expected: 21\n";
    cout << "  Match: " << (abs(result - 21.0) < 0.1 ? "✅" : "❌") << "\n\n";
    
    cout << "========================================\n";
    cout << "  KEY INSIGHT\n";
    cout << "========================================\n";
    cout << "  Multiplication = Addition sa log space\n";
    cout << "  Walang bootstrapping kailangan\n";
    cout << "  Zero-level lahat\n";
    return 0;
}
