// ============================================
// φ-SLOT ROTATION TEST
// Subok kung ang EvalRotate ay pwedeng gamitin
// para ilipat ang Slot 1 papuntang Slot 0
// nang walang EvalMult at Level 0 pa rin
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
    cout << "  φ-SLOT ROTATION TEST\n";
    cout << "========================================\n\n";

    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(1);
    parameters.SetScalingModSize(50);
    parameters.SetBatchSize(2);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keyPair = cc->KeyGen();
    
    // Generate rotation keys
    cc->EvalAtIndexKeyGen(keyPair.secretKey, {1, -1});

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    cout << "  ✅ CKKS initialized (depth 1, 2 slots)\n";
    cout << "  Rotation keys generated\n\n";

    auto encrypt_dual = [&](double value) {
        vector<double> v(2, 0.0);
        v[0] = value;                        // Normal space
        v[1] = log(value) / LN_PHI;          // Log space (φ-base)
        
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_dual = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(2);
        auto results = result_pt->GetCKKSPackedValue();
        
        double normal = results[0].real();
        double log_val = results[1].real();
        double log_result = pow(PHI, log_val);
        
        return make_pair(normal, log_result);
    };

    // ============================================
    // TEST: SLOT ROTATION
    // ============================================

    cout << "========================================\n";
    cout << "  TEST: SLOT ROTATION\n";
    cout << "========================================\n\n";

    // Encrypt 35 sa dual space
    auto ct_35 = encrypt_dual(35.0);
    
    cout << "  Original:\n";
    auto [orig_normal, orig_log] = decrypt_dual(ct_35);
    cout << "  Slot 0 (Normal): " << orig_normal << "\n";
    cout << "  Slot 1 (Log): " << orig_log << "\n\n";

    // Rotate by -1: Slot 1 → Slot 0
    auto ct_rotated = cc->EvalRotate(ct_35, -1);
    
    cout << "  After EvalRotate(-1):\n";
    auto [rot_normal, rot_log] = decrypt_dual(ct_rotated);
    cout << "  Slot 0 (Normal): " << rot_normal << "\n";
    cout << "  Slot 1 (Log): " << rot_log << "\n\n";

    cout << "  Level: " << ct_rotated->GetLevel() << "\n";
    cout << "  Towers: " << ct_rotated->GetElements()[0].GetNumOfElements() << "\n\n";

    // ============================================
    // TEST: (5 × 7) + 3 gamit ang Rotation
    // ============================================

    cout << "========================================\n";
    cout << "  TEST: (5 × 7) + 3 WITH ROTATION\n";
    cout << "========================================\n\n";

    auto ct_5 = encrypt_dual(5.0);
    auto ct_7 = encrypt_dual(7.0);
    auto ct_3 = encrypt_dual(3.0);

    // Step 1: 5 × 7 sa log space
    auto ct_mult = cc->EvalAdd(ct_5, ct_7);
    
    cout << "  After 5 × 7:\n";
    auto [m_normal, m_log] = decrypt_dual(ct_mult);
    cout << "  Slot 0: " << m_normal << "\n";
    cout << "  Slot 1 (log): " << m_log << "\n\n";

    // Step 2: Rotate para ilipat ang Slot 1 papuntang Slot 0
    auto ct_rot = cc->EvalRotate(ct_mult, -1);
    
    cout << "  After rotate:\n";
    auto [r_normal, r_log] = decrypt_dual(ct_rot);
    cout << "  Slot 0: " << r_normal << "\n";
    cout << "  Slot 1: " << r_log << "\n\n";

    // Step 3: Add 3
    auto ct_final = cc->EvalAdd(ct_rot, ct_3);
    
    cout << "  After + 3:\n";
    auto [f_normal, f_log] = decrypt_dual(ct_final);
    cout << "  Slot 0: " << f_normal << " (expected: 38)\n";
    cout << "  Slot 1: " << f_log << "\n\n";

    cout << "  Level: " << ct_final->GetLevel() << "\n";
    cout << "  Towers: " << ct_final->GetElements()[0].GetNumOfElements() << "\n\n";

    return 0;
}
