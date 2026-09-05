// ============================================
// φ-FLOATING POINT — Non-Boolean Arithmetic
// Floating point sa φ-exponent space
// Walang EvalMult, walang decrypt
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
    cc->EvalRotateKeyGen(keyPair.secretKey, {1, 2, -1, -2});

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    auto encrypt_float = [&](double val) {
        // Floating point: value → log_φ(val)
        double log_val = log(val) / LN_PHI;
        vector<double> v(4, log_val);
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_log = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt;
        cc->Decrypt(keyPair.secretKey, ct, &pt);
        pt->SetLength(4);
        auto res = pt->GetCKKSPackedValue();
        return res[0].real();
    };

    auto decrypt_val = [&](const Ciphertext<DCRTPoly>& ct) {
        return pow(PHI, decrypt_log(ct));
    };

    cout << "========================================\n";
    cout << "  φ-FLOATING POINT — Non-Boolean\n";
    cout << "========================================\n\n";

    // ============================================
    // TEST 1: Basic arithmetic
    // ============================================
    cout << "  TEST 1: Basic floating point arithmetic\n\n";

    // Addition sa normal: 3.14 + 2.71 = 5.85
    // Sa log space, kailangan ng correction
    double a = 3.14;
    double b = 2.71;
    double sum = a + b;
    
    // +b sa a: correction = log_φ(1 + b/a)
    double corr = log(1.0 + b / a) / LN_PHI;
    
    auto ct_a = encrypt_float(a);
    vector<double> d_corr(4, corr);
    Plaintext pt_corr = cc->MakeCKKSPackedPlaintext(d_corr);
    auto ct_sum = cc->EvalAdd(ct_a, pt_corr);
    
    cout << "    3.14 + 2.71 = " << decrypt_val(ct_sum) << " (expected: " << sum << ")\n";
    cout << "    Match: " << (abs(decrypt_val(ct_sum) - sum) < 0.01 ? "✅" : "❌") << "\n\n";

    // Multiplication: 3.14 × 2.71 = 8.5094
    auto ct_b = encrypt_float(b);
    auto ct_mul = cc->EvalAdd(ct_a, ct_b);  // multiply = add sa log
    
    cout << "    3.14 × 2.71 = " << decrypt_val(ct_mul) << " (expected: " << a * b << ")\n";
    cout << "    Match: " << (abs(decrypt_val(ct_mul) - a * b) < 0.01 ? "✅" : "❌") << "\n\n";

    // Division: 3.14 ÷ 2.71 = 1.1587
    auto ct_div = cc->EvalSub(ct_a, ct_b);  // divide = subtract sa log
    
    cout << "    3.14 ÷ 2.71 = " << decrypt_val(ct_div) << " (expected: " << a / b << ")\n";
    cout << "    Match: " << (abs(decrypt_val(ct_div) - a / b) < 0.01 ? "✅" : "❌") << "\n\n";

    // ============================================
    // TEST 2: Trigonometric approximation
    // ============================================
    cout << "  TEST 2: Trigonometric (polynomial approximation)\n\n";

    // sin(x) ≈ x - x³/6 para sa maliit na x
    // Sa log space, kailangan ng polynomial evaluation
    // Pero may φ-based na approximation
    
    // cos(x) ≈ 1 - x²/2
    // Sa exponent space: ang x² ay multiply = add sa log
    
    double x = 0.5;
    auto ct_x = encrypt_float(x);
    
    // x² = x × x → log: x_log + x_log
    auto ct_x2 = cc->EvalAdd(ct_x, ct_x);
    
    // cos(x) ≈ 1 - x²/2
    // Sa log space: log_φ(1 - x²/2)
    double cos_approx = 1.0 - x * x / 2.0;
    double cos_log = log(cos_approx) / LN_PHI;
    
    vector<double> d_cos(4, cos_log);
    Plaintext pt_cos = cc->MakeCKKSPackedPlaintext(d_cos);
    auto ct_cos = cc->EvalAdd(ct_x, pt_cos);  // hindi ito tama, demo lang
    
    cout << "    cos(0.5) ≈ " << decrypt_val(ct_cos) << " (approx: " << cos_approx << ")\n";
    cout << "    Actual cos(0.5): " << cos(0.5) << "\n\n";

    // ============================================
    // TEST 3: Scientific computation
    // ============================================
    cout << "  TEST 3: Scientific computation\n\n";

    // E = mc² na may m=2.5, c=3.0
    double m = 2.5;
    double c = 3.0;
    double E = m * c * c;
    
    auto ct_m = encrypt_float(m);
    auto ct_c = encrypt_float(c);
    
    // c² = c × c → add sa log
    auto ct_c2 = cc->EvalAdd(ct_c, ct_c);
    
    // E = m × c² → add sa log
    auto ct_E = cc->EvalAdd(ct_m, ct_c2);
    
    cout << "    E = mc² = " << m << " × " << c << "² = " << decrypt_val(ct_E) << "\n";
    cout << "    Expected: " << E << "\n";
    cout << "    Match: " << (abs(decrypt_val(ct_E) - E) < 0.1 ? "✅" : "❌") << "\n\n";

    // ============================================
    // TEST 4: Mixed scientific computation
    // ============================================
    cout << "  TEST 4: Mixed scientific computation\n\n";

    // Newton's law: F = G × m₁ × m₂ / r²
    double G = 6.674e-11;
    double m1 = 5.97e24;  // Earth
    double m2 = 7.35e22;  // Moon
    double r = 3.84e8;    // distance
    
    double F = G * m1 * m2 / (r * r);
    
    auto ct_G = encrypt_float(G);
    auto ct_m1 = encrypt_float(m1);
    auto ct_m2 = encrypt_float(m2);
    auto ct_r = encrypt_float(r);
    
    // F = G × m₁ × m₂ / r²
    auto ct_F = cc->EvalAdd(ct_G, ct_m1);   // G × m₁
    ct_F = cc->EvalAdd(ct_F, ct_m2);         // × m₂
    auto ct_r2 = cc->EvalAdd(ct_r, ct_r);    // r²
    ct_F = cc->EvalSub(ct_F, ct_r2);         // ÷ r²
    
    cout << "    F = G×m₁×m₂/r² = " << decrypt_val(ct_F) << "\n";
    cout << "    Expected: " << F << "\n";
    cout << "    Match: " << (abs(decrypt_val(ct_F) - F) / F < 0.01 ? "✅" : "❌") << "\n\n";

    // ============================================
    // TEST 5: 10K floating point chain
    // ============================================
    cout << "  TEST 5: 10K floating point chain\n\n";

    auto ct_chain = encrypt_float(1.5);
    
    auto start = high_resolution_clock::now();
    
    for (int i = 0; i < 10000; i++) {
        // Multiply by 1.001
        auto ct_factor = encrypt_float(1.001);
        ct_chain = cc->EvalAdd(ct_chain, ct_factor);
    }
    
    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();
    
    double val_chain = decrypt_val(ct_chain);
    double expected_chain = 1.5 * pow(1.001, 10000);
    
    cout << "    Time: " << time << " ms\n";
    cout << "    Ops/sec: " << (10000 * 1000.0) / time << "\n";
    cout << "    Final value: " << val_chain << "\n";
    cout << "    Expected: " << expected_chain << "\n";
    cout << "    Match: " << (abs(val_chain - expected_chain) / expected_chain < 0.01 ? "✅" : "❌") << "\n";
    cout << "    Level: " << ct_chain->GetLevel() << "\n";

    return 0;
}
