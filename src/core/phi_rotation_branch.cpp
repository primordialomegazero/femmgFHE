// ============================================
// φ-ROTATION BRANCH — Natural Rotation Selection
// Comparison: floor(n/φ²) mod 2
// Selection: EvalRotate(rotation_index)
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
    parameters.SetBatchSize(8);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    auto keyPair = cc->KeyGen();
    cc->EvalRotateKeyGen(keyPair.secretKey, {1, 2, 3, 4, -1, -2, -3, -4});

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    auto encrypt_state = [&](double log_val, double branch_t, double branch_f) {
        vector<double> v(8, 0.0);
        v[0] = log_val;      // log value
        v[1] = branch_t;     // branch true
        v[2] = branch_f;     // branch false
        v[3] = log_val;      // backup
        v[4] = branch_t;     // backup
        v[5] = branch_f;     // backup
        v[6] = 0.0;
        v[7] = 0.0;
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_state = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt;
        cc->Decrypt(keyPair.secretKey, ct, &pt);
        pt->SetLength(8);
        auto res = pt->GetCKKSPackedValue();
        vector<double> out;
        for (int i = 0; i < 8; i++) out.push_back(res[i].real());
        return out;
    };

    cout << "========================================\n";
    cout << "  φ-ROTATION BRANCH — Natural Selection\n";
    cout << "========================================\n\n";
    cout << "  Comparison: floor(n/φ²) mod 2\n";
    cout << "  Selection: EvalRotate(rotation_index)\n\n";

    // ============================================
    // TEST: Rotation-based branching
    // ============================================
    cout << "  TEST: Rotation-based branching\n\n";

    // State: (log, branch_t, branch_f, backup...)
    // Ang rotation index ay mula sa floor(n/φ²)
    
    auto ct = encrypt_state(3.0, 4.0, 2.0);
    
    cout << "    Initial state:\n";
    auto v = decrypt_state(ct);
    cout << "    Slot 0 (log): " << v[0] << "\n";
    cout << "    Slot 1 (branch_t): " << v[1] << "\n";
    cout << "    Slot 2 (branch_f): " << v[2] << "\n\n";

    // Ang rotation index para sa branch selection:
    // floor(n/φ²) mod 2 = 0 → rotate(1) para sa branch_t
    // floor(n/φ²) mod 2 = 1 → rotate(2) para sa branch_f
    
    int n = 3;  // current log value
    int rot_idx = (int)floor(n / (PHI * PHI)) % 2;
    
    cout << "    n=" << n << ", rotation index=" << rot_idx << "\n";
    cout << "    Branch: " << (rot_idx == 0 ? "true (×φ)" : "false (÷φ)") << "\n\n";

    // I-rotate para ilipat ang tamang branch sa Slot 0
    auto ct_rot = cc->EvalRotate(ct, rot_idx + 1);
    auto v_rot = decrypt_state(ct_rot);
    
    cout << "    After rotate(" << rot_idx + 1 << "):\n";
    cout << "    Slot 0: " << v_rot[0] << "\n";
    cout << "    Level: " << ct_rot->GetLevel() << "\n";

    return 0;
}
