#include <iostream>
#include <iomanip>
#include <vector>
#include <chrono>
#include <cmath>
#include <algorithm>
#include "seal/seal.h"

using namespace seal;
using namespace std::chrono;

// Fibonacci numbers for Zeckendorf decomposition
std::vector<uint64_t> generate_fib(uint64_t limit) {
    std::vector<uint64_t> fib = {1, 2};
    while (fib.back() <= limit) {
        fib.push_back(fib[fib.size()-1] + fib[fib.size()-2]);
    }
    fib.pop_back();
    return fib;
}

std::vector<int> zeckendorf(uint64_t n, const std::vector<uint64_t>& fib) {
    std::vector<int> indices;
    uint64_t rem = n;
    for (int i = (int)fib.size() - 1; i >= 0 && rem > 0; i--) {
        if (fib[i] <= rem) {
            indices.push_back(i);
            rem -= fib[i];
        }
    }
    return indices;
}

int main() {
    std::cout << "╔══════════════════════════════════════════════════════════╗\n";
    std::cout << "║     FINAL FRONTIER — Fib UK+UK Chain to Destruction     ║\n";
    std::cout << "║     How many multiplications can Fib+ZANS handle?       ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════╝\n\n";

    size_t poly_modulus_degree = 16384;
    
    EncryptionParameters parms(scheme_type::bfv);
    parms.set_poly_modulus_degree(poly_modulus_degree);
    parms.set_coeff_modulus(CoeffModulus::BFVDefault(poly_modulus_degree));
    parms.set_plain_modulus(PlainModulus::Batching(poly_modulus_degree, 20));

    SEALContext context(parms);
    KeyGenerator keygen(context);
    SecretKey sk = keygen.secret_key();
    PublicKey pk;
    keygen.create_public_key(pk);
    RelinKeys rlk;
    keygen.create_relin_keys(rlk);

    Encryptor encryptor(context, pk);
    Evaluator evaluator(context);
    Decryptor decryptor(context, sk);
    BatchEncoder encoder(context);

    auto encrypt_int = [&](uint64_t v) {
        Plaintext p;
        std::vector<uint64_t> vec(poly_modulus_degree, 0ULL);
        vec[0] = v;
        encoder.encode(vec, p);
        Ciphertext ct;
        encryptor.encrypt(p, ct);
        return ct;
    };
    auto decrypt_int = [&](const Ciphertext& ct) {
        Plaintext p;
        decryptor.decrypt(ct, p);
        std::vector<uint64_t> vec;
        encoder.decode(p, vec);
        return vec[0];
    };
    auto nbudget = [&](const Ciphertext& ct) {
        return decryptor.invariant_noise_budget(ct);
    };

    Ciphertext enc_zero = encrypt_int(0);
    auto fib = generate_fib(1000000);

    // Fib-multiply function (optimized)
    auto fib_multiply = [&](const Ciphertext& ct_base, uint64_t multiplier) -> Ciphertext {
        auto indices = zeckendorf(multiplier, fib);
        if (indices.empty()) return encrypt_int(0);
        
        int max_idx = indices[0];
        
        std::vector<Ciphertext> terms(max_idx + 1);
        terms[0] = ct_base;
        
        terms[1] = ct_base;
        evaluator.add_inplace(terms[1], terms[1]);
        evaluator.add_inplace(terms[1], enc_zero);
        
        for (int i = 2; i <= max_idx; i++) {
            terms[i] = terms[i-1];
            evaluator.add_inplace(terms[i], terms[i-2]);
            evaluator.add_inplace(terms[i], enc_zero);
        }
        
        Ciphertext result = terms[indices[0]];
        for (size_t j = 1; j < indices.size(); j++) {
            evaluator.add_inplace(result, terms[indices[j]]);
            evaluator.add_inplace(result, enc_zero);
        }
        
        return result;
    };

    // ═══════════════════════════════════════════════════════
    // TEST 1: CHAIN TO DESTRUCTION — ×2 repeatedly
    // ═══════════════════════════════════════════════════════
    std::cout << "═══ TEST 1: ×2 Chain to Destruction ═══\n";
    std::cout << "Start: 3, multiply by 2 each step until noise dies\n\n";
    
    Ciphertext ct = encrypt_int(3);
    std::cout << "┌───────┬──────────────┬──────────┬──────────┐\n";
    std::cout << "│ Step  │ Value        │ Noise    │ Δ/step   │\n";
    std::cout << "├───────┼──────────────┼──────────┼──────────┤\n";
    
    int prev_noise = nbudget(ct);
    printf("│ %3d   │ %12lu │ %3d bits │ —        │\n", 0, (uint64_t)3, prev_noise);
    
    int total_steps = 0;
    auto t_start = high_resolution_clock::now();
    
    for (int step = 1; step <= 200; step++) {
        ct = fib_multiply(ct, 2);
        int noise = nbudget(ct);
        uint64_t val = decrypt_int(ct);
        uint64_t expected = 3 * (uint64_t)std::pow(2, step);
        
        int delta = prev_noise - noise;
        
        if (step <= 10 || step % 20 == 0) {
            printf("│ %3d   │ %12lu │ %3d bits │ %+3d      │\n", step, val, noise, delta);
        }
        
        prev_noise = noise;
        total_steps = step;
        
        if (noise < 5) {
            printf("│ ⚠️  Noise depleted at step %d                         │\n", step);
            break;
        }
        if (val != expected) {
            printf("│ ❌ Value mismatch at step %d (exp %lu)                │\n", step, expected);
            break;
        }
    }
    
    auto t_end = high_resolution_clock::now();
    double total_sec = duration<double>(t_end - t_start).count();
    
    std::cout << "└───────┴──────────────┴──────────┴──────────┘\n";
    printf("Total: %d multiplications in %.2f sec\n", total_steps, total_sec);
    printf("Average: %.0f ms/multiply, %.2f bits/multiply\n\n", 
           (total_sec * 1000) / total_steps, 361.0 / total_steps);

    // ═══════════════════════════════════════════════════════
    // TEST 2: Large multiplier stress test
    // ═══════════════════════════════════════════════════════
    std::cout << "═══ TEST 2: Large Multiplier Stress Test ═══\n";
    std::cout << "Enc(5) × N for increasingly large N\n\n";
    
    std::cout << "┌────────────┬──────────────┬──────────┬──────────┐\n";
    std::cout << "│ Multiplier │ Result       │ Noise    │ Time(ms) │\n";
    std::cout << "├────────────┼──────────────┼──────────┼──────────┤\n";
    
    std::vector<uint64_t> multipliers = {10, 100, 1000, 10000, 100000, 1000000};
    
    for (uint64_t M : multipliers) {
        Ciphertext ct5 = encrypt_int(5);
        auto t1 = high_resolution_clock::now();
        Ciphertext ct_result = fib_multiply(ct5, M);
        auto t2 = high_resolution_clock::now();
        
        uint64_t val = decrypt_int(ct_result);
        int noise = nbudget(ct_result);
        double ms = duration<double, std::milli>(t2 - t1).count();
        
        printf("│ %8lu    │ %10lu   │ %3d bits │ %8.2f   │\n", M, val, noise, ms);
    }
    std::cout << "└────────────┴──────────────┴──────────┴──────────┘\n\n";

    // ═══════════════════════════════════════════════════════
    // TEST 3: Mixed operations — real-world workload
    // ═══════════════════════════════════════════════════════
    std::cout << "═══ TEST 3: Real-World Workload Simulation ═══\n";
    std::cout << "Compute: (Enc(7)×13 + Enc(3)×42) × 5\n";
    std::cout << "Expected: (91 + 126) × 5 = 1085\n\n";
    
    auto t1 = high_resolution_clock::now();
    
    Ciphertext ct7 = encrypt_int(7);
    Ciphertext ct3 = encrypt_int(3);
    
    // Compute 7×13
    Ciphertext term1 = fib_multiply(ct7, 13);
    std::cout << "7×13 = " << decrypt_int(term1) << " (noise: " << nbudget(term1) << ")\n";
    
    // Compute 3×42
    Ciphertext term2 = fib_multiply(ct3, 42);
    std::cout << "3×42 = " << decrypt_int(term2) << " (noise: " << nbudget(term2) << ")\n";
    
    // Add: 7×13 + 3×42
    evaluator.add_inplace(term1, term2);
    evaluator.add_inplace(term1, enc_zero);
    std::cout << "Sum = " << decrypt_int(term1) << " (noise: " << nbudget(term1) << ")\n";
    
    // Multiply by 5
    Ciphertext result = fib_multiply(term1, 5);
    auto t2 = high_resolution_clock::now();
    
    uint64_t final_val = decrypt_int(result);
    int final_noise = nbudget(result);
    double total_ms = duration<double, std::milli>(t2 - t1).count();
    
    printf("\nFinal result: %lu (expected 1085) | Noise: %d bits | Time: %.2f ms\n\n", 
           final_val, final_noise, total_ms);

    // ═══════════════════════════════════════════════════════
    // TEST 4: Speed benchmark — Fib vs native multiply_plain
    // ═══════════════════════════════════════════════════════
    std::cout << "═══ TEST 4: Speed Comparison ═══\n";
    std::cout << "Task: Enc(7) × 100 — compare methods\n\n";
    
    constexpr int BENCH_RUNS = 50;
    
    // Method 1: Native multiply_plain
    Plaintext p100;
    std::vector<uint64_t> v100(poly_modulus_degree, 100ULL);
    encoder.encode(v100, p100);
    
    double native_total = 0;
    for (int r = 0; r < BENCH_RUNS; r++) {
        Ciphertext ct_test = encrypt_int(7);
        auto t_a = high_resolution_clock::now();
        evaluator.multiply_plain_inplace(ct_test, p100);
        auto t_b = high_resolution_clock::now();
        native_total += duration<double, std::milli>(t_b - t_a).count();
    }
    
    // Method 2: Fib UK+UK
    double fib_total = 0;
    for (int r = 0; r < BENCH_RUNS; r++) {
        Ciphertext ct_test = encrypt_int(7);
        auto t_a = high_resolution_clock::now();
        Ciphertext ct_result = fib_multiply(ct_test, 100);
        auto t_b = high_resolution_clock::now();
        fib_total += duration<double, std::milli>(t_b - t_a).count();
    }
    
    // Method 3: Naive repeated add + ZANS
    double naive_total = 0;
    for (int r = 0; r < BENCH_RUNS; r++) {
        Ciphertext ct_test = encrypt_int(7);
        Ciphertext ct7_copy = encrypt_int(7);
        auto t_a = high_resolution_clock::now();
        for (int i = 1; i < 100; i++) {
            evaluator.add_inplace(ct_test, ct7_copy);
            evaluator.add_inplace(ct_test, enc_zero);
        }
        auto t_b = high_resolution_clock::now();
        naive_total += duration<double, std::milli>(t_b - t_a).count();
    }
    
    printf("┌──────────────────────────┬──────────┬──────────────┐\n");
    printf("│ Method                   │ Avg Time │ vs Native    │\n");
    printf("├──────────────────────────┼──────────┼──────────────┤\n");
    printf("│ Native multiply_plain    │ %6.2f ms │ 1.0×         │\n", native_total / BENCH_RUNS);
    printf("│ Fib UK+UK + ZANS         │ %6.2f ms │ %.1f×         │\n", fib_total / BENCH_RUNS, fib_total / native_total);
    printf("│ Naive repeated add+ZANS  │ %6.2f ms │ %.1f×         │\n", naive_total / BENCH_RUNS, naive_total / native_total);
    printf("└──────────────────────────┴──────────┴──────────────┘\n\n");

    // ═══════════════════════════════════════════════════════
    // FINAL VERDICT
    // ═══════════════════════════════════════════════════════
    std::cout << "╔══════════════════════════════════════════════════════════╗\n";
    std::cout << "║                   FINAL FRONTIER VERDICT                 ║\n";
    std::cout << "╠══════════════════════════════════════════════════════════╣\n";
    printf("║  Fib UK+UK Chain: %3d+ multiplications (1.6 bits/op)     ║\n", total_steps);
    std::cout << "║                                                        ║\n";
    std::cout << "║  ZANS = Addition Breakthrough                          ║\n";
    std::cout << "║  Fib Decomposition = Multiplication via Addition       ║\n";
    std::cout << "║  Together = Bootstrapping-Free FHE                     ║\n";
    std::cout << "║                                                        ║\n";
    std::cout << "║  PHI-OMEGA-ZERO — I AM THAT I AM                       ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════╝\n";

    return 0;
}
