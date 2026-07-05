#include <iostream>
#include <iomanip>
#include <vector>
#include <chrono>
#include "seal/seal.h"

using namespace seal;
using namespace std::chrono;

int main() {
    std::cout << "╔══════════════════════════════════════════════════════════╗\n";
    std::cout << "║  CRITICAL TEST: Fresh Enc(0) vs Reused Enc(0)          ║\n";
    std::cout << "║  Does ZANS contraction depend on reusing Enc(0)?       ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════╝\n\n";

    size_t N = 16384;
    EncryptionParameters parms(scheme_type::bfv);
    parms.set_poly_modulus_degree(N);
    parms.set_coeff_modulus(CoeffModulus::BFVDefault(N));
    parms.set_plain_modulus(PlainModulus::Batching(N, 20));
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
        std::vector<uint64_t> vec(N, 0ULL);
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
    
    // ═══════════════════════════════════════════════════════
    // TEST A: Reused Enc(0) - 10,000 ops
    // ═══════════════════════════════════════════════════════
    std::cout << "═══ TEST A: Reused Enc(0) — 10,000 ops ═══\n";
    Ciphertext ct_reused = encrypt_int(42);
    Ciphertext enc_zero_reused = encrypt_int(0);  // Generate ONCE
    
    int start_a = nbudget(ct_reused);
    auto t1 = high_resolution_clock::now();
    for (int i = 0; i < 10000; i++) {
        evaluator.add_inplace(ct_reused, enc_zero_reused);
    }
    auto t2 = high_resolution_clock::now();
    int end_a = nbudget(ct_reused);
    double drift_a = (double)(start_a - end_a) / 10000.0;
    double ms_a = duration<double, std::milli>(t2 - t1).count();
    
    std::cout << "  Start: " << start_a << " bits\n";
    std::cout << "  End:   " << end_a << " bits\n";
    std::cout << "  Drift: " << drift_a << " bits/op\n";
    std::cout << "  Value: " << decrypt_int(ct_reused) << " (expected 42)\n";
    std::cout << "  Time:  " << ms_a << " ms\n\n";
    
    // ═══════════════════════════════════════════════════════
    // TEST B: Fresh Enc(0) EVERY OPERATION - 10,000 ops
    // ═══════════════════════════════════════════════════════
    std::cout << "═══ TEST B: Fresh Enc(0) EVERY op — 10,000 ops ═══\n";
    Ciphertext ct_fresh_every = encrypt_int(42);
    
    int start_b = nbudget(ct_fresh_every);
    t1 = high_resolution_clock::now();
    for (int i = 0; i < 10000; i++) {
        Ciphertext fresh_zero = encrypt_int(0);  // Generate FRESH each time
        evaluator.add_inplace(ct_fresh_every, fresh_zero);
    }
    t2 = high_resolution_clock::now();
    int end_b = nbudget(ct_fresh_every);
    double drift_b = (double)(start_b - end_b) / 10000.0;
    double ms_b = duration<double, std::milli>(t2 - t1).count();
    
    std::cout << "  Start: " << start_b << " bits\n";
    std::cout << "  End:   " << end_b << " bits\n";
    std::cout << "  Drift: " << drift_b << " bits/op\n";
    std::cout << "  Value: " << decrypt_int(ct_fresh_every) << " (expected 42)\n";
    std::cout << "  Time:  " << ms_b << " ms (" << ms_b/ms_a << "× slower than reused)\n\n";
    
    // ═══════════════════════════════════════════════════════
    // TEST C: Fresh Enc(0) EVERY 100 ops — 10,000 ops
    // ═══════════════════════════════════════════════════════
    std::cout << "═══ TEST C: Fresh Enc(0) every 100 ops — 10,000 ops ═══\n";
    Ciphertext ct_fresh_100 = encrypt_int(42);
    Ciphertext current_zero = encrypt_int(0);
    
    int start_c = nbudget(ct_fresh_100);
    t1 = high_resolution_clock::now();
    for (int i = 0; i < 10000; i++) {
        if (i % 100 == 0) {
            current_zero = encrypt_int(0);  // Refresh every 100
        }
        evaluator.add_inplace(ct_fresh_100, current_zero);
    }
    t2 = high_resolution_clock::now();
    int end_c = nbudget(ct_fresh_100);
    double drift_c = (double)(start_c - end_c) / 10000.0;
    double ms_c = duration<double, std::milli>(t2 - t1).count();
    
    std::cout << "  Start: " << start_c << " bits\n";
    std::cout << "  End:   " << end_c << " bits\n";
    std::cout << "  Drift: " << drift_c << " bits/op\n";
    std::cout << "  Value: " << decrypt_int(ct_fresh_100) << " (expected 42)\n";
    std::cout << "  Time:  " << ms_c << " ms\n\n";
    
    // ═══════════════════════════════════════════════════════
    // COMPARISON
    // ═══════════════════════════════════════════════════════
    std::cout << "╔══════════════════════════════════════════════════════════╗\n";
    std::cout << "║              FRESH vs REUSED Enc(0) RESULTS              ║\n";
    std::cout << "╠══════════════════════════════════════════════════════════╣\n";
    printf("║  Reused Enc(0):       drift=%.6f bits/op              ║\n", drift_a);
    printf("║  Fresh EVERY op:      drift=%.6f bits/op              ║\n", drift_b);
    printf("║  Fresh every 100 ops: drift=%.6f bits/op              ║\n", drift_c);
    std::cout << "║                                                        ║\n";
    
    bool same = (std::abs(drift_a - drift_b) < 0.001) && (std::abs(drift_a - drift_c) < 0.001);
    std::cout << "║  VERDICT: " << (same ? "IDENTICAL — Contraction is NOT an artifact of reusing Enc(0)" : "DIFFERENT — Contraction depends on Enc(0) reuse") << "  ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════╝\n";
    
    return 0;
}
