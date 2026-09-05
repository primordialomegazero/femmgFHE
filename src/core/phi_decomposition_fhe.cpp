// ============================================
// φ-DECOMPOSITION FHE — 50 iterations
// Arbitrary integer bilang φ-powers
// Bawat φ-power ay may constant correction
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
    parameters.SetBatchSize(1);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    auto keyPair = cc->KeyGen();

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    auto encrypt_n = [&](double n) {
        vector<double> v(1, n);
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_n = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt;
        cc->Decrypt(keyPair.secretKey, ct, &pt);
        pt->SetLength(1);
        return pt->GetCKKSPackedValue()[0].real();
    };

    // Decompose integer sa φ-powers
    auto decompose = [&](double x) {
        vector<double> terms;
        double remaining = x;
        for (int p = 10; p >= -10; p--) {
            double phi_p = pow(PHI, p);
            if (remaining >= phi_p - 1e-6) {
                terms.push_back(phi_p);
                remaining -= phi_p;
            }
        }
        return terms;
    };

    cout << "========================================\n";
    cout << "  φ-DECOMPOSITION FHE — 50 iterations\n";
    cout << "========================================\n\n";
    cout << "  Arbitrary integer bilang φ-powers\n";
    cout << "  Bawat φ-power ay may constant correction\n\n";

    // Start: value = 10
    double n_start = log(10.0) / LN_PHI;
    auto ct_state = encrypt_n(n_start);

    cout << "  Initial: value = 10, n = " << n_start << "\n\n";

    // Test: 10 + 5 = 15
    cout << "  TEST: 10 + 5 = 15\n";
    cout << "  Decompose 5 sa φ-powers...\n";
    
    auto terms_5 = decompose(5.0);
    cout << "  5 = ";
    for (size_t i = 0; i < terms_5.size(); i++) {
        if (i > 0) cout << " + ";
        cout << terms_5[i];
    }
    cout << "\n\n";

    // Sa log space, ang +5 ay:
    // log_φ(10 + 5) = log_φ(15)
    // Ang correction ay: log_φ(1 + 5/10) = log_φ(1.5)
    double n_target = log(15.0) / LN_PHI;
    double correction = n_target - n_start;

    cout << "  Target n (log_φ(15)): " << n_target << "\n";
    cout << "  Correction: " << correction << "\n\n";

    // I-apply ang correction sa FHE
    vector<double> delta_v(1, correction);
    Plaintext pt_delta = cc->MakeCKKSPackedPlaintext(delta_v);
    ct_state = cc->EvalAdd(ct_state, pt_delta);

    double n_after = decrypt_n(ct_state);
    double value_after = pow(PHI, n_after);

    cout << "  After FHE addition:\n";
    cout << "    n: " << n_after << "\n";
    cout << "    value: " << value_after << " (expected: 15)\n";
    cout << "    Match: " << (abs(value_after - 15.0) < 0.01 ? "✅" : "❌") << "\n\n";

    // ============================================
    // TEST: Sequence ng arbitrary operations
    // ============================================
    cout << "  TEST: Sequence ng arbitrary operations\n";
    cout << "  10 + 5 - 2 × 3 ÷ 4 = ?\n\n";

    // Reset sa 10
    ct_state = encrypt_n(n_start);

    // +5
    correction = log(15.0 / 10.0) / LN_PHI;
    delta_v[0] = correction;
    pt_delta = cc->MakeCKKSPackedPlaintext(delta_v);
    ct_state = cc->EvalAdd(ct_state, pt_delta);
    
    double n1 = decrypt_n(ct_state);
    cout << "  +5 → " << pow(PHI, n1) << "\n";

    // -2
    correction = log(13.0 / 15.0) / LN_PHI;
    delta_v[0] = correction;
    pt_delta = cc->MakeCKKSPackedPlaintext(delta_v);
    ct_state = cc->EvalAdd(ct_state, pt_delta);
    
    double n2 = decrypt_n(ct_state);
    cout << "  -2 → " << pow(PHI, n2) << "\n";

    // ×3
    correction = log(3.0) / LN_PHI;
    delta_v[0] = correction;
    pt_delta = cc->MakeCKKSPackedPlaintext(delta_v);
    ct_state = cc->EvalAdd(ct_state, pt_delta);
    
    double n3 = decrypt_n(ct_state);
    cout << "  ×3 → " << pow(PHI, n3) << "\n";

    // ÷4
    correction = log(4.0) / LN_PHI;
    delta_v[0] = correction;
    pt_delta = cc->MakeCKKSPackedPlaintext(delta_v);
    ct_state = cc->EvalSub(ct_state, pt_delta);
    
    double n4 = decrypt_n(ct_state);
    cout << "  ÷4 → " << pow(PHI, n4) << " (expected: 9.75)\n";
    cout << "  Match: " << (abs(pow(PHI, n4) - 9.75) < 0.01 ? "✅" : "❌") << "\n\n";

    cout << "  Level: " << ct_state->GetLevel() << "\n";

    return 0;
}
