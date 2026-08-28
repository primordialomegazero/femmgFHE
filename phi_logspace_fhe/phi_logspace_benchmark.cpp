// ============================================
// φ-LOG SPACE FHE — BENCHMARK
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <chrono>
#include <complex>

#include "pke/openfhe.h"

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

int main() {
    cout << "========================================\n";
    cout << "  φ-LOG SPACE FHE — BENCHMARK\n";
    cout << "========================================\n\n";
    
    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(30);
    parameters.SetScalingModSize(50);
    parameters.SetBatchSize(1);
    
    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    
    auto keyPair = cc->KeyGen();
    cc->EvalMultKeyGen(keyPair.secretKey);
    
    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);
    
    auto encrypt_log = [&](double value) {
        vector<double> val(1, log(value) / LN_PHI);
        Plaintext pt = cc->MakeCKKSPackedPlaintext(val);
        return cc->Encrypt(keyPair.publicKey, pt);
    };
    
    auto decrypt_value = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext result_pt;
        cc->Decrypt(keyPair.secretKey, ct, &result_pt);
        result_pt->SetLength(1);
        return pow(PHI, result_pt->GetCKKSPackedValue()[0].real());
    };
    
    cout << "BENCHMARK: Log-Space vs Traditional\n";
    cout << "===================================\n\n";
    
    vector<pair<double, double>> tests = {
        {7.0, 11.0}, {13.0, 17.0}, {25.0, 30.0}, {100.0, 125.0}
    };
    
    cout << "  a × b | Log-Space | Traditional | Speedup | Level\n";
    cout << "  ------|-----------|-------------|---------|-------\n";
    
    for (auto& [a, b] : tests) {
        // Log-Space (addition)
        auto start_log = high_resolution_clock::now();
        auto ct_a = encrypt_log(a);
        auto ct_b = encrypt_log(b);
        auto ct_result = cc->EvalAdd(ct_a, ct_b);
        double result_log = decrypt_value(ct_result);
        auto end_log = high_resolution_clock::now();
        auto time_log = duration_cast<milliseconds>(end_log - start_log).count();
        
        // Traditional (multiplication)
        auto start_trad = high_resolution_clock::now();
        vector<double> val_a(1, a);
        vector<double> val_b(1, b);
        Plaintext pt_a = cc->MakeCKKSPackedPlaintext(val_a);
        Plaintext pt_b = cc->MakeCKKSPackedPlaintext(val_b);
        auto ct_trad_a = cc->Encrypt(keyPair.publicKey, pt_a);
        auto ct_trad_b = cc->Encrypt(keyPair.publicKey, pt_b);
        auto ct_trad_result = cc->EvalMult(ct_trad_a, ct_trad_b);
        auto end_trad = high_resolution_clock::now();
        auto time_trad = duration_cast<milliseconds>(end_trad - start_trad).count();
        
        double speedup = (double)time_trad / max(time_log, 1L);
        int level = ct_result->GetLevel();
        
        cout << "  " << setw(4) << fixed << setprecision(0) << a << "×"
             << setw(4) << b << " | "
             << setw(9) << time_log << " ms | "
             << setw(11) << time_trad << " ms | "
             << setw(7) << setprecision(2) << speedup << "× | "
             << setw(5) << level << "\n";
    }
    
    cout << "\n✅ Benchmark complete\n";
    return 0;
}
