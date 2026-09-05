// ============================================
// φ-EXPONENT ONLY — Verification
// Ang exponent lang ang state
// Lahat ay derived mula sa φ
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include "pke/openfhe.h"

using namespace lbcrypto;
using namespace std;

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

    cout << "========================================\n";
    cout << "  φ-EXPONENT ONLY — Verification\n";
    cout << "========================================\n\n";
    cout << "  State: n lang (exponent)\n";
    cout << "  Lahat ay derived: φ^n, mode, mod φ\n\n";

    // ============================================
    // TEST 1: Derived values mula sa n
    // ============================================
    cout << "--- TEST 1: Derived values ---\n\n";
    cout << "  n | φ^n | mode | mod φ | φ^n mod 1\n";
    cout << "  --|-----|------|-------|----------\n";

    for (int n = 0; n <= 15; n++) {
        double phi_n = pow(PHI, n);
        string mode = (n % 2 == 0) ? "add" : "mul";
        double mod_phi = fmod(phi_n, PHI);
        double mod_1 = fmod(phi_n, 1.0);
        
        cout << "  " << setw(2) << n << " | "
             << setw(12) << fixed << setprecision(4) << phi_n << " | "
             << setw(3) << mode << " | "
             << setw(6) << mod_phi << " | "
             << setw(8) << mod_1 << "\n";
    }
    cout << "\n";

    // ============================================
    // TEST 2: FHE — i-verify na ang n lang ang state
    // ============================================
    cout << "--- TEST 2: FHE verification ---\n\n";
    
    auto ct_n = encrypt_n(3.0);  // Start sa n=3
    
    vector<double> delta_v(1, 1.0);
    Plaintext pt_delta = cc->MakeCKKSPackedPlaintext(delta_v);
    
    cout << "  Start: n=3\n";
    cout << "  Operations: 10 EvalAdd ng +1\n\n";
    
    for (int i = 0; i < 10; i++) {
        ct_n = cc->EvalAdd(ct_n, pt_delta);
        double n = decrypt_n(ct_n);
        
        // Lahat ay derived mula sa n
        double phi_n = pow(PHI, n);
        string mode = (((int)round(n)) % 2 == 0) ? "add" : "mul";
        double mod_phi = fmod(phi_n, PHI);
        
        cout << "  Step " << setw(2) << i << ": n=" << setw(5) << n
             << ", φ^n=" << setw(12) << phi_n
             << ", mode=" << mode
             << ", mod_φ=" << setw(8) << mod_phi << "\n";
    }
    cout << "\n";

    // ============================================
    // TEST 3: Ang operasyon ayon sa mode
    // ============================================
    cout << "--- TEST 3: Operasyon ayon sa mode ---\n\n";
    cout << "  Addition mode (even n): EvalAdd sa value = addition\n";
    cout << "  Multiplication mode (odd n): EvalAdd sa log = multiply\n\n";
    
    // Addition mode: n=2 (even), value = φ² = 2.618
    double n_add = 2.0;
    double val_add = pow(PHI, n_add);
    double result_add = val_add + 5.0;  // addition
    
    cout << "  Addition mode: φ² + 5 = " << val_add << " + 5 = " << result_add << "\n";
    
    // Multiplication mode: n=3 (odd), value = φ³ = 4.236
    double n_mul = 3.0;
    double val_mul = pow(PHI, n_mul);
    double result_mul = val_mul * 5.0;  // multiplication
    
    cout << "  Multiplication mode: φ³ × 5 = " << val_mul << " × 5 = " << result_mul << "\n\n";

    // ============================================
    // TEST 4: Ang φ^n bilang universal value
    // ============================================
    cout << "--- TEST 4: Universal value ---\n\n";
    cout << "  Ang φ^n ay may dual interpretation:\n";
    cout << "  - Normal: actual value\n";
    cout << "  - Log: exponent n\n\n";
    
    cout << "  n | φ^n (normal) | n (log) | Operation\n";
    cout << "  --|---------------|---------|-----------\n";
    
    for (int n = 0; n <= 8; n++) {
        double val = pow(PHI, n);
        string op;
        if (n % 2 == 0) {
            op = "addition: val + x = φ^n + x";
        } else {
            op = "multiply: val × x = φ^n × x";
        }
        
        cout << "  " << setw(1) << n << " | "
             << setw(14) << fixed << setprecision(4) << val << " | "
             << setw(7) << n << " | "
             << op << "\n";
    }
    
    cout << "\n  Level: " << ct_n->GetLevel() << "\n";

    return 0;
}
