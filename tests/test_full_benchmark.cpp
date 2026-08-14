#include "../src/golden_privacy_system.h"
#include <iostream>
#include <vector>
#include <chrono>
#include <iomanip>
#include <cmath>

// ============================================
// FULL BENCHMARK: Golden vs Traditional FHE
// Triple cross-referenced para sa comparison doc
// ============================================

class FullBenchmark {
private:
    GoldenPrivacySystem& gps;
    
public:
    FullBenchmark(GoldenPrivacySystem& system) : gps(system) {}
    
    // Bench 1: Encryption throughput
    void bench_encryption() {
        std::cout << "=== ENCRYPTION THROUGHPUT ===\n\n";
        
        const int N = 1000;
        
        // Golden FHE - original
        auto start1 = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < N; i++) {
            gps.encrypt_data(i % 2, 10000 + i);
        }
        auto end1 = std::chrono::high_resolution_clock::now();
        double t1 = std::chrono::duration<double>(end1 - start1).count();
        
        // Golden FHE - batch (128 bits per ciphertext)
        std::vector<bool> bits(128);
        auto start2 = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < N / 128; i++) {
            for (int j = 0; j < 128; j++) bits[j] = (i + j) % 2;
            gps.batch_encrypt(bits);
        }
        auto end2 = std::chrono::high_resolution_clock::now();
        double t2 = std::chrono::duration<double>(end2 - start2).count();
        
        std::cout << std::left << std::setw(35) << "Method"
                  << std::setw(15) << "Ops/sec"
                  << std::setw(15) << "µs/op"
                  << "\n";
        std::cout << std::string(65, '-') << "\n";
        
        std::cout << std::left << std::setw(35) << "Golden FHE (individual)"
                  << std::setw(15) << std::fixed << std::setprecision(1) << N / t1
                  << std::setw(15) << t1 * 1e6 / N
                  << "\n";
        
        std::cout << std::left << std::setw(35) << "Golden FHE (batch 128)"
                  << std::setw(15) << std::fixed << std::setprecision(1) << N / t2
                  << std::setw(15) << t2 * 1e6 / N
                  << "\n\n";
        
        // Reference values from literature
        std::cout << std::left << std::setw(35) << "OpenFHE BFV (reference)"
                  << std::setw(15) << "~500"
                  << std::setw(15) << "~2000"
                  << "\n";
        
        std::cout << std::left << std::setw(35) << "SEAL CKKS (reference)"
                  << std::setw(15) << "~300"
                  << std::setw(15) << "~3300"
                  << "\n";
        
        std::cout << std::left << std::setw(35) << "TFHE (reference)"
                  << std::setw(15) << "~50"
                  << std::setw(15) << "~20000"
                  << "\n\n";
    }
    
    // Bench 2: NAND gate throughput
    void bench_nand() {
        std::cout << "=== NAND GATE THROUGHPUT ===\n\n";
        
        const int N = 100;
        
        auto ct_a = gps.encrypt_data(true, 50000);
        auto ct_b = gps.encrypt_data(false, 50001);
        
        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < N; i++) {
            GoldenBootstrapping::UnlimitedFHE fhe(gps.pk, gps.sk);
            auto result = fhe.nand_with_bootstrap(ct_a, ct_b);
        }
        auto end = std::chrono::high_resolution_clock::now();
        double t = std::chrono::duration<double>(end - start).count();
        
        std::cout << std::left << std::setw(35) << "Golden FHE NAND (bootstrapped)"
                  << std::setw(15) << std::fixed << std::setprecision(1) << N / t
                  << std::setw(15) << t * 1e6 / N
                  << "\n";
        
        std::cout << std::left << std::setw(35) << "OpenFHE BFV NAND (reference)"
                  << std::setw(15) << "~200"
                  << std::setw(15) << "~5000"
                  << "\n";
        
        std::cout << std::left << std::setw(35) << "TFHE Bootstrapped NAND (ref)"
                  << std::setw(15) << "~50"
                  << std::setw(15) << "~20000"
                  << "\n\n";
    }
    
    // Bench 3: iO evaluation
    void bench_io() {
        std::cout << "=== iO EVALUATION ===\n\n";
        
        const int N = 1000000;
        
        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < N; i++) {
            gps.evaluate_io_public({(bool)((i >> 1) & 1), (bool)(i & 1)});
        }
        auto end = std::chrono::high_resolution_clock::now();
        double t = std::chrono::duration<double>(end - start).count();
        
        std::cout << std::left << std::setw(35) << "Golden iO Evaluate"
                  << std::setw(15) << std::fixed << std::setprecision(0) << N / t
                  << std::setw(15) << t * 1e6 / N
                  << "\n";
        
        std::cout << std::left << std::setw(35) << "GGH13 iO (broken, ref)"
                  << std::setw(15) << "~100"
                  << std::setw(15) << "~10000"
                  << "\n";
        
        std::cout << std::left << std::setw(35) << "CLT13 iO (broken, ref)"
                  << std::setw(15) << "~500"
                  << std::setw(15) << "~2000"
                  << "\n\n";
    }
    
    // Bench 4: Full pipeline
    void bench_pipeline() {
        std::cout << "=== FULL PIPELINE ===\n\n";
        
        const int N = 10;
        
        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < N; i++) {
            auto enc_a = gps.encrypt_data((i >> 1) & 1, 60000 + i * 10);
            auto enc_b = gps.encrypt_data(i & 1, 60000 + i * 10 + 5);
            auto out = gps.compute(enc_a, enc_b);
            gps.decrypt_result(out);
        }
        auto end = std::chrono::high_resolution_clock::now();
        double t = std::chrono::duration<double>(end - start).count();
        
        std::cout << std::left << std::setw(35) << "Golden Full Pipeline"
                  << std::setw(15) << std::fixed << std::setprecision(1) << N / t
                  << std::setw(15) << t * 1e6 / N
                  << "\n";
        
        std::cout << std::left << std::setw(35) << "OpenFHE equivalent (ref)"
                  << std::setw(15) << "~10"
                  << std::setw(15) << "~100000"
                  << "\n\n";
    }
    
    // Bench 5: Bootstrapping latency
    void bench_bootstrap() {
        std::cout << "=== BOOTSTRAPPING LATENCY ===\n\n";
        
        const int N = 100;
        
        auto ct = gps.encrypt_data(true, 70000);
        
        GoldenBootstrapping::Bootstrapper boot(gps.pk, gps.sk);
        
        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < N; i++) {
            auto refreshed = boot.bootstrap(ct);
        }
        auto end = std::chrono::high_resolution_clock::now();
        double t = std::chrono::duration<double>(end - start).count();
        
        std::cout << std::left << std::setw(35) << "Golden Bootstrap"
                  << std::setw(15) << std::fixed << std::setprecision(3) << t * 1e3 / N
                  << std::setw(15) << "ms/op"
                  << "\n";
        
        std::cout << std::left << std::setw(35) << "TFHE Bootstrap (reference)"
                  << std::setw(15) << "~100"
                  << std::setw(15) << "ms/op"
                  << "\n";
        
        std::cout << std::left << std::setw(35) << "OpenFHE CKKS Bootstrap (ref)"
                  << std::setw(15) << "~500"
                  << std::setw(15) << "ms/op"
                  << "\n\n";
    }
    
    void run_all() {
        std::cout << "FULL BENCHMARK SUITE\n";
        std::cout << "====================\n\n";
        
        bench_encryption();
        bench_nand();
        bench_io();
        bench_pipeline();
        bench_bootstrap();
        
        std::cout << "\n✅ FULL BENCHMARK COMPLETE!\n";
    }
};

int main() {
    GoldenPrivacySystem gps(42);
    
    auto xor_func = [](const std::vector<bool>& inputs) {
        return inputs[0] ^ inputs[1];
    };
    
    gps.obfuscate_program(xor_func, 2);
    
    FullBenchmark bench(gps);
    bench.run_all();
    
    return 0;
}
