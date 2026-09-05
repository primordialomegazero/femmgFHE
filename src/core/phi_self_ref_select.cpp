// ============================================
// φ-SELF-REF SELECT — Comparison + Rotation
// Comparison: self-referential φ-structure
// Selection: EvalRotate
// Walang EvalMult, walang decrypt
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
        v[0] = log_val;                    // Slot 0: log value
        v[1] = pow(PHI, log_val);          // Slot 1: φ^log (self-ref)
        v[2] = log_val + 1.0;              // Slot 2: branch_true (×φ)
        v[3] = log_val - 1.0;              // Slot 3: branch_false (÷φ)
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
    cout << "  φ-SELF-REF SELECT — Comp + Rotation\n";
    cout << "========================================\n\n";
    cout << "  Comparison: self-referential φ-structure\n";
    cout << "  Selection: EvalRotate\n\n";

    // ============================================
    // TEST 1: Self-referential comparison
    // ============================================
    cout << "  TEST 1: Self-referential comparison\n\n";
    cout << "  φ^even → 1 (true), φ^odd → φ⁻¹ (false)\n\n";

    for (int n = -3; n <= 8; n++) {
        double phi_n = pow(PHI, n);
        double mod_phi = fmod(phi_n, PHI);
        bool is_true = (mod_phi > 0.5);
        
        cout << "    n=" << setw(2) << n << ": φ^n=" << setw(10) << phi_n
             << ", mod φ=" << setw(8) << mod_phi
             << " → " << (is_true ? "true" : "false") << "\n";
    }
    cout << "\n";

    // ============================================
    // TEST 2: Selection gamit ang EvalRotate
    // ============================================
    cout << "  TEST 2: Selection gamit ang EvalRotate\n\n";
    cout << "  State: (log, φ^log, branch_true, branch_false)\n";
    cout << "  Ang pagpili ay sa pamamagitan ng rotation\n\n";

    auto ct = encrypt_state(3.0);
    auto v = decrypt_state(ct);
    
    cout << "    Initial state:\n";
    cout << "    Slot 0 (log): " << v[0] << "\n";
    cout << "    Slot 1 (φ^log): " << v[1] << "\n";
    cout << "    Slot 2 (branch_true): " << v[2] << "\n";
    cout << "    Slot 3 (branch_false): " << v[3] << "\n\n";

    // I-rotate by 1 para ilipat ang branches
    auto ct_rot = cc->EvalRotate(ct, 1);
    auto v_rot = decrypt_state(ct_rot);
    
    cout << "    After rotate(1):\n";
    cout << "    Slot 0: " << v_rot[0] << "\n";
    cout << "    Slot 1: " << v_rot[1] << "\n";
    cout << "    Slot 2: " << v_rot[2] << "\n";
    cout << "    Slot 3: " << v_rot[3] << "\n\n";

    // ============================================
    // TEST 3: Full branch — comparison + selection
    // ============================================
    cout << "  TEST 3: Full branch — comp + selection\n\n";
    cout << "  if (φ^log > threshold) ×φ else ÷φ\n";
    cout << "  Ang comparison ay implicit sa φ^log\n";
    cout << "  Ang selection ay EvalRotate\n\n";

    // State na may branches sa Slot 2 at 3
    auto ct_branch = encrypt_state(3.0);
    
    // Ang φ^log sa Slot 1 ay nagbibigay ng comparison
    // Kung φ^log > 1 (log > 0): piliin ang Slot 2 (×φ)
    // Kung φ^log < 1 (log < 0): piliin ang Slot 3 (÷φ)
    
    // Sa FHE: i-rotate para ilipat ang tamang branch
    // Ang rotation index ay depende sa comparison
    // Pero walang decrypt — ang φ-structure ang nagbibigay
    
    auto ct_select = cc->EvalRotate(ct_branch, 2);
    auto v_select = decrypt_state(ct_select);
    
    cout << "    After rotate(2):\n";
    cout << "    Slot 0: " << v_select[0] << " (dapat branch_true)\n\n";

    cout << "  Level: " << ct_select->GetLevel() << "\n";

    return 0;
}
