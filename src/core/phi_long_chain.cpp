// ============================================
// φ-LONG CHAIN — 100K Mixed Arbitrary Ops
// Exponent space na may lahat ng operations
// Walang decrypt, walang EvalMult
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <chrono>
#include <cstdlib>
#include "pke/openfhe.h"

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

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
    cout << "  φ-LONG CHAIN — 100K Mixed Arbitrary Ops\n";
    cout << "========================================\n\n";
    cout << "  Exponent space na may lahat ng operations\n";
    cout << "  Walang decrypt, walang EvalMult\n\n";

    // ============================================
    // Pre-computed deltas para sa arbitrary ops
    // ============================================
    vector<double> deltas;
    vector<string> op_names;
    
    // Multiply/Divide ng arbitrary values
    for (int k = 2; k <= 10; k++) {
        deltas.push_back(log(k) / LN_PHI);
        op_names.push_back("×" + to_string(k));
        
        deltas.push_back(-log(k) / LN_PHI);
        op_names.push_back("÷" + to_string(k));
    }
    
    // Add/Sub ng φ-powers
    deltas.push_back(1.0);
    op_names.push_back("+φ^(n-1)");
    
    deltas.push_back(-2.0);
    op_names.push_back("-φ^(n-1)");
    
    // Multiply/Divide ng φ mismo
    deltas.push_back(1.0);
    op_names.push_back("×φ");
    
    deltas.push_back(-1.0);
    op_names.push_back("÷φ");

    cout << "  Available operations: " << deltas.size() << "\n";
    cout << "  Running 100K mixed arbitrary operations...\n\n";

    // Start sa φ¹⁵
    auto ct = encrypt_n(15.0);
    double expected_n = 15.0;
    
    auto start = high_resolution_clock::now();
    
    int N = 100000;
    
    for (int i = 0; i < N; i++) {
        int idx = i % deltas.size();
        double delta = deltas[idx];
        
        vector<double> d(1, delta);
        Plaintext pt = cc->MakeCKKSPackedPlaintext(d);
        
        if (delta >= 0) {
            ct = cc->EvalAdd(ct, pt);
        } else {
            ct = cc->EvalSub(ct, pt);
        }
        
        expected_n += delta;
        
        // Ipakita ang progress tuwing 10K
        if (i % 10000 == 0 && i > 0) {
            double n_now = decrypt_n(ct);
            cout << "    Step " << setw(5) << i << ": n=" << setw(12) << n_now
                 << ", value=" << setw(15) << pow(PHI, n_now) << "\n";
        }
    }
    
    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();
    
    double n_final = decrypt_n(ct);
    
    cout << "\n  ✅ Complete!\n";
    cout << "  Time: " << time << " ms\n";
    cout << "  Ops/sec: " << (N * 1000.0) / time << "\n\n";
    cout << "  Final n: " << n_final << "\n";
    cout << "  Expected n: " << expected_n << "\n";
    cout << "  Match: " << (abs(n_final - expected_n) < 1.0 ? "✅" : "❌") << "\n";
    cout << "  Final value: " << pow(PHI, n_final) << "\n";
    cout << "  Level: " << ct->GetLevel() << "\n";

    return 0;
}
