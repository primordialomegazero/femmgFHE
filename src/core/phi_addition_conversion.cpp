// ============================================
// φ-ADDITION CONVERSION
// I-convert ang log_φ(x) papuntang x gamit
// ang addition lang (Fibonacci + φ^frac constant)
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
    cout << "  φ-ADDITION CONVERSION\n";
    cout << "========================================\n\n";

    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(1);
    parameters.SetScalingModSize(50);
    parameters.SetBatchSize(16);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keyPair = cc->KeyGen();

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);
    const double SQRT5 = sqrt(5.0);
    const double INV_SQRT5 = 1.0 / SQRT5;
    const double PHI_FRAC_CONST = PHI - INV_SQRT5;

    // Fibonacci sequence
    vector<long long> fib = {0, 1};
    for (int i = 2; i <= 30; i++) {
        fib.push_back(fib[i-1] + fib[i-2]);
    }

    cout << "  ✅ CKKS initialized (depth 1, 16 slots)\n\n";

    // ============================================
    // ENCODING: LOG VALUE + FIBONACCI + CONST
    // ============================================

    auto encrypt_conversion = [&](double log_phi_x) {
        vector<double> v(16, 0.0);
        
        // Slot 0: log_φ(x)
        v[0] = log_phi_x;
        
        // Slot 1: φ^frac constant
        v[1] = PHI_FRAC_CONST;
        
        // Slots 2-15: Fibonacci numbers φ^n
        for (int i = 2; i < 16; i++) {
            int n = i - 2;
            v[i] = fib[n] * PHI + fib[n-1];
        }
        
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_conv = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(16);
        return result_pt->GetCKKSPackedValue()[0].real();
    };

    // ============================================
    // TEST: LOG → NORMAL CONVERSION VIA ADDITION
    // ============================================

    cout << "========================================\n";
    cout << "  LOG → NORMAL CONVERSION VIA ADDITION\n";
    cout << "========================================\n\n";

    cout << "  x | log_φ(x) | φ^frac (const) | φ^n (fib) | Sum | Match?\n";
    cout << "  --|-----------|-----------------|-----------|-----|--------\n";

    for (double x : {21.0, 34.0, 55.0, 89.0, 144.0}) {
        double log_phi_x = log(x) / LN_PHI;
        int n = (int)floor(log_phi_x);
        
        double phi_n = fib[n] * PHI + fib[n-1];
        double sum = phi_n + PHI_FRAC_CONST;
        bool match = abs(sum - x) < 1.0;
        
        cout << "  " << setw(4) << x << " | "
             << setw(9) << fixed << setprecision(4) << log_phi_x << " | "
             << setw(15) << PHI_FRAC_CONST << " | "
             << setw(9) << phi_n << " | "
             << setw(4) << sum << " | "
             << (match ? "✅" : "❌") << "\n";
    }

    cout << "\n========================================\n";
    cout << "  KEY INSIGHT\n";
    cout << "========================================\n\n";
    cout << "  Para sa malalaking x (Fibonacci numbers),\n";
    cout << "  ang conversion ay:\n";
    cout << "  x ≈ φ^n + φ^frac\n";
    cout << "  = (F_n φ + F_{n-1}) + (φ - 1/√5)\n";
    cout << "  LAHAT AY ADDITION LANG!\n\n";

    return 0;
}
