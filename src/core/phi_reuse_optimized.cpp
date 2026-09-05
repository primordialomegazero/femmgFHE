// ============================================
// φ-REUSE OPTIMIZED — Reuse ng Deltas
// I-encode lang ang unique deltas
// I-reuse sa loop — walang memory blow-up
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
    parameters.SetBatchSize(16);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    auto keyPair = cc->KeyGen();

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    auto encrypt_state = [&](double n) {
        vector<double> v(16, n);
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_state = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt;
        cc->Decrypt(keyPair.secretKey, ct, &pt);
        pt->SetLength(16);
        auto res = pt->GetCKKSPackedValue();
        vector<double> out;
        for (int i = 0; i < 16; i++) out.push_back(res[i].real());
        return out;
    };

    cout << "========================================\n";
    cout << "  φ-REUSE OPTIMIZED — Reuse ng Deltas\n";
    cout << "========================================\n\n";
    cout << "  I-encode lang ang unique deltas\n";
    cout << "  I-reuse sa loop — walang memory blow-up\n\n";

    // ============================================
    // TEST: 100K na may reuse ng deltas
    // ============================================
    cout << "  TEST: 100K na may reuse ng deltas\n\n";

    // Unique deltas lang — 8 iba't ibang values
    vector<double> unique_pattern = {1.0, -0.5, 2.0, -1.5, 0.5, -2.0, 1.5, -0.5};
    vector<Plaintext> unique_encoded;
    
    for (double delta : unique_pattern) {
        vector<double> d(16, delta);
        Plaintext pt = cc->MakeCKKSPackedPlaintext(d);
        unique_encoded.push_back(pt);
    }
    
    cout << "    Unique deltas encoded: " << unique_encoded.size() << "\n\n";

    auto ct = encrypt_state(10.0);
    
    auto start = high_resolution_clock::now();
    
    int N = 100000;
    
    for (int i = 0; i < N; i++) {
        // I-reuse ang pre-encoded na delta
        ct = cc->EvalAdd(ct, unique_encoded[i % unique_encoded.size()]);
    }
    
    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();
    
    auto v_final = decrypt_state(ct);
    
    cout << "    Final: " << v_final[0] << "\n";
    cout << "    Time: " << time << " ms\n";
    cout << "    Ops/sec: " << (N * 1000.0) / time << "\n";
    cout << "    Level: " << ct->GetLevel() << "\n\n";

    // ============================================
    // TEST: 1M na may reuse ng deltas
    // ============================================
    cout << "  TEST: 1M na may reuse ng deltas\n\n";

    ct = encrypt_state(10.0);
    
    start = high_resolution_clock::now();
    
    N = 1000000;
    
    for (int i = 0; i < N; i++) {
        ct = cc->EvalAdd(ct, unique_encoded[i % unique_encoded.size()]);
    }
    
    end = high_resolution_clock::now();
    time = duration_cast<milliseconds>(end - start).count();
    
    v_final = decrypt_state(ct);
    
    cout << "    Final: " << v_final[0] << "\n";
    cout << "    Time: " << time << " ms\n";
    cout << "    Ops/sec: " << (N * 1000.0) / time << "\n";
    cout << "    Level: " << ct->GetLevel() << "\n";

    return 0;
}
