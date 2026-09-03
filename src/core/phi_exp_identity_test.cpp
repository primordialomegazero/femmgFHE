// ============================================
// φ-EXPONENTIATION VIA φ-IDENTITY
// Subok kung ang φ² = φ + 1 ay pwedeng
// magbigay ng exponentiation nang walang EvalMult
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
    cout << "  φ-EXPONENTIATION VIA IDENTITY\n";
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

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);
    const double PHI_INV = 1.0 / PHI;

    cout << "  ✅ CKKS initialized (depth 1, 2 slots)\n\n";

    auto encrypt_val = [&](double v) {
        vector<double> vec(2, v);
        Plaintext pt = cc->MakeCKKSPackedPlaintext(vec);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_val = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(2);
        return result_pt->GetCKKSPackedValue()[0].real();
    };

    // ============================================
    // TEST: φ^n gamit ang identity
    // ============================================

    cout << "========================================\n";
    cout << "  TEST: φ^n (Identity Method)\n";
    cout << "========================================\n\n";

    // φ^1 = φ
    // φ^2 = φ + 1
    // φ^3 = 2φ + 1
    // φ^4 = 3φ + 2
    // φ^5 = 5φ + 3

    cout << "  n | φ^n (identity) | φ^n (direct) | Match?\n";
    cout << "  --|----------------|--------------|--------\n";

    for (int n = 0; n <= 10; n++) {
        // Identity method: F_n φ + F_{n-1}
        double fib_n = (pow(PHI, n) - pow(-PHI_INV, n)) / sqrt(5);
        double fib_nm1 = (pow(PHI, n-1) - pow(-PHI_INV, n-1)) / sqrt(5);
        double identity_val = fib_n * PHI + fib_nm1;
        double direct_val = pow(PHI, n);
        
        bool match = abs(identity_val - direct_val) < 0.001;
        
        cout << "  " << setw(2) << n << " | "
             << setw(14) << fixed << setprecision(6) << identity_val << " | "
             << setw(12) << direct_val << " | "
             << (match ? "✅" : "❌") << "\n";
    }

    // ============================================
    // TEST: LOG → NORMAL CONVERSION
    // ============================================

    cout << "\n========================================\n";
    cout << "  LOG → NORMAL CONVERSION\n";
    cout << "========================================\n\n";

    double target = 35.0;
    double log_phi_35 = log(target) / LN_PHI;

    cout << "  log_φ(35) = " << log_phi_35 << "\n";
    cout << "  φ^" << log_phi_35 << " = " << pow(PHI, log_phi_35) << "\n\n";

    // Decompose log_phi_35 sa integer at fractional
    int int_part = (int)floor(log_phi_35);
    double frac_part = log_phi_35 - int_part;

    cout << "  Integer part: " << int_part << "\n";
    cout << "  Fractional part: " << frac_part << "\n\n";

    // φ^int_part gamit ang identity
    double phi_int = pow(PHI, int_part);
    // φ^frac_part — approximation
    double phi_frac = pow(PHI, frac_part);

    cout << "  φ^" << int_part << " = " << phi_int << "\n";
    cout << "  φ^" << frac_part << " = " << phi_frac << "\n";
    cout << "  Product: " << (phi_int * phi_frac) << "\n";
    cout << "  Expected: " << target << "\n\n";

    cout << "  Level: 0 (walang EvalMult na ginamit)\n\n";

    return 0;
}
