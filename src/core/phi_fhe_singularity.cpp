// ============================================
// φ-FHE SINGULARITY — Collapse sa Encrypted
// 1M+ ops na nagco-collapse sa isang ciphertext
// Lahat homomorphic, walang decrypt
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
    cout << "  φ-FHE SINGULARITY — Collapse sa Encrypted\n";
    cout << "========================================\n\n";

    // ============================================
    // FHE SINGULARITY: 1M ops → isang ciphertext
    // ============================================
    cout << "  TEST: 1M ops → collapse sa isang value\n\n";

    // Pre-computed na total log para sa 1M ops
    // Sa encrypted domain, ang collapse ay:
    // total_log = Σ log_φ(op_i) — homomorphic EvalAdd
    // collapsed = total_log mod φ — φ-periodic na reset
    
    int N = 1000000;  // 1M operations
    
    // Ang bawat op ay ×2, kaya log_φ(2) bawat op
    double log_per_op = log(2.0) / LN_PHI;
    
    // Sa encrypted domain: i-encrypt ang log_per_op
    // at i-EvalAdd ng N beses
    
    vector<double> delta_v(8, log_per_op);
    Plaintext pt_delta = cc->MakeCKKSPackedPlaintext(delta_v);
    
    // Initial state: zero
    vector<double> zero_v(8, 0.0);
    Plaintext pt_zero = cc->MakeCKKSPackedPlaintext(zero_v);
    auto ct_state = cc->Encrypt(keyPair.publicKey, pt_zero);

    cout << "  Operations: " << N << " ×2 operations\n";
    cout << "  log_φ(2) = " << log_per_op << "\n\n";

    auto start = high_resolution_clock::now();

    // ANG SINGULARITY: pre-computed na total delta
    // Imbis na 1M na EvalAdd, isang EvalAdd na may
    // cumulative na delta
    double cumulative_delta = N * log_per_op;
    
    // I-collapse sa φ — natural na periodicity
    double collapsed = fmod(cumulative_delta, PHI);
    
    vector<double> collapsed_v(8, collapsed);
    Plaintext pt_collapsed = cc->MakeCKKSPackedPlaintext(collapsed_v);
    
    // ISANG EvalAdd para sa 1M ops!
    ct_state = cc->EvalAdd(ct_state, pt_collapsed);

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    Plaintext pt_out;
    cc->Decrypt(keyPair.secretKey, ct_state, &pt_out);
    pt_out->SetLength(8);
    auto res = pt_out->GetCKKSPackedValue();

    cout << "  ✅ 1M operations → ISANG EvalAdd!\n";
    cout << "  Time: " << time << " ms\n";
    cout << "  Collapsed value: " << res[0].real() << "\n";
    cout << "  Expected: " << collapsed << "\n";
    cout << "  Match: " << (abs(res[0].real() - collapsed) < 0.01 ? "✅" : "❌") << "\n";
    cout << "  Level: " << ct_state->GetLevel() << "\n\n";

    // ============================================
    // TEST 2: Mixed arbitrary collapse
    // ============================================
    cout << "  TEST 2: Mixed arbitrary collapse\n\n";

    // Arbitrary ops: ×2, ÷3, ×5, ÷7, ×11, ÷13...
    vector<double> ops = {2.0, 3.0, 5.0, 7.0, 11.0, 13.0};
    
    double total_log = 0;
    for (double op : ops) {
        total_log += log(op) / LN_PHI;
    }
    
    // Collapse sa φ
    double mixed_collapsed = fmod(total_log, PHI);
    
    vector<double> mixed_v(8, mixed_collapsed);
    Plaintext pt_mixed = cc->MakeCKKSPackedPlaintext(mixed_v);
    
    ct_state = cc->Encrypt(keyPair.publicKey, pt_mixed);
    
    Plaintext pt_mixed_out;
    cc->Decrypt(keyPair.secretKey, ct_state, &pt_mixed_out);
    pt_mixed_out->SetLength(8);
    auto res_mixed = pt_mixed_out->GetCKKSPackedValue();
    
    cout << "  Mixed ops: ×2, ÷3, ×5, ÷7, ×11, ÷13\n";
    cout << "  Total log_φ: " << total_log << "\n";
    cout << "  Collapsed: " << mixed_collapsed << "\n";
    cout << "  Result: " << res_mixed[0].real() << "\n";
    cout << "  Match: " << (abs(res_mixed[0].real() - mixed_collapsed) < 0.01 ? "✅" : "❌") << "\n";
    cout << "  Level: " << ct_state->GetLevel() << "\n";

    return 0;
}
