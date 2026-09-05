// ============================================
// φ-CKKS PARAMS — Natural na φ-based
// Scaling: φ^85 ≈ 2^59
// Ring dim: φ^20 ≈ 2^14
// Test ang φ-based na parameters
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
    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    cout << "========================================\n";
    cout << "  φ-CKKS PARAMS — Natural na φ-based\n";
    cout << "========================================\n\n";
    cout << "  φ^85 = " << pow(PHI, 85) << " ≈ 2^59 = " << pow(2.0, 59) << "\n";
    cout << "  φ^20 = " << pow(PHI, 20) << " ≈ 2^14 = " << pow(2.0, 14) << "\n\n";

    // ============================================
    // Test 1: Standard CKKS (2-based)
    // ============================================
    cout << "  TEST 1: Standard CKKS (2-based)\n\n";

    CCParams<CryptoContextCKKSRNS> std_params;
    std_params.SetMultiplicativeDepth(1);
    std_params.SetScalingModSize(59);
    std_params.SetBatchSize(8);
    std_params.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc_std = GenCryptoContext(std_params);
    cc_std->Enable(PKE);
    cc_std->Enable(KEYSWITCH);
    cc_std->Enable(LEVELEDSHE);
    auto keyPair_std = cc_std->KeyGen();

    // I-encrypt ang φ-based na values
    vector<double> vals(8, 0.0);
    for (int i = 0; i < 8; i++) {
        vals[i] = pow(PHI, i);  // φ⁰, φ¹, ..., φ⁷
    }
    
    Plaintext pt_std = cc_std->MakeCKKSPackedPlaintext(vals);
    auto ct_std = cc_std->Encrypt(keyPair_std.publicKey, pt_std);

    // Decrypt at check
    Plaintext pt_out_std;
    cc_std->Decrypt(keyPair_std.secretKey, ct_std, &pt_out_std);
    pt_out_std->SetLength(8);
    auto res_std = pt_out_std->GetCKKSPackedValue();

    cout << "  Values after decrypt:\n  ";
    for (int i = 0; i < 8; i++) {
        cout << setw(8) << res_std[i].real();
    }
    cout << "\n\n";
    cout << "  Ring dim: " << cc_std->GetRingDimension() << "\n";
    cout << "  Level: " << ct_std->GetLevel() << "\n\n";

    // ============================================
    // Test 2: φ-based na scaling (try iba't ibang values)
    // ============================================
    cout << "  TEST 2: φ-based na scaling exploration\n\n";

    // Subukan ang iba't ibang scaling na malapit sa φ-powers
    vector<int> scaling_sizes = {50, 55, 59, 60, 62};
    
    for (int s : scaling_sizes) {
        cout << "  Scaling mod size " << s << ":\n";
        
        try {
            CCParams<CryptoContextCKKSRNS> test_params;
            test_params.SetMultiplicativeDepth(1);
            test_params.SetScalingModSize(s);
            test_params.SetBatchSize(8);
            test_params.SetSecurityLevel(HEStd_128_classic);
            
            CryptoContext<DCRTPoly> cc_test = GenCryptoContext(test_params);
            cc_test->Enable(PKE);
            cc_test->Enable(KEYSWITCH);
            cc_test->Enable(LEVELEDSHE);
            auto kp_test = cc_test->KeyGen();
            
            Plaintext pt_test = cc_test->MakeCKKSPackedPlaintext(vals);
            auto ct_test = cc_test->Encrypt(kp_test.publicKey, pt_test);
            
            Plaintext pt_out_test;
            cc_test->Decrypt(kp_test.secretKey, ct_test, &pt_out_test);
            pt_out_test->SetLength(8);
            auto res_test = pt_out_test->GetCKKSPackedValue();
            
            // Compute error
            double max_err = 0;
            for (int i = 0; i < 8; i++) {
                max_err = max(max_err, abs(res_test[i].real() - vals[i]));
            }
            
            cout << "    ✅ Max error: " << max_err << "\n";
            cout << "    Ring dim: " << cc_test->GetRingDimension() << "\n";
            
        } catch (const exception& e) {
            cout << "    ❌ Failed: " << e.what() << "\n";
        }
    }
    cout << "\n";

    // ============================================
    // Test 3: φ-power na values sa standard CKKS
    // ============================================
    cout << "  TEST 3: φ-power values sa standard CKKS\n\n";

    // Ang φ-powers ay may natural na exponential growth
    // Sa CKKS, ito ay maaaring magdulot ng overflow
    // kung masyadong malaki ang exponents
    
    vector<double> phi_powers(8, 0.0);
    for (int i = 0; i < 8; i++) {
        phi_powers[i] = pow(PHI, i + 10);  // φ¹⁰ hanggang φ¹⁷
    }
    
    Plaintext pt_phi_powers = cc_std->MakeCKKSPackedPlaintext(phi_powers);
    auto ct_phi_powers = cc_std->Encrypt(keyPair_std.publicKey, pt_phi_powers);
    
    Plaintext pt_out_phi;
    cc_std->Decrypt(keyPair_std.secretKey, ct_phi_powers, &pt_out_phi);
    pt_out_phi->SetLength(8);
    auto res_phi = pt_out_phi->GetCKKSPackedValue();
    
    cout << "  φ-power values after decrypt:\n  ";
    for (int i = 0; i < 8; i++) {
        cout << setw(12) << res_phi[i].real();
    }
    cout << "\n\n";
    
    // Compute relative error
    double max_rel_err = 0;
    for (int i = 0; i < 8; i++) {
        double rel_err = abs(res_phi[i].real() - phi_powers[i]) / phi_powers[i];
        max_rel_err = max(max_rel_err, rel_err);
    }
    cout << "  Max relative error: " << max_rel_err << "\n";
    cout << "  Level: " << ct_phi_powers->GetLevel() << "\n";

    return 0;
}
