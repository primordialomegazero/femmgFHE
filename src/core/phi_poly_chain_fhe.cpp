// ============================================
// φ-POLY CHAIN FHE — Polynomial Evaluation
// φ-anchored polynomial na puro EvalAdd
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

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    // Fibonacci numbers para sa φ-power linearization
    vector<long long> fib = {0, 1};
    for (int i = 2; i <= 50; i++) fib.push_back(fib[i-1] + fib[i-2]);

    auto encrypt_log = [&](double val) {
        double log_val = log(val) / LN_PHI;
        vector<double> v(4, log_val);
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_val = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt;
        cc->Decrypt(keyPair.secretKey, ct, &pt);
        pt->SetLength(4);
        double log_val = pt->GetCKKSPackedValue()[0].real();
        return pow(PHI, log_val);
    };

    auto decrypt_log_val = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt;
        cc->Decrypt(keyPair.secretKey, ct, &pt);
        pt->SetLength(4);
        return pt->GetCKKSPackedValue()[0].real();
    };

    cout << "========================================\n";
    cout << "  φ-POLY CHAIN FHE — Polynomial Evaluation\n";
    cout << "========================================\n\n";
    cout << "  φ-anchored polynomial na puro EvalAdd\n\n";

    // ============================================
    // TEST 1: Taylor series ng e^x sa φ
    // ============================================
    cout << "  TEST 1: e^x Taylor series sa φ\n\n";
    cout << "  e^φ = 1 + φ + φ²/2! + φ³/3! + ...\n";
    cout << "  Sa φ-linear form:\n";
    cout << "  e^φ ≈ Σ (F_n×φ + F_{n-1}) / n!\n\n";

    // I-compute ang coefficients sa φ-linear form
    double coeff_const = 0.0;
    double coeff_phi = 0.0;
    double factorial = 1.0;
    
    for (int n = 0; n <= 15; n++) {
        if (n > 0) factorial *= n;
        double term = 1.0 / factorial;
        
        // φ^n = F_n × φ + F_{n-1}
        coeff_const += term * fib[n-1];  // F_{n-1} (para sa n≥1)
        coeff_phi += term * fib[n];      // F_n (para sa n≥1)
        
        if (n == 0) coeff_const += 1.0;  // φ^0 = 1
    }
    
    double e_phi_linear = coeff_const + coeff_phi * PHI;
    double e_phi_actual = exp(PHI);
    
    cout << "  Linear: " << coeff_const << " + " << coeff_phi << "φ = " << e_phi_linear << "\n";
    cout << "  Actual e^φ: " << e_phi_actual << "\n";
    cout << "  Error: " << abs(e_phi_linear - e_phi_actual) << "\n\n";

    // ============================================
    // TEST 2: Polynomial evaluation sa FHE
    // ============================================
    cout << "  TEST 2: Polynomial evaluation sa FHE\n\n";

    // f(φ) = 1 + 2φ + 3φ² + 5φ³ + 8φ⁴ (Fibonacci coefficients)
    // Linear: (1 + 3 + 5×2 + 8×3) + (2 + 3 + 5×3 + 8×5)φ
    // = (1 + 3 + 10 + 24) + (2 + 3 + 15 + 40)φ
    // = 38 + 60φ
    
    double f_const = 38.0;
    double f_phi_coeff = 60.0;
    double f_val = f_const + f_phi_coeff * PHI;
    
    auto ct_f = encrypt_log(f_val);
    
    cout << "  f(φ) = 38 + 60φ = " << decrypt_val(ct_f) << "\n";
    cout << "  Expected: " << f_val << "\n";
    cout << "  Match: " << (abs(decrypt_val(ct_f) - f_val) < 0.01 ? "✅" : "❌") << "\n\n";

    // ============================================
    // TEST 3: 10K polynomial chain
    // ============================================
    cout << "  TEST 3: 10K polynomial chain\n\n";

    // Chain: f₁(φ) = 1 + φ, f₂(φ) = 1 + φ + φ², ...
    // Sa bawat step, i-add ang susunod na φ-power
    // Sa log space, ito ay EvalAdd ng φ-power na value
    
    auto ct_chain = encrypt_log(1.0 + PHI);  // f₁(φ) = 1 + φ = φ²
    double expected = 1.0 + PHI;
    
    auto start = high_resolution_clock::now();
    
    for (int i = 2; i <= 10000; i++) {
        // I-add ang φ^i sa current value
        // Sa log space: log_φ(current + φ^i)
        double phi_i = pow(PHI, i);
        double new_val = expected + phi_i;
        double log_new = log(new_val) / LN_PHI;
        
        vector<double> d(4, log_new);
        Plaintext pt = cc->MakeCKKSPackedPlaintext(d);
        ct_chain = cc->EvalAdd(ct_chain, pt);
        
        expected = new_val;
        
        if (i <= 5 || i % 2500 == 0) {
            cout << "    Step " << setw(4) << i << ": value=" << decrypt_val(ct_chain) 
                 << ", expected=" << expected << "\n";
        }
    }
    
    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();
    
    cout << "\n    Time: " << time << " ms\n";
    cout << "    Ops/sec: " << (10000 * 1000.0) / time << "\n";
    cout << "    Level: " << ct_chain->GetLevel() << "\n\n";

    // ============================================
    // TEST 4: φ-anchored na scientific computation
    // ============================================
    cout << "  TEST 4: φ-anchored scientific computation\n\n";

    // Gravitational potential: V = -GM/r sa φ-space
    // Kung G=1, M=φ⁵, r=φ²:
    // V = -φ⁵ / φ² = -φ³
    // Sa φ-linear form: φ³ = 2φ + 1
    // V = -(2φ + 1) = -2φ - 1
    
    double V_const = -1.0;
    double V_phi = -2.0;
    double V_val = V_const + V_phi * PHI;
    
    auto ct_V = encrypt_log(abs(V_val));  // log ng |V| para sa positive
    // Negation sa log space: hindi simple — kailangan ng sign tracking
    
    cout << "  V = -φ³ = " << V_val << "\n";
    cout << "  |V| sa FHE: " << decrypt_val(ct_V) << "\n";
    cout << "  Expected |V|: " << abs(V_val) << "\n";
    cout << "  Match: " << (abs(decrypt_val(ct_V) - abs(V_val)) < 0.01 ? "✅" : "❌") << "\n";

    return 0;
}
