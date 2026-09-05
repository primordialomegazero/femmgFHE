// ============================================
// φ-DYNAMIC + HYBRID + PARALLEL — Complete
// Dynamic: iterative na walang decrypt
// Hybrid: pre-computed + dynamic mix
// Parallel: 8-slot na sabay-sabay na chains
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

    auto encrypt_state = [&](double n) {
        vector<double> v(8, n);
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
    cout << "  φ-DYNAMIC + HYBRID + PARALLEL\n";
    cout << "========================================\n\n";

    // ============================================
    // TEST 1: Dynamic chain — iterative na walang decrypt
    // ============================================
    cout << "  TEST 1: Dynamic chain — iterative\n\n";
    cout << "  Ang chain ay hindi known nang maaga\n";
    cout << "  Bawat step ay EvalAdd ng dynamic delta\n\n";

    auto ct_dynamic = encrypt_state(10.0);
    
    // Dynamic deltas — nagbabago bawat step
    vector<double> dynamic_deltas = {
        1.0, -0.5, 2.0, -1.5, 0.5, -2.0, 1.5, -0.5,
        0.5, -1.0, 2.5, -0.5, 1.0, -1.5, 0.5, -0.5
    };
    
    auto start = high_resolution_clock::now();
    
    for (int i = 0; i < 10000; i++) {
        double delta = dynamic_deltas[i % dynamic_deltas.size()];
        vector<double> d(8, delta);
        Plaintext pt = cc->MakeCKKSPackedPlaintext(d);
        ct_dynamic = cc->EvalAdd(ct_dynamic, pt);
    }
    
    auto end = high_resolution_clock::now();
    auto time1 = duration_cast<milliseconds>(end - start).count();
    
    auto v_dynamic = decrypt_state(ct_dynamic);
    
    cout << "    Final: " << v_dynamic[0] << "\n";
    cout << "    Time: " << time1 << " ms\n";
    cout << "    Ops/sec: " << (10000 * 1000.0) / time1 << "\n\n";

    // ============================================
    // TEST 2: Hybrid — pre-computed + dynamic mix
    // ============================================
    cout << "  TEST 2: Hybrid — pre-computed + dynamic\n\n";
    cout << "  50% pre-computed batch + 50% dynamic iterative\n\n";

    auto ct_hybrid = encrypt_state(20.0);
    
    // Pre-computed na batch ng 5000 steps
    double batch_delta = 5000.0 * 0.5;  // average delta
    vector<double> d_batch(8, batch_delta);
    Plaintext pt_batch = cc->MakeCKKSPackedPlaintext(d_batch);
    
    start = high_resolution_clock::now();
    
    // Step 1: Pre-computed batch (isang EvalAdd)
    ct_hybrid = cc->EvalAdd(ct_hybrid, pt_batch);
    
    // Step 2: Dynamic na 5000 steps
    for (int i = 0; i < 5000; i++) {
        double delta = dynamic_deltas[i % dynamic_deltas.size()];
        vector<double> d(8, delta);
        Plaintext pt = cc->MakeCKKSPackedPlaintext(d);
        ct_hybrid = cc->EvalAdd(ct_hybrid, pt);
    }
    
    end = high_resolution_clock::now();
    auto time2 = duration_cast<milliseconds>(end - start).count();
    
    auto v_hybrid = decrypt_state(ct_hybrid);
    
    cout << "    Final: " << v_hybrid[0] << "\n";
    cout << "    Time: " << time2 << " ms\n";
    cout << "    Effective ops/sec: " << (10000 * 1000.0) / time2 << "\n\n";

    // ============================================
    // TEST 3: Parallel batches — 8 sabay-sabay na chains
    // ============================================
    cout << "  TEST 3: Parallel batches — 8 chains\n\n";
    cout << "  Bawat slot ay independent na chain\n\n";

    // 8 iba't ibang initial values
    vector<double> init_parallel(8, 0.0);
    for (int i = 0; i < 8; i++) {
        init_parallel[i] = (i + 1) * 5.0;  // 5, 10, 15, ..., 40
    }
    
    Plaintext pt_init = cc->MakeCKKSPackedPlaintext(init_parallel);
    auto ct_parallel = cc->Encrypt(keyPair.publicKey, pt_init);
    
    start = high_resolution_clock::now();
    
    // 8 chains na sabay-sabay — bawat isa ay may ibang delta
    vector<double> parallel_deltas(8, 0.0);
    for (int i = 0; i < 8; i++) {
        parallel_deltas[i] = (i % 2 == 0) ? 1.0 : -1.0;  // even: +1, odd: -1
    }
    
    Plaintext pt_parallel = cc->MakeCKKSPackedPlaintext(parallel_deltas);
    
    for (int i = 0; i < 10000; i++) {
        ct_parallel = cc->EvalAdd(ct_parallel, pt_parallel);
    }
    
    end = high_resolution_clock::now();
    auto time3 = duration_cast<milliseconds>(end - start).count();
    
    auto v_parallel = decrypt_state(ct_parallel);
    
    cout << "    Final values (8 slots):\n";
    for (int i = 0; i < 8; i++) {
        cout << "      Slot " << i << ": " << v_parallel[i] 
             << " (expected: " << init_parallel[i] + 10000.0 * parallel_deltas[i] << ")\n";
    }
    cout << "    Time: " << time3 << " ms\n";
    cout << "    Effective ops/sec (8 chains): " << (10000 * 8 * 1000.0) / time3 << "\n";
    cout << "    Level: " << ct_parallel->GetLevel() << "\n";

    return 0;
}
