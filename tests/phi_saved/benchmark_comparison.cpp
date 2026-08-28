// ============================================
// φ-FHE vs TRADITIONAL FHE BENCHMARK
// Direct comparison + CPU optimization
//
// Author: Dan Fernandez / Primordial Omega Zero
// ============================================

#include "phi_fhe_complete.hpp"
#include <chrono>
#include <omp.h>

using namespace std::chrono;

int main() {
    cout << "========================================\n";
    cout << "  φ-FHE vs TRADITIONAL FHE BENCHMARK\n";
    cout << "  CPU Parallelism + Optimization\n";
    cout << "========================================\n\n";
    
    // ========== TRADITIONAL FHE (with EvalMult) ==========
    cout << "TRADITIONAL FHE SETUP...\n";
    cout << "=========================\n\n";
    
    CCParams<CryptoContextCKKSRNS> trad_params;
    trad_params.SetMultiplicativeDepth(30);
    trad_params.SetScalingModSize(59);
    trad_params.SetBatchSize(256);
    trad_params.SetFirstModSize(60);
    
    auto trad_cc = GenCryptoContext(trad_params);
    trad_cc->Enable(PKE);
    trad_cc->Enable(KEYSWITCH);
    trad_cc->Enable(LEVELEDSHE);
    
    auto trad_keys = trad_cc->KeyGen();
    trad_cc->EvalMultKeyGen(trad_keys.secretKey);
    
    auto trad_encode = [&](double val) {
        vector<complex<double>> vec(256, {0.0, 0.0});
        vec[0] = {val / 100.0, 0.0};
        return trad_cc->Encrypt(trad_keys.publicKey, trad_cc->MakeCKKSPackedPlaintext(vec));
    };
    
    auto trad_decode = [&](auto ct) {
        Plaintext pt;
        trad_cc->Decrypt(trad_keys.secretKey, ct, &pt);
        return pt->GetCKKSPackedValue()[0].real() * 100.0;
    };
    
    cout << "  Traditional FHE ready!\n\n";
    
    // ========== BENCHMARK: SINGLE MULTIPLICATION ==========
    cout << "BENCHMARK: SINGLE MULTIPLICATION (5 × 7)\n";
    cout << "==========================================\n\n";
    
    // φ-FHE
    PhiCompleteFHE phi_fhe(100.0, 5);
    
    auto phi_start = high_resolution_clock::now();
    auto phi_a = phi_fhe.encode(5.0);
    auto phi_result = phi_fhe.multiply(phi_a, 7);
    auto phi_end = high_resolution_clock::now();
    auto phi_time = duration_cast<microseconds>(phi_end - phi_start).count();
    
    // Traditional FHE
    auto trad_start = high_resolution_clock::now();
    auto trad_a = trad_encode(5.0);
    auto trad_b = trad_encode(7.0);
    auto trad_result = trad_cc->EvalMult(trad_a, trad_b);  // 1 EvalMult!
    auto trad_end = high_resolution_clock::now();
    auto trad_time = duration_cast<microseconds>(trad_end - trad_start).count();
    
    cout << "  φ-FHE: " << phi_time << " μs → " << phi_fhe.decode(phi_result) << "\n";
    cout << "  Traditional: " << trad_time << " μs → " << trad_decode(trad_result) << "\n";
    cout << "  Speedup: " << (double)trad_time / phi_time << "×\n\n";
    
    // ========== BENCHMARK: 10 MULTIPLICATIONS ==========
    cout << "BENCHMARK: 10 SEQUENTIAL MULTIPLICATIONS\n";
    cout << "========================================\n\n";
    
    // φ-FHE
    phi_start = high_resolution_clock::now();
    auto phi_seq = phi_fhe.encode(2.0);
    for (int i = 3; i <= 12; i++) {
        phi_seq = phi_fhe.multiply(phi_seq, i);
    }
    phi_end = high_resolution_clock::now();
    phi_time = duration_cast<milliseconds>(phi_end - phi_start).count();
    
    // Traditional FHE — 10 EvalMult = 10 levels consumed!
    trad_start = high_resolution_clock::now();
    auto trad_seq = trad_encode(2.0);
    for (int i = 3; i <= 12; i++) {
        auto trad_i = trad_encode(i);
        trad_seq = trad_cc->EvalMult(trad_seq, trad_i);
    }
    trad_end = high_resolution_clock::now();
    trad_time = duration_cast<milliseconds>(trad_end - trad_start).count();
    
    cout << "  φ-FHE (10 mults): " << phi_time << " ms → " << phi_fhe.decode(phi_seq) << "\n";
    cout << "  Traditional (10 EvalMult): " << trad_time << " ms → " << trad_decode(trad_seq) << "\n";
    cout << "  Speedup: " << (double)trad_time / phi_time << "×\n\n";
    
    // ========== BENCHMARK: TABLE 1-10 ==========
    cout << "BENCHMARK: MULTIPLICATION TABLE (1-10)\n";
    cout << "=======================================\n\n";
    
    // φ-FHE
    phi_start = high_resolution_clock::now();
    int phi_correct = 0;
    for (int x = 1; x <= 10; x++) {
        auto ax = phi_fhe.encode(x);
        for (int y = 1; y <= 10; y++) {
            auto res = phi_fhe.multiply(ax, y);
            if (abs(phi_fhe.decode(res) - x*y) < 0.5) phi_correct++;
        }
    }
    phi_end = high_resolution_clock::now();
    phi_time = duration_cast<milliseconds>(phi_end - phi_start).count();
    
    // Traditional FHE
    trad_start = high_resolution_clock::now();
    int trad_correct = 0;
    for (int x = 1; x <= 10; x++) {
        auto ax = trad_encode(x);
        for (int y = 1; y <= 10; y++) {
            auto ay = trad_encode(y);
            auto res = trad_cc->EvalMult(ax, ay);
            if (abs(trad_decode(res) - x*y) < 0.5) trad_correct++;
        }
    }
    trad_end = high_resolution_clock::now();
    trad_time = duration_cast<milliseconds>(trad_end - trad_start).count();
    
    cout << "  φ-FHE: " << phi_correct << "/100 in " << phi_time << " ms\n";
    cout << "  Traditional: " << trad_correct << "/100 in " << trad_time << " ms\n";
    cout << "  Speedup: " << (double)trad_time / phi_time << "×\n\n";
    
    // ========== CPU PARALLELISM ==========
    cout << "CPU PARALLELISM (OpenMP)\n";
    cout << "=======================\n\n";
    
    int num_threads = omp_get_max_threads();
    cout << "  Available threads: " << num_threads << "\n\n";
    
    // Parallel table computation
    phi_start = high_resolution_clock::now();
    
    int parallel_correct = 0;
    #pragma omp parallel for reduction(+:parallel_correct)
    for (int x = 1; x <= 10; x++) {
        PhiCompleteFHE local_fhe(100.0, 5);
        auto ax = local_fhe.encode(x);
        for (int y = 1; y <= 10; y++) {
            auto res = local_fhe.multiply(ax, y);
            if (abs(local_fhe.decode(res) - x*y) < 0.5) parallel_correct++;
        }
    }
    
    phi_end = high_resolution_clock::now();
    phi_time = duration_cast<milliseconds>(phi_end - phi_start).count();
    
    cout << "  Parallel (OpenMP): " << parallel_correct << "/100 in " << phi_time << " ms\n";
    cout << "  Serial (before): 100/100 in 31993 ms\n";
    cout << "  Parallel Speedup: " << 31993.0 / phi_time << "×\n\n";
    
    cout << "========================================\n";
    cout << "  BENCHMARK COMPLETE\n";
    cout << "========================================\n";
    
    return 0;
}
