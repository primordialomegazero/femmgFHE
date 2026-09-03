// ============================================
// φ-FIBONACCI MARKER FHE
// Fibonacci marker para sa operation detection
// Slot 3: marker (F_1 para sa +, F_3 para sa ×)
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
    cout << "  φ-FIBONACCI MARKER FHE\n";
    cout << "========================================\n\n";

    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(2);
    parameters.SetScalingModSize(50);
    parameters.SetBatchSize(4);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keyPair = cc->KeyGen();

    const double PHI = 1.6180339887498948482;
    const double PHI_INV = 1.0 / PHI;
    const double LN_PHI = log(PHI);

    vector<long long> fib = {0, 1};
    for (int i = 2; i <= 50; i++) {
        fib.push_back(fib[i-1] + fib[i-2]);
    }

    cout << "  ✅ CKKS initialized (depth 2, 4 slots)\n\n";

    // ============================================
    // ENCODING NA MAY FIBONACCI MARKER
    // Slot 0: x×φ — addition value
    // Slot 1: x×φ⁻¹ — multiplication value
    // Slot 2: log_φ(x) — log space
    // Slot 3: Fibonacci marker (F_1=1 para sa +, F_3=2 para sa ×)
    // ============================================

    auto encrypt_marker = [&](double x, bool is_addition) {
        double log_phi_x = log(x) / LN_PHI;
        double marker = is_addition ? fib[1] : fib[3];  // F_1=1 o F_3=2
        
        vector<double> v(4, 0.0);
        v[0] = x * PHI;
        v[1] = x * PHI_INV;
        v[2] = log_phi_x;
        v[3] = marker;
        
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_marker = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(4);
        auto results = result_pt->GetCKKSPackedValue();
        vector<double> vals(4);
        for (int i = 0; i < 4; i++) vals[i] = results[i].real();
        return vals;
    };

    // ============================================
    // TEST: 5 + 7 = 12 (parehong addition)
    // ============================================

    cout << "========================================\n";
    cout << "  TEST: 5 + 7 = 12 (parehong addition)\n";
    cout << "========================================\n\n";

    auto ct_5_add = encrypt_marker(5.0, true);
    auto ct_7_add = encrypt_marker(7.0, true);

    auto ct_sum = cc->EvalAdd(ct_5_add, ct_7_add);
    auto sum_vals = decrypt_marker(ct_sum);

    cout << "  Slot 0 (xφ): " << sum_vals[0] << " (expected: 12φ = " << (12*PHI) << ")\n";
    cout << "  Slot 3 (marker): " << sum_vals[3] << " (expected: F_1+F_1 = 2 = F_3)\n\n";

    // ============================================
    // TEST: 5 × 7 = 35 (parehong multiplication)
    // ============================================

    cout << "========================================\n";
    cout << "  TEST: 5 × 7 = 35 (parehong multiplication)\n";
    cout << "========================================\n\n";

    auto ct_5_mul = encrypt_marker(5.0, false);
    auto ct_7_mul = encrypt_marker(7.0, false);

    auto ct_prod = cc->EvalAdd(ct_5_mul, ct_7_mul);
    auto prod_vals = decrypt_marker(ct_prod);

    cout << "  Slot 2 (log): " << prod_vals[2] << " → φ^ = " << pow(PHI, prod_vals[2]) << "\n";
    cout << "  Slot 3 (marker): " << prod_vals[3] << " (expected: F_3+F_3 = 4 = F_5?)\n\n";

    // ============================================
    // TEST: 5 + 7 mixed detection
    // ============================================

    cout << "========================================\n";
    cout << "  TEST: MIXED DETECTION (add + mult)\n";
    cout << "========================================\n\n";

    auto ct_mixed = cc->EvalAdd(ct_5_add, ct_7_mul);
    auto mixed_vals = decrypt_marker(ct_mixed);

    cout << "  Slot 3 (marker): " << mixed_vals[3] << " (expected: F_1+F_3 = 3 = F_4 → bridge)\n\n";

    cout << "========================================\n";
    cout << "  DETECTION LOGIC\n";
    cout << "========================================\n\n";
    cout << "  Marker = 2 (F_3) → parehong addition\n";
    cout << "  Marker = 4 (F_5) → parehong multiplication\n";
    cout << "  Marker = 3 (F_4) → mixed → kailangan ng bridge\n\n";

    return 0;
}
