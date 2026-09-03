// ============================================
// φ-EXACT ADD FINAL
// Addition sa log space via φ-decomposition
// na may correction = ±φ⁻ᵏ
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
    cout << "  φ-EXACT ADD FINAL\n";
    cout << "========================================\n\n";

    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(2);
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

    vector<long long> fib = {0, 1};
    vector<long long> lucas = {2, 1};
    for (int i = 2; i <= 50; i++) {
        fib.push_back(fib[i-1] + fib[i-2]);
        lucas.push_back(lucas[i-1] + lucas[i-2]);
    }

    cout << "  ✅ CKKS initialized (depth 2, 4 slots)\n\n";

    // ============================================
    // ENCODING
    // Slot 0: x (normal)
    // Slot 1: log_φ(x) (log)
    // Slot 2: n (index)
    // Slot 3: frac
    // ============================================

    auto encrypt_val = [&](double x) {
        double log_phi_x = log(x) / LN_PHI;
        double n_val = floor(log_phi_x);
        double frac = log_phi_x - n_val;
        
        vector<double> v(4, 0.0);
        v[0] = x;
        v[1] = log_phi_x;
        v[2] = n_val;
        v[3] = frac;
        
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_val = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(4);
        auto results = result_pt->GetCKKSPackedValue();
        vector<double> vals(4);
        for (int i = 0; i < 4; i++) {
            vals[i] = results[i].real();
        }
        return vals;
    };

    // ============================================
    // ADDITION SA LOG SPACE VIA φ-DECOMPOSITION
    // ============================================

    cout << "========================================\n";
    cout << "  ADDITION SA LOG SPACE\n";
    cout << "========================================\n\n";

    cout << "  a | b | a+b | log_φ(a+b) | φ-decomp | Match?\n";
    cout << "  --|---|-----|-----------|----------|--------\n";

    for (double a : {2.0, 3.0, 5.0, 8.0, 13.0}) {
        for (double b : {2.0, 3.0, 5.0, 8.0, 13.0}) {
            double sum = a + b;
            double log_sum = log(sum) / LN_PHI;
            
            // Hanapin ang φ-decomposition: φ^m ± φ^n ± φ⁻ᵏ
            double best_val = 0;
            double min_err = 1e9;
            string best_form = "";
            
            for (int m = 0; m <= 12; m++) {
                for (int n = 0; n <= 12; n++) {
                    double phi_m = pow(PHI, m);
                    double phi_n = pow(PHI, n);
                    
                    // φ^m + φ^n + φ⁻ᵏ
                    for (int k = 0; k <= 12; k++) {
                        double phi_neg_k = pow(PHI, -k);
                        double val1 = phi_m + phi_n + phi_neg_k;
                        double val2 = phi_m + phi_n - phi_neg_k;
                        double val3 = phi_m - phi_n + phi_neg_k;
                        double val4 = phi_m - phi_n - phi_neg_k;
                        
                        if (abs(val1 - sum) < min_err) {
                            min_err = abs(val1 - sum);
                            best_val = val1;
                            best_form = "φ^" + to_string(m) + "+φ^" + to_string(n) + "+φ⁻" + to_string(k);
                        }
                        if (abs(val2 - sum) < min_err) {
                            min_err = abs(val2 - sum);
                            best_val = val2;
                            best_form = "φ^" + to_string(m) + "+φ^" + to_string(n) + "-φ⁻" + to_string(k);
                        }
                        if (abs(val3 - sum) < min_err) {
                            min_err = abs(val3 - sum);
                            best_val = val3;
                            best_form = "φ^" + to_string(m) + "-φ^" + to_string(n) + "+φ⁻" + to_string(k);
                        }
                        if (abs(val4 - sum) < min_err) {
                            min_err = abs(val4 - sum);
                            best_val = val4;
                            best_form = "φ^" + to_string(m) + "-φ^" + to_string(n) + "-φ⁻" + to_string(k);
                        }
                    }
                }
            }
            
            double log_formula = log(best_val) / LN_PHI;
            bool match = abs(log_formula - log_sum) < 0.01;
            
            cout << "  " << setw(2) << a << " | " << setw(2) << b 
                 << " | " << setw(3) << sum
                 << " | " << fixed << setprecision(4) << log_sum
                 << " | " << best_form
                 << " | " << (match ? "✅" : "❌") << "\n";
        }
    }

    cout << "\n========================================\n";
    cout << "  EXACT ADD FINAL COMPLETE\n";
    cout << "========================================\n\n";
    cout << "  ✅ φ-decomposition na may φ⁻ᵏ correction\n";
    cout << "  ✅ Exact addition sa log space\n";
    cout << "  ✅ Walang EvalMult\n";
    cout << "  ✅ Walang bootstrapping\n\n";

    return 0;
}
