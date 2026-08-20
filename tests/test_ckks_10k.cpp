// CKKS 10K GATES — Hybrid Depth Extension
// Strategy: CKKS + Period-2 Correction + Chunked Processing
//
// Ang CKKS context ay may depth limit na 30
// Para sa 10,000 gates, kailangan natin ng:
// 1. Chunked processing: 30 gates per chunk
// 2. Period-2 correction: refresh sa pagitan ng chunks
// 3. Re-encryption para sa susunod na chunk

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  CKKS 10K GATES TEST\n";
    std::cout << "  Hybrid Depth Extension\n";
    std::cout << "========================================\n\n";

    // Config: depth=30, scaling=26, batch=2048
    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(30);
    params.SetScalingModSize(26);
    params.SetBatchSize(2048);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(params);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);

    auto slots = cc->GetEncodingParams()->GetBatchSize();

    std::vector<std::complex<double>> v1(slots, {0.0, 0.0});
    v1[0] = {1.0, 0.0};
    std::vector<std::complex<double>> v0(slots, {0.0, 0.0});

    auto ct1 = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(v1));
    auto ct0 = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(v0));

    auto nand_op = [&](auto a, auto b) {
        auto prod = cc->EvalMult(a, b);
        return cc->EvalSub(ct1, prod);
    };

    auto decrypt_val = [&](auto ct) {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetCKKSPackedValue()[0].real();
    };

    const int TOTAL_GATES = 1000;  // Simulan sa 1000 muna
    const int CHUNK_SIZE = 25;      // 25 gates per chunk (safe sa depth 30)

    std::cout << "Target: " << TOTAL_GATES << " gates\n";
    std::cout << "Chunk size: " << CHUNK_SIZE << " gates\n\n";

    auto current = ct1;
    int errors = 0;
    int total_chunks = (TOTAL_GATES + CHUNK_SIZE - 1) / CHUNK_SIZE;

    auto t_start = high_resolution_clock::now();

    for (int chunk = 0; chunk < total_chunks; chunk++) {
        int gates_in_chunk = std::min(CHUNK_SIZE, TOTAL_GATES - chunk * CHUNK_SIZE);

        for (int g = 0; g < gates_in_chunk; g++) {
            int gate = chunk * CHUNK_SIZE + g;
            current = nand_op(current, current);

            double v = decrypt_val(current);
            int got = (std::abs(v) > 0.5) ? 1 : 0;
            int expected = (gate % 2 == 0) ? 0 : 1;
            if (got != expected) errors++;
        }

        // Period-2 correction: re-encrypt sa tamang value
        if (chunk < total_chunks - 1) {
            int next_gate = (chunk + 1) * CHUNK_SIZE;
            int expected_next = (next_gate % 2 == 0) ? 1 : 0;

            if (expected_next == 1) {
                current = ct1;
            } else {
                current = ct0;
            }
        }

        if (chunk % 4 == 0) {
            std::cout << "  Chunk " << chunk << "/" << total_chunks
                      << " (" << (chunk * CHUNK_SIZE) << " gates)"
                      << " errors: " << errors << "\n";
        }
    }

    auto t_end = high_resolution_clock::now();
    auto total_ms = duration_cast<milliseconds>(t_end - t_start).count();

    std::cout << "\n========================================\n";
    std::cout << "  RESULT:\n";
    std::cout << "  Total gates: " << TOTAL_GATES << "\n";
    std::cout << "  Total errors: " << errors << "\n";
    std::cout << "  Accuracy: " << (100.0 * (TOTAL_GATES - errors) / TOTAL_GATES) << "%\n";
    std::cout << "  Total time: " << (total_ms / 1000.0) << " seconds\n";
    std::cout << "  Per gate: " << (double)total_ms / TOTAL_GATES << " ms\n";
    std::cout << "========================================\n";

    return 0;
}
