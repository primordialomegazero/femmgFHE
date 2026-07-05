#include <iostream>
#include <iomanip>
#include <vector>
#include <chrono>
#include <cmath>
#include <fstream>
#include "seal/seal.h"

using namespace seal;
using namespace std::chrono;

int main() {
    std::cout << "╔══════════════════════════════════════════════════════════╗\n";
    std::cout << "║  1 MILLION ZANS OPERATIONS — ULTIMATE STRESS TEST      ║\n";
    std::cout << "║  Can ct + Enc(0) survive 1,000,000 iterations?         ║\n";
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

    Ciphertext ct = encrypt_int(42);
    Ciphertext enc_zero = encrypt_int(0);
    
    int start_noise = nbudget(ct);
    std::cout << "Start noise: " << start_noise << " bits\n";
    std::cout << "Value: " << decrypt_int(ct) << "\n\n";
    
    constexpr int TOTAL = 1000000;
    constexpr int CHECKPOINT = 100000;  // Report every 100K
    
    std::cout << "┌──────────┬──────────┬──────────┬──────────────┬──────────┐\n";
    std::cout << "│ Ops      │ Noise    │ Drift    │ Drift/op     │ Value    │\n";
    std::cout << "├──────────┼──────────┼──────────┼──────────────┼──────────┤\n";
    printf("│ %5d     │ %3d bits │ —        │ —            │ %8lu │\n", 
           0, start_noise, decrypt_int(ct));
    
    auto total_start = high_resolution_clock::now();
    int prev_noise = start_noise;
    int prev_ops = 0;
    
    for (int checkpoint = 1; checkpoint <= 10; checkpoint++) {
        int target = checkpoint * CHECKPOINT;
        
        auto seg_start = high_resolution_clock::now();
        
        for (int i = prev_ops; i < target; i++) {
            evaluator.add_inplace(ct, enc_zero);
        }
        
        auto seg_end = high_resolution_clock::now();
        
        int noise = nbudget(ct);
        uint64_t val = decrypt_int(ct);
        int drift = prev_noise - noise;
        double drift_per_op = (double)drift / (target - prev_ops);
        double seg_sec = duration<double>(seg_end - seg_start).count();
        double tps = (target - prev_ops) / seg_sec;
        
        printf("│ %5d     │ %3d bits │ %+4d     │ %10.6f    │ %8lu │\n", 
               target, noise, drift, drift_per_op, val);
        
        std::cout << "  └─ Segment: " << (target - prev_ops) << " ops in " 
                  << seg_sec << "s (" << (int)tps << " TPS)\n";
        
        prev_noise = noise;
        prev_ops = target;
        
        if (noise < 5) {
            std::cout << "\n⚠️  NOISE DEPLETED at " << target << " ops!\n";
            break;
        }
        if (val != 42) {
            std::cout << "\n❌ VALUE CORRUPTED at " << target << " ops! (got " << val << ")\n";
            break;
        }
    }
    
    auto total_end = high_resolution_clock::now();
    double total_sec = duration<double>(total_end - total_start).count();
    
    std::cout << "└──────────┴──────────┴──────────┴──────────────┴──────────┘\n\n";
    
    int final_noise = nbudget(ct);
    int total_drift = start_noise - final_noise;
    double avg_drift = (double)total_drift / prev_ops;
    uint64_t final_val = decrypt_int(ct);
    
    std::cout << "═══════════════════════════════════════════════\n";
    std::cout << "  FINAL RESULTS\n";
    std::cout << "═══════════════════════════════════════════════\n";
    printf("  Operations completed: %d\n", prev_ops);
    printf("  Start noise:          %d bits\n", start_noise);
    printf("  Final noise:          %d bits\n", final_noise);
    printf("  Total drift:          %d bits\n", total_drift);
    printf("  Average drift/op:     %.8f bits\n", avg_drift);
    printf("  Total time:           %.2f sec\n", total_sec);
    printf("  Average throughput:   %.0f ops/sec\n", prev_ops / total_sec);
    printf("  Final value:          %lu (expected 42)\n", final_val);
    printf("  Value preserved:      %s\n", final_val == 42 ? "YES ✅" : "NO ❌");
    
    double theoretical_drift = prev_ops * 1.0;  // 1 bit/op theoretical
    double improvement = theoretical_drift / total_drift;
    printf("  Improvement:          %.0f× over theoretical (~1 bit/op)\n", improvement);
    
    // Project how many more ops until noise depleted
    int remaining_noise = final_noise - 10;  // 10-bit safety margin
    double projected_ops = remaining_noise / avg_drift;
    printf("  Remaining noise:      %d bits\n", remaining_noise);
    printf("  Projected max ops:    %.0f\n", prev_ops + projected_ops);
    
    std::cout << "\n  STATUS: " << (prev_ops >= TOTAL ? "🎯 1 MILLION COMPLETE!" : "⚠️  Depleted before 1M") << "\n";
    std::cout << "═══════════════════════════════════════════════\n";
    
    return 0;
}
