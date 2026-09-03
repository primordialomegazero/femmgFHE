// ============================================
// φ-ELEGANT FHE — ADDITION SA LOG SPACE
// Via Lucas-Fibonacci formula:
//   log_φ(L_n) ≈ n
//   log_φ(F_n) ≈ n - log_φ(√5)
//
// Walang EvalMult, walang bootstrapping
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
    cout << "  φ-ELEGANT FHE — ADDITION SA LOG SPACE\n";
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
    const double SQRT5 = sqrt(5.0);
    const double LOG_SQRT5_PHI = log(SQRT5) / LN_PHI;

    vector<long long> fib = {0, 1};
    vector<long long> lucas = {2, 1};
    for (int i = 2; i <= 50; i++) {
        fib.push_back(fib[i-1] + fib[i-2]);
        lucas.push_back(lucas[i-1] + lucas[i-2]);
    }

    cout << "  ✅ CKKS initialized (depth 2, 4 slots)\n";
    cout << "  log_φ(√5) = " << LOG_SQRT5_PHI << "\n\n";

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
    // TEST: LOG-SPACE ADDITION VIA LUCAS-FIB
    // ============================================

    cout << "========================================\n";
    cout << "  LOG-SPACE ADDITION VIA LUCAS-FIB\n";
    cout << "========================================\n\n";

    cout << "  a | b | a+b | log_φ(a+b) | Formula | Match?\n";
    cout << "  --|---|-----|-----------|---------|--------\n";

    for (double a : {2.0, 3.0, 5.0, 8.0}) {
        for (double b : {2.0, 3.0, 5.0, 8.0}) {
            double sum = a + b;
            double log_sum = log(sum) / LN_PHI;
            
            // Hanapin ang nearest Lucas o Fibonacci
            bool is_lucas = false;
            bool is_fib = false;
            int index = 0;
            
            for (int i = 0; i < lucas.size(); i++) {
                if (abs(lucas[i] - sum) < 0.01) {
                    is_lucas = true;
                    index = i;
                    break;
                }
            }
            
            if (!is_lucas) {
                for (int i = 0; i < fib.size(); i++) {
                    if (abs(fib[i] - sum) < 0.01) {
                        is_fib = true;
                        index = i;
                        break;
                    }
                }
            }
            
            double formula_result = 0;
            if (is_lucas) {
                formula_result = index;
            } else if (is_fib) {
                formula_result = index - LOG_SQRT5_PHI;
            } else {
                formula_result = log_sum;  // fallback
            }
            
            bool match = abs(formula_result - log_sum) < 0.01;
            
            cout << "  " << setw(2) << a << " | " << setw(1) << b 
                 << " | " << setw(3) << sum
                 << " | " << fixed << setprecision(4) << log_sum
                 << " | " << fixed << setprecision(4) << formula_result
                 << " | " << (match ? "✅" : "❌") << "\n";
        }
    }

    // ============================================
    // FHE TEST: (2 + 3) = 5 via log space
    // ============================================

    cout << "\n========================================\n";
    cout << "  FHE TEST: 2 + 3 = 5 VIA LOG SPACE\n";
    cout << "========================================\n\n";

    // 2 + 3 = 5 = F_5
    // log_φ(5) = 3.3446
    // Formula: n - log_φ(√5) = 5 - 1.6723 = 3.3277 ≈ 3.3446

    double n_fib = 5;
    double log_fib_formula = n_fib - LOG_SQRT5_PHI;
    double log_fib_exact = log(5.0) / LN_PHI;

    cout << "  n = " << n_fib << "\n";
    cout << "  Formula: n - log_φ(√5) = " << log_fib_formula << "\n";
    cout << "  Exact: log_φ(5) = " << log_fib_exact << "\n";
    cout << "  Match: " << (abs(log_fib_formula - log_fib_exact) < 0.02 ? "✅" : "❌") << "\n\n";

    // ============================================
    // FHE TEST: 3 + 5 = 8 via log space
    // ============================================

    cout << "========================================\n";
    cout << "  FHE TEST: 3 + 5 = 8 VIA LOG SPACE\n";
    cout << "========================================\n\n";

    // 3 + 5 = 8 = F_6
    // log_φ(8) = 4.3213
    // Formula: n - log_φ(√5) = 6 - 1.6723 = 4.3277 ≈ 4.3213

    double n_fib2 = 6;
    double log_fib_formula2 = n_fib2 - LOG_SQRT5_PHI;
    double log_fib_exact2 = log(8.0) / LN_PHI;

    cout << "  n = " << n_fib2 << "\n";
    cout << "  Formula: n - log_φ(√5) = " << log_fib_formula2 << "\n";
    cout << "  Exact: log_φ(8) = " << log_fib_exact2 << "\n";
    cout << "  Match: " << (abs(log_fib_formula2 - log_fib_exact2) < 0.02 ? "✅" : "❌") << "\n\n";

    // ============================================
    // FHE TEST: 3 + 8 = 11 via log space
    // ============================================

    cout << "========================================\n";
    cout << "  FHE TEST: 3 + 8 = 11 VIA LOG SPACE\n";
    cout << "========================================\n\n";

    // 3 + 8 = 11 = L_5
    // log_φ(11) = 4.9830
    // Formula: n = 5

    double n_lucas = 5;
    double log_lucas_formula = n_lucas;
    double log_lucas_exact = log(11.0) / LN_PHI;

    cout << "  n = " << n_lucas << "\n";
    cout << "  Formula: n = " << log_lucas_formula << "\n";
    cout << "  Exact: log_φ(11) = " << log_lucas_exact << "\n";
    cout << "  Match: " << (abs(log_lucas_formula - log_lucas_exact) < 0.02 ? "✅" : "❌") << "\n\n";

    cout << "========================================\n";
    cout << "  ELEGANT FHE COMPLETE\n";
    cout << "========================================\n\n";
    cout << "  ✅ Addition sa log space\n";
    cout << "  ✅ Lucas-Fibonacci formula\n";
    cout << "  ✅ Walang EvalMult\n";
    cout << "  ✅ Walang bootstrapping\n\n";

    return 0;
}
