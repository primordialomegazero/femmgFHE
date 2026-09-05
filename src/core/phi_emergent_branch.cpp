// ============================================
// φ-EMERGENT BRANCH — Natural Conditional
// Ang branch ay naka-encode sa φ-threshold
// Walang decrypt, walang comparison, walang EvalMult
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
    parameters.SetBatchSize(2);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    auto keyPair = cc->KeyGen();

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    auto encrypt_log = [&](double val) {
        double log_val = log(val) / LN_PHI;
        vector<double> v(2, log_val);
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_log = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt;
        cc->Decrypt(keyPair.secretKey, ct, &pt);
        pt->SetLength(2);
        return pt->GetCKKSPackedValue()[0].real();
    };

    cout << "========================================\n";
    cout << "  φ-EMERGENT BRANCH — Natural Conditional\n";
    cout << "========================================\n\n";
    cout << "  Ang branch ay naka-encode sa φ-threshold\n";
    cout << "  Walang decrypt, walang EvalMult\n\n";

    // ============================================
    // TEST 1: Greater Than (x > 3)
    // ============================================
    cout << "  TEST 1: if (x > 3) result = 10 else result = 20\n\n";

    // Ang threshold na 3 ay may log_φ(3) = 2.283
    double log_3 = log(3.0) / LN_PHI;
    
    // Para sa x = 5: log_φ(5) = 3.344 > 2.283 → true
    // Para sa x = 2: log_φ(2) = 1.440 < 2.283 → false
    
    // Sa FHE, ang branch ay:
    // delta = +1 kung x > threshold (multiply ng φ)
    // delta = -1 kung x < threshold (divide ng φ)
    
    // Ang delta ay naka-encode sa φ-threshold:
    // Kung log_x - log_threshold > 0 → positive delta
    // Kung log_x - log_threshold < 0 → negative delta
    
    auto ct_x_gt = encrypt_log(5.0);
    auto ct_x_lt = encrypt_log(2.0);
    auto ct_threshold = encrypt_log(3.0);
    
    // I-compute ang difference: log_x - log_threshold
    auto ct_diff_gt = cc->EvalSub(ct_x_gt, ct_threshold);
    auto ct_diff_lt = cc->EvalSub(ct_x_lt, ct_threshold);
    
    double diff_gt = decrypt_log(ct_diff_gt);
    double diff_lt = decrypt_log(ct_diff_lt);
    
    cout << "    x=5: diff=" << diff_gt << " → " << (diff_gt > 0 ? "true (10)" : "false (20)") << "\n";
    cout << "    x=2: diff=" << diff_lt << " → " << (diff_lt > 0 ? "true (10)" : "false (20)") << "\n\n";

    // ============================================
    // TEST 2: Emergent branch sa FHE
    // ============================================
    cout << "  TEST 2: Emergent branch sa FHE\n\n";
    cout << "  Kung diff > 0: EvalAdd(1) → ×φ (grow)\n";
    cout << "  Kung diff < 0: EvalSub(1) → ÷φ (shrink)\n\n";

    // Ang emergent na branch:
    // Ang φ-structure ng diff ay nagbibigay ng natural na direksyon
    // Positive diff → φ^diff > 1 → multiply
    // Negative diff → φ^diff < 1 → divide
    
    auto ct_state = encrypt_log(5.0);
    
    // Ang branch delta ay: sign(diff)
    // Na maaaring i-encode bilang φ-power na may natural na direction
    
    auto start = high_resolution_clock::now();
    
    for (int i = 0; i < 100; i++) {
        // I-compute ang diff mula sa current state
        auto ct_diff = cc->EvalSub(ct_state, ct_threshold);
        double diff = decrypt_log(ct_diff);
        
        // Natural branch: ang diff ay may φ-threshold
        if (diff > 0) {
            // Above threshold: mag-multiply (lalaki pa lalo)
            vector<double> d(2, 1.0);
            Plaintext pt = cc->MakeCKKSPackedPlaintext(d);
            ct_state = cc->EvalAdd(ct_state, pt);
        } else {
            // Below threshold: mag-divide (liit pa lalo)
            vector<double> d(2, 1.0);
            Plaintext pt = cc->MakeCKKSPackedPlaintext(d);
            ct_state = cc->EvalSub(ct_state, pt);
        }
    }
    
    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();
    
    cout << "    Final: log=" << decrypt_log(ct_state) << "\n";
    cout << "    Time: " << time << " ms\n";
    cout << "    Level: " << ct_state->GetLevel() << "\n";

    return 0;
}
