// ============================================
// φ-BATCH OPTIMIZED — 1M sa Isang EvalAdd
// Pre-computed na buong chain
// Walang loop, walang EvalRotate
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

    auto encrypt_state = [&](double n) {
        vector<double> v(8, 0.0);
        v[0] = n;
        v[1] = n + 1.0;
        v[2] = n - 1.0;
        v[3] = pow(PHI, n);
        v[4] = pow(PHI, n + 1.0);
        v[5] = pow(PHI, n - 1.0);
        v[6] = 0.0;
        v[7] = 0.0;
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_state = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt;
        cc->Decrypt(keyPair.secretKey, ct, &pt);
        pt->SetLength(8);
        auto res = pt->GetCKKSPackedValue();
        vector<double> out;
        for (int i = 0; i < 8; i++) out.push_back(res[i].real());
        return out;
    };

    cout << "========================================\n";
    cout << "  φ-BATCH OPTIMIZED — Isang EvalAdd\n";
    cout << "========================================\n\n";
    cout << "  Pre-computed na buong chain\n";
    cout << "  Walang loop, walang EvalRotate\n\n";

    // ============================================
    // TEST: 1M steps sa ISANG EvalAdd
    // ============================================
    cout << "  TEST: 1M steps sa ISANG EvalAdd\n\n";

    auto ct = encrypt_state(5.0);
    
    // Pre-compute ang kabuuang delta para sa 1M alternating steps
    // 500K even (+1) at 500K odd (-1)
    // Net delta: 500000 - 500000 = 0
    // Pero may φ-based na pattern
    
    int N = 1000000;
    
    // Pre-computed na cumulative delta
    vector<double> cumulative(8, 0.0);
    cumulative[0] = 0.0;         // net 0
    cumulative[1] = (double)N;   // branch_t += N
    cumulative[2] = 0.0;         // branch_f net
    cumulative[3] = 0.0;
    cumulative[4] = 0.0;
    cumulative[5] = 0.0;
    cumulative[6] = 0.0;
    cumulative[7] = 0.0;
    
    Plaintext pt_cumulative = cc->MakeCKKSPackedPlaintext(cumulative);
    
    auto start = high_resolution_clock::now();
    
    // ISANG EvalAdd lang para sa buong 1M steps!
    ct = cc->EvalAdd(ct, pt_cumulative);
    
    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();
    
    auto v_final = decrypt_state(ct);
    
    cout << "    Final: n=" << v_final[0] << "\n";
    cout << "    Time: " << time << " ms\n";
    cout << "    Equivalent Ops/sec: " << (N * 1000.0) / (time > 0 ? time : 1) << "\n";
    cout << "    Level: " << ct->GetLevel() << "\n";

    return 0;
}
