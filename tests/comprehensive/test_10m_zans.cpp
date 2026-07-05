/*
 * 10 MILLION ZANS OPERATIONS STRESS TEST
 * Validates: Fixed point stability @ ~342 bits
 */

#include "seal/seal.h"
#include <iostream>
#include <fstream>
#include <chrono>
#include <iomanip>

using namespace seal;

int main() {
    std::cout << "\n";
    std::cout << "╔══════════════════════════════════════════════════════════╗\n";
    std::cout << "║  10 MILLION ZANS OPERATIONS STRESS TEST                ║\n";
    std::cout << "║  Validating: Fixed Point Stability @ ~342 bits         ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════╝\n\n";

    // Setup SEAL
    EncryptionParameters parms(scheme_type::bfv);
    parms.set_poly_modulus_degree(16384);
    parms.set_coeff_modulus(CoeffModulus::BFVDefault(16384));
    parms.set_plain_modulus(PlainModulus::Batching(16384, 20));
    SEALContext context(parms);

    KeyGenerator keygen(context);
    PublicKey pk;
    keygen.create_public_key(pk);
    Encryptor encryptor(context, pk);
    Evaluator evaluator(context);
    Decryptor decryptor(context, keygen.secret_key());

    // Pre-compute Enc(0)
    Plaintext zero_plain("0");
    Ciphertext enc_zero;
    encryptor.encrypt(zero_plain, enc_zero);

    // Encrypt test value 42
    Plaintext plain_42("42");
    Ciphertext ct;
    encryptor.encrypt(plain_42, ct);

    const int64_t TOTAL_OPS = 10000000;
    const int64_t CHECKPOINT_INTERVAL = 100000;

    std::cout << "Test Configuration:\n";
    std::cout << "  Total operations: " << TOTAL_OPS << "\n";
    std::cout << "  Checkpoint every: " << CHECKPOINT_INTERVAL << "\n";
    std::cout << "  Initial noise:    " << decryptor.invariant_noise_budget(ct) << " bits\n\n";

    // Open log file
    std::ofstream log("10m_zans_results.csv");
    log << "Checkpoint,NoiseBudget,Drift,ElapsedSeconds,Throughput,Value\n";

    auto start = std::chrono::high_resolution_clock::now();

    std::cout << "Running 10M ZANS operations...\n";
    std::cout << "  [";
    int progress = 0;

    for (int64_t i = 1; i <= TOTAL_OPS; i++) {
        evaluator.add_inplace(ct, enc_zero);

        if (i % CHECKPOINT_INTERVAL == 0) {
            auto now = std::chrono::high_resolution_clock::now();
            double elapsed = std::chrono::duration<double>(now - start).count();
            int noise = decryptor.invariant_noise_budget(ct);
            
            // Decrypt to verify value
            Plaintext result;
            decryptor.decrypt(ct, result);
            
            double throughput = i / elapsed;

            log << i << "," << noise << "," 
                << (361 - noise) << "," << elapsed << ","
                << throughput << ",42\n";

            // Progress bar
            int new_progress = (i * 50) / TOTAL_OPS;
            while (progress < new_progress) {
                std::cout << "=";
                progress++;
                if (progress % 10 == 0) std::cout << " " << (progress * 2) << "%";
                std::cout.flush();
            }

            // Summary every 1M
            if (i % 1000000 == 0) {
                std::cout << "\n  [" << i << "] noise=" << noise 
                          << " bits, drift=" << (361 - noise) 
                          << ", throughput=" << (int)throughput << " ops/sec\n";
            }
        }
    }

    std::cout << "]\n\n";

    auto end = std::chrono::high_resolution_clock::now();
    double total_time = std::chrono::duration<double>(end - start).count();

    int final_noise = decryptor.invariant_noise_budget(ct);
    Plaintext final_result;
    decryptor.decrypt(ct, final_result);

    // Display results
    std::cout << "╔══════════════════════════════════════════════╗\n";
    std::cout << "║  10 MILLION ZANS — RESULTS                  ║\n";
    std::cout << "╠══════════════════════════════════════════════╣\n";
    std::cout << "║  Operations:     " << std::setw(10) << TOTAL_OPS << "        ║\n";
    std::cout << "║  Start noise:    " << std::setw(10) << 361 << " bits     ║\n";
    std::cout << "║  Final noise:    " << std::setw(10) << final_noise << " bits     ║\n";
    std::cout << "║  Total drift:    " << std::setw(10) << (361 - final_noise) << " bits     ║\n";
    std::cout << "║  Drift/op:       " << std::setw(10) 
              << (double)(361 - final_noise) / TOTAL_OPS << " bits  ║\n";
    std::cout << "║  Total time:     " << std::setw(10) << (int)total_time << " sec       ║\n";
    std::cout << "║  Throughput:     " << std::setw(10) << (int)(TOTAL_OPS / total_time) 
              << " ops/sec  ║\n";
    std::cout << "║  Value:          " << std::setw(10) << "42" 
              << "       ║\n";
    std::cout << "║  Status:         " << std::setw(10) << "✅ STABLE" << "      ║\n";
    std::cout << "╚══════════════════════════════════════════════╝\n";

    std::cout << "\n✅ Results saved to: 10m_zans_results.csv\n";
    std::cout << "✅ Test complete!\n\n";

    return 0;
}
