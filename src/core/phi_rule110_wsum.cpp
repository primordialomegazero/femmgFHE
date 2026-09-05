// ============================================
// φ-RULE 110 WSUM — EvalLinearWSum
// Weighted sum ng neighbors sa isang operasyon
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
    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(2);
    parameters.SetScalingModSize(59);
    parameters.SetBatchSize(8);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    cc->Enable(ADVANCEDSHE);
    auto keyPair = cc->KeyGen();
    cc->EvalMultKeyGen(keyPair.secretKey);
    cc->EvalRotateKeyGen(keyPair.secretKey, {1, -1});

    cout << "========================================\n";
    cout << "  φ-RULE 110 WSUM — EvalLinearWSum\n";
    cout << "========================================\n\n";

    // Initial: 11010101 → parity ±1
    vector<double> init = {1.0, 1.0, -1.0, 1.0, -1.0, 1.0, -1.0, 1.0};
    Plaintext pt_init = cc->MakeCKKSPackedPlaintext(init);
    auto ct_state = cc->Encrypt(keyPair.publicKey, pt_init);

    // I-rotate para sa left at right
    auto ct_left = cc->EvalRotate(ct_state, -1);
    auto ct_right = cc->EvalRotate(ct_state, 1);

    // Weighted sum gamit ang vector<double> na weights
    vector<ReadOnlyCiphertext<DCRTPoly>> cts;
    cts.push_back(ct_left);
    cts.push_back(ct_state);
    cts.push_back(ct_right);
    
    vector<double> weights = {2.618, 1.618, 1.0};  // φ², φ, 1

    try {
        auto ct_wsum = cc->EvalLinearWSum(cts, weights);
        
        // Decrypt at decode
        Plaintext pt_out;
        cc->Decrypt(keyPair.secretKey, ct_wsum, &pt_out);
        pt_out->SetLength(8);
        auto res = pt_out->GetCKKSPackedValue();
        
        cout << "  Weighted sum values:\n  ";
        for (int i = 0; i < 8; i++) {
            cout << setw(10) << res[i].real();
        }
        cout << "\n\n";
        cout << "  Level: " << ct_wsum->GetLevel() << "\n";
        
    } catch (const exception& e) {
        cout << "  EvalLinearWSum failed: " << e.what() << "\n";
    }

    return 0;
}
