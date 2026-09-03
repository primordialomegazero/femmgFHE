// ============================================
// φ-ENCODING SYNC TEST
// Subok kung ang encoding mismo ay pwedeng
// magbigay ng automatic sync sa pagitan
// ng normal at log space
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
    cout << "  φ-ENCODING SYNC TEST\n";
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

    // ============================================
    // ENCODING VARIATIONS
    // ============================================

    auto encrypt_v1 = [&](double value) {
        vector<double> v(4, 0.0);
        v[0] = value;                        // Normal
        v[1] = log(value) / LN_PHI;          // Log φ
        v[2] = value * PHI;                  // Normal × φ
        v[3] = log(value);                   // Natural log
        
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto encrypt_v2 = [&](double value) {
        vector<double> v(4, 0.0);
        v[0] = value;                        // Normal
        v[1] = value;                        // Same value (hindi log)
        v[2] = value;                        // Same value
        v[3] = value;                        // Same value
        
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_4 = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(4);
        auto results = result_pt->GetCKKSPackedValue();
        return vector<double>{results[0].real(), results[1].real(), 
                              results[2].real(), results[3].real()};
    };

    // ============================================
    // TEST: (5 × 7) + 3
    // ============================================

    cout << "========================================\n";
    cout << "  TEST: (5 × 7) + 3\n";
    cout << "========================================\n\n";

    // V1: Log encoding
    auto c5_v1 = encrypt_v1(5.0);
    auto c7_v1 = encrypt_v1(7.0);
    auto c3_v1 = encrypt_v1(3.0);
    
    auto mult_v1 = cc->EvalAdd(c5_v1, c7_v1);
    auto r1 = decrypt_4(mult_v1);
    
    cout << "  V1 After 5 × 7:\n";
    cout << "  Slot 0 (Normal): " << r1[0] << "\n";
    cout << "  Slot 1 (Logφ): " << r1[1] << " → " << pow(PHI, r1[1]) << "\n";
    cout << "  Slot 2 (Normal×φ): " << r1[2] << "\n";
    cout << "  Slot 3 (Loge): " << r1[3] << " → " << exp(r1[3]) << "\n\n";

    auto add_v1 = cc->EvalAdd(mult_v1, c3_v1);
    auto r2 = decrypt_4(add_v1);
    
    cout << "  V1 After + 3:\n";
    cout << "  Slot 0 (Normal): " << r2[0] << "\n";
    cout << "  Slot 1 (Logφ): " << r2[1] << " → " << pow(PHI, r2[1]) << "\n";
    cout << "  Slot 2 (Normal×φ): " << r2[2] << "\n";
    cout << "  Slot 3 (Loge): " << r2[3] << " → " << exp(r2[3]) << "\n\n";

    // V2: Same value encoding
    auto c5_v2 = encrypt_v2(5.0);
    auto c7_v2 = encrypt_v2(7.0);
    auto c3_v2 = encrypt_v2(3.0);
    
    auto mult_v2 = cc->EvalAdd(c5_v2, c7_v2);
    auto r3 = decrypt_4(mult_v2);
    
    cout << "  V2 After 5 × 7:\n";
    cout << "  Slot 0: " << r3[0] << "\n";
    cout << "  Slot 1: " << r3[1] << "\n";
    cout << "  Slot 2: " << r3[2] << "\n";
    cout << "  Slot 3: " << r3[3] << "\n\n";

    auto add_v2 = cc->EvalAdd(mult_v2, c3_v2);
    auto r4 = decrypt_4(add_v2);
    
    cout << "  V2 After + 3:\n";
    cout << "  Slot 0: " << r4[0] << "\n";
    cout << "  Slot 1: " << r4[1] << "\n";
    cout << "  Slot 2: " << r4[2] << "\n";
    cout << "  Slot 3: " << r4[3] << "\n\n";

    // ============================================
    // ANALYSIS
    // ============================================

    cout << "========================================\n";
    cout << "  ANALYSIS\n";
    cout << "========================================\n\n";
    cout << "  V1: Log encoding — Slot 1 ay log\n";
    cout << "      Kailangan ng exponentiation para ma-convert\n\n";
    cout << "  V2: Same value — Slot 0-3 ay pare-pareho\n";
    cout << "      Walang multiplication sa log space\n\n";
    cout << "  Walang automatic sync sa encoding pa lang.\n";
    cout << "  Kailangan ng operasyon para sa conversion.\n\n";

    cout << "  Level: " << add_v1->GetLevel() << "\n";
    cout << "  Towers: " << add_v1->GetElements()[0].GetNumOfElements() << "\n\n";

    return 0;
}
