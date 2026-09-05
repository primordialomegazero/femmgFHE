// ============================================
// φ-OPTIMIZED PARALLEL — Pre-encoded Deltas
// Pre-encode lahat ng deltas
// Batch size 16 para sa mas maraming chains
// Profile: EvalAdd vs Encoding
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
    cout << "  φ-OPTIMIZED PARALLEL — Pre-encoded\n";
    cout << "========================================\n\n";

    // ============================================
    // PROFILE: Encoding vs EvalAdd
    // ============================================
    cout << "  PROFILE: Encoding vs EvalAdd\n\n";

    // Pre-encode ang deltas
    vector<Plaintext> pre_encoded;
    int N_pre = 1000;
    
    auto start_encode = high_resolution_clock::now();
    
    for (int i = 0; i < N_pre; i++) {
        vector<double> d(16, 0.5);
        Plaintext pt = cc->MakeCKKSPackedPlaintext(d);
        pre_encoded.push_back(pt);
    }
    
    auto end_encode = high_resolution_clock::now();
    auto time_encode = duration_cast<milliseconds>(end_encode - start_encode).count();
    
    cout << "    Encoding " << N_pre << " deltas: " << time_encode << " ms\n";
    cout << "    Encoding rate: " << (N_pre * 1000.0) / time_encode << " deltas/sec\n\n";

    // Profile EvalAdd
    auto ct = encrypt_state(10.0);
    
    auto start_eval = high_resolution_clock::now();
    
    for (int i = 0; i < N_pre; i++) {
        ct = cc->EvalAdd(ct, pre_encoded[i]);
    }
    
    auto end_eval = high_resolution_clock::now();
    auto time_eval = duration_cast<milliseconds>(end_eval - start_eval).count();
    
    cout << "    EvalAdd " << N_pre << " deltas: " << time_eval << " ms\n";
    cout << "    EvalAdd rate: " << (N_pre * 1000.0) / time_eval << " ops/sec\n\n";

    // ============================================
    // TEST 1: 100K na pre-encoded na dynamic
    // ============================================
    cout << "  TEST 1: 100K na pre-encoded na dynamic\n\n";

    // Pre-encode ang buong chain
    vector<Plaintext> chain_encoded;
    vector<double> pattern = {1.0, -0.5, 2.0, -1.5, 0.5, -2.0, 1.5, -0.5};
    
    start_encode = high_resolution_clock::now();
    
    for (int i = 0; i < 100000; i++) {
        double delta = pattern[i % pattern.size()];
        vector<double> d(16, delta);
        Plaintext pt = cc->MakeCKKSPackedPlaintext(d);
        chain_encoded.push_back(pt);
    }
    
    end_encode = high_resolution_clock::now();
    time_encode = duration_cast<milliseconds>(end_encode - start_encode).count();
    
    cout << "    Pre-encoding 100K deltas: " << time_encode << " ms\n\n";

    // I-apply ang pre-encoded na chain
    ct = encrypt_state(10.0);
    
    auto start = high_resolution_clock::now();
    
    for (int i = 0; i < 100000; i++) {
        ct = cc->EvalAdd(ct, chain_encoded[i]);
    }
    
    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();
    
    auto v_final = decrypt_state(ct);
    
    cout << "    Final: " << v_final[0] << "\n";
    cout << "    EvalAdd time: " << time << " ms\n";
    cout << "    EvalAdd rate: " << (100000 * 1000.0) / time << " ops/sec\n";
    cout << "    Total time (encode + eval): " << time_encode + time << " ms\n";
    cout << "    Level: " << ct->GetLevel() << "\n\n";

    // ============================================
    // TEST 2: Batch size 16 — parallel chains
    // ============================================
    cout << "  TEST 2: Batch size 16 — 16 parallel chains\n\n";

    vector<double> init_16(16, 0.0);
    for (int i = 0; i < 16; i++) {
        init_16[i] = (i + 1) * 3.0;
    }
    
    Plaintext pt_init_16 = cc->MakeCKKSPackedPlaintext(init_16);
    auto ct_16 = cc->Encrypt(keyPair.publicKey, pt_init_16);
    
    vector<double> delta_16(16, 0.0);
    for (int i = 0; i < 16; i++) {
        delta_16[i] = (i % 3 == 0) ? 1.0 : (i % 3 == 1) ? -0.5 : 0.5;
    }
    
    Plaintext pt_delta_16 = cc->MakeCKKSPackedPlaintext(delta_16);
    
    start = high_resolution_clock::now();
    
    for (int i = 0; i < 10000; i++) {
        ct_16 = cc->EvalAdd(ct_16, pt_delta_16);
    }
    
    end = high_resolution_clock::now();
    time = duration_cast<milliseconds>(end - start).count();
    
    auto v_16 = decrypt_state(ct_16);
    
    cout << "    Final values: ";
    for (int i = 0; i < 4; i++) {
        cout << v_16[i] << " ";
    }
    cout << "...\n";
    cout << "    Time: " << time << " ms\n";
    cout << "    Effective ops/sec (16 chains): " << (10000 * 16 * 1000.0) / time << "\n";
    cout << "    Level: " << ct_16->GetLevel() << "\n";

    return 0;
}
