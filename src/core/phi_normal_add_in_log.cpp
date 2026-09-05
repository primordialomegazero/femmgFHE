// ============================================
// φ-NORMAL ADD IN LOG — Maliit lang
// Addition sa normal space habang nasa log space
// Ang correction term: log_φ(1 + φ^(-n))
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

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    // Helper muna bago lambda
    auto log_phi = [&](double x) { return log(x) / LN_PHI; };

    auto encrypt_log = [&](double log_val) {
        vector<double> v(4, 0.0);
        v[0] = log_val;
        v[1] = pow(PHI, log_val);
        v[2] = log_phi(1.0 + pow(PHI, -log_val));
        v[3] = 0.0;
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_log = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt;
        cc->Decrypt(keyPair.secretKey, ct, &pt);
        pt->SetLength(4);
        auto res = pt->GetCKKSPackedValue();
        return vector<double>{res[0].real(), res[1].real(), res[2].real(), res[3].real()};
    };

    cout << "========================================\n";
    cout << "  φ-NORMAL ADD IN LOG\n";
    cout << "========================================\n\n";
    cout << "  State sa log space\n";
    cout << "  Gusto nating mag-add ng +1 sa normal\n";
    cout << "  Correction: log_φ(1 + φ^(-n))\n\n";

    // Test sa iba't ibang n values
    vector<double> n_values = {0.5, 1.0, 2.0, 5.0, 10.0};
    
    for (double n : n_values) {
        auto ct = encrypt_log(n);
        auto v = decrypt_log(ct);
        double correction = log_phi(1.0 + pow(PHI, -n));
        
        cout << "  n = " << n << "\n";
        cout << "    Log: " << v[0] << "\n";
        cout << "    Normal: " << v[1] << "\n";
        cout << "    Correction (+1 sa normal): " << correction << "\n";
        cout << "    New log kung +1 sa normal: " << log_phi(v[1] + 1.0) << "\n";
        cout << "    EvalAdd ng correction: " << v[0] + correction << "\n";
        cout << "    Match: " << (abs((v[0] + correction) - log_phi(v[1] + 1.0)) < 0.01 ? "✅" : "❌") << "\n\n";
    }

    // ============================================
    // TEST: EvalAdd ng correction sa log
    // ============================================
    cout << "--- TEST: EvalAdd ng correction ---\n\n";
    
    auto ct_state = encrypt_log(1.0);
    
    double corr_n1 = log_phi(1.0 + pow(PHI, -1.0));
    vector<double> corr_v(4, 0.0);
    corr_v[0] = corr_n1;
    Plaintext pt_corr = cc->MakeCKKSPackedPlaintext(corr_v);
    
    cout << "  Start: log=1, normal=φ\n";
    cout << "  Correction para sa +1: " << corr_n1 << "\n";
    cout << "  EvalAdd ng correction...\n\n";
    
    ct_state = cc->EvalAdd(ct_state, pt_corr);
    
    auto v_after = decrypt_log(ct_state);
    
    cout << "  After EvalAdd:\n";
    cout << "    Log: " << v_after[0] << "\n";
    cout << "    Normal (dapat φ+1=φ²): " << v_after[1] << "\n";
    cout << "    Expected log ng φ²: " << 2.0 << "\n";
    cout << "    Match: " << (abs(v_after[0] - 2.0) < 0.01 ? "✅" : "❌") << "\n\n";

    // ============================================
    // TEST: Paulit-ulit na EvalAdd ng correction
    // ============================================
    cout << "--- TEST: Paulit-ulit na EvalAdd ng correction ---\n\n";
    
    auto ct_loop = encrypt_log(1.0);
    
    for (int i = 0; i < 5; i++) {
        ct_loop = cc->EvalAdd(ct_loop, pt_corr);
        auto v_loop = decrypt_log(ct_loop);
        cout << "  Step " << i+1 << ": log=" << v_loop[0] 
             << ", normal=" << v_loop[1] << "\n";
    }
    cout << "\n";

    return 0;
}
