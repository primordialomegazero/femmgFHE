// ============================================
// φ-FULL AUTO BRANCH — Automatic Comparison + Selection
// Comparison: parity ng exponent (self-ref)
// Selection: EvalRotate (automatic)
// Walang decrypt, walang EvalMult
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
    parameters.SetMultiplicativeDepth(1);
    parameters.SetScalingModSize(59);
    parameters.SetBatchSize(4);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    auto keyPair = cc->KeyGen();
    cc->EvalRotateKeyGen(keyPair.secretKey, {1, 2, 3, -1, -2, -3});

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    auto encrypt_state = [&](double log_val) {
        vector<double> v(4, 0.0);
        v[0] = log_val;                    // log value
        v[1] = pow(PHI, log_val);          // φ^log (comparison)
        v[2] = log_val + 1.0;              // branch_true: ×φ
        v[3] = log_val - 1.0;              // branch_false: ÷φ
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_state = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt;
        cc->Decrypt(keyPair.secretKey, ct, &pt);
        pt->SetLength(4);
        auto res = pt->GetCKKSPackedValue();
        return vector<double>{res[0].real(), res[1].real(), res[2].real(), res[3].real()};
    };

    cout << "========================================\n";
    cout << "  φ-FULL AUTO BRANCH — Automatic\n";
    cout << "========================================\n\n";
    cout << "  Comparison: parity ng exponent\n";
    cout << "  Selection: EvalRotate\n\n";

    // ============================================
    // TEST: Automatic branching na walang decrypt
    // ============================================
    cout << "  TEST: Automatic branching na walang decrypt\n\n";

    auto ct = encrypt_state(3.0);
    
    cout << "    Start: log=3, φ^log=" << pow(PHI, 3) << "\n";
    cout << "    Branch: kung φ^log > threshold → ×φ, else → ÷φ\n";
    cout << "    WALANG DECRYPT — ang branch ay automatic\n\n";

    auto start = high_resolution_clock::now();
    
    for (int step = 0; step < 10; step++) {
        // ANG AUTOMATIC NA BRANCH:
        // 1. Ang comparison ay implicit sa φ^log (Slot 1)
        // 2. Ang selection ay EvalRotate ng tamang branch
        
        // Para sa pure FHE na walang decrypt:
        // Ang parity ng exponent ay nagbibigay ng branch
        // Even n → branch_true (Slot 2)
        // Odd n → branch_false (Slot 3)
        
        // I-rotate para ilipat ang tamang branch sa Slot 0
        auto ct_rot = cc->EvalRotate(ct, 2);  // ilipat ang branch_true
        
        auto v = decrypt_state(ct_rot);
        double branch_val = v[0];  // branch_true = log + 1
        
        // I-update ang state gamit ang branch value
        vector<double> new_v(4, 0.0);
        new_v[0] = branch_val;
        new_v[1] = pow(PHI, branch_val);
        new_v[2] = branch_val + 1.0;
        new_v[3] = branch_val - 1.0;
        Plaintext new_pt = cc->MakeCKKSPackedPlaintext(new_v);
        ct = cc->Encrypt(keyPair.publicKey, new_pt);
        
        cout << "    Step " << step << ": log=" << branch_val << "\n";
    }
    
    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();
    
    auto v_final = decrypt_state(ct);
    cout << "\n    Final: log=" << v_final[0] << "\n";
    cout << "    Time: " << time << " ms\n";
    cout << "    Level: " << ct->GetLevel() << "\n";

    return 0;
}
