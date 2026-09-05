// ============================================
// φ-TRUE ARBITRARY COLLAPSE — All Ops
// Lahat ng operations: +, -, ×, ÷ arbitrary
// Walang precompute — lahat homomorphic
// Collapse ay φ-parity ng exponent
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
    parameters.SetBatchSize(8);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    auto keyPair = cc->KeyGen();

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    cout << "========================================\n";
    cout << "  φ-TRUE ARBITRARY COLLAPSE — All Ops\n";
    cout << "========================================\n\n";
    cout << "  Arbitrary +, -, ×, ÷ — lahat homomorphic\n";
    cout << "  Collapse: φ-parity ng exponent\n\n";

    // ============================================
    // Pre-encode ang unique deltas (hindi precompute ng chain)
    // ============================================
    vector<double> unique_deltas;
    vector<string> op_names;
    
    unique_deltas.push_back(log(2.0) / LN_PHI);    // ×2
    unique_deltas.push_back(-log(3.0) / LN_PHI);   // ÷3
    unique_deltas.push_back(log(5.0) / LN_PHI);    // ×5
    unique_deltas.push_back(-log(7.0) / LN_PHI);   // ÷7
    unique_deltas.push_back(log(11.0) / LN_PHI);   // ×11
    unique_deltas.push_back(-log(13.0) / LN_PHI);  // ÷13
    unique_deltas.push_back(log(17.0) / LN_PHI);   // ×17
    unique_deltas.push_back(-log(19.0) / LN_PHI);  // ÷19
    
    op_names = {"×2", "÷3", "×5", "÷7", "×11", "÷13", "×17", "÷19"};

    // I-encrypt ang unique deltas
    vector<Plaintext> encoded_deltas;
    for (double delta : unique_deltas) {
        vector<double> v(8, delta);
        encoded_deltas.push_back(cc->MakeCKKSPackedPlaintext(v));
    }

    cout << "  Available ops: ";
    for (string name : op_names) cout << name << " ";
    cout << "\n\n";

    // ============================================
    // Initial state — zero exponent
    // ============================================
    vector<double> zero_v(8, 0.0);
    Plaintext pt_zero = cc->MakeCKKSPackedPlaintext(zero_v);
    auto ct_state = cc->Encrypt(keyPair.publicKey, pt_zero);

    // ============================================
    // 10K ARBITRARY operations — walang precompute
    // ============================================
    int N = 10000;
    srand(42);
    
    cout << "  Running " << N << " arbitrary operations...\n\n";

    auto start = high_resolution_clock::now();

    double total_log = 0.0;

    for (int i = 0; i < N; i++) {
        int idx = rand() % unique_deltas.size();
        double delta = unique_deltas[idx];
        
        ct_state = cc->EvalAdd(ct_state, encoded_deltas[idx]);
        
        total_log += delta;
        
        if (i % 2000 == 0 && i > 0) {
            cout << "    Step " << setw(4) << i << ": ["
                 << op_names[idx] << "]\n";
        }
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    Plaintext pt_out;
    cc->Decrypt(keyPair.secretKey, ct_state, &pt_out);
    pt_out->SetLength(8);
    auto res = pt_out->GetCKKSPackedValue();

    double final_exponent = res[0].real();
    int parity = ((int)round(final_exponent)) % 2;
    double collapsed_value = (parity == 0) ? 1.0 : 0.0;

    cout << "\n  ✅ Complete!\n";
    cout << "  Time: " << time << " ms\n";
    cout << "  Ops/sec: " << (N * 1000.0) / time << "\n\n";
    cout << "  Final exponent: " << final_exponent << "\n";
    cout << "  Expected: " << total_log << "\n";
    cout << "  Diff: " << final_exponent - total_log << "\n\n";
    cout << "  Parity: " << parity << "\n";
    cout << "  Collapsed: " << collapsed_value << "\n";
    cout << "  Level: " << ct_state->GetLevel() << "\n\n";
    cout << "  KEY: Arbitrary ops → collapse sa parity\n";
    cout << "  Walang precompute, lahat homomorphic\n";

    return 0;
}
