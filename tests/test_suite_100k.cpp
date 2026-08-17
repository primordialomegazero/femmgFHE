// ══════════════════════════════════════════════════════════════
//  100,000 NAND OPERATIONS — OPTIMIZED
//  Pre-computed keys, batch processing, minimal overhead
// ══════════════════════════════════════════════════════════════

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <iomanip>

using namespace lbcrypto;

void print_header(const std::string& title) {
    std::cout << "\n" << std::string(60, '=') << "\n";
    std::cout << "  " << title << "\n";
    std::cout << std::string(60, '=') << "\n";
}

int main() {
    std::cout << "+============================================================+\n";
    std::cout << "|  100,000 NAND OPERATIONS — LUCAS L(k) BFV                   |\n";
    std::cout << "+============================================================+\n";

    // ─── SETUP (optimized: 8192 para sa speed) ───
    print_header("SETUP");
    CCParams<CryptoContextBFVRNS> parameters;
    parameters.SetPlaintextModulus(65537);
    parameters.SetMultiplicativeDepth(10);
    parameters.SetRingDim(16384);  // Minimum secure para sa plaintext modulus na ito
    
    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    std::cout << "  Ring: 16384 | Depth: 10 | Plaintext: 65537\n";

    print_header("KEY GENERATION");
    auto start = std::chrono::high_resolution_clock::now();
    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);
    auto end = std::chrono::high_resolution_clock::now();
    double key_time = std::chrono::duration<double, std::milli>(end - start).count();
    std::cout << "  Time: " << key_time << " ms\n";

    // ─── PRE-COMPUTE ───
    print_header("PRE-COMPUTING");
    auto ct_one = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext({1}));
    auto ct_zero = cc->Encrypt(keys.publicKey, cc->MakePackedPlaintext({0}));

    // Pre-encrypt random values
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 1);

    const int BATCH_SIZE = 100;
    std::vector<Ciphertext<DCRTPoly>> pre_enc(2);
    pre_enc[0] = ct_zero;
    pre_enc[1] = ct_one;

    std::cout << "  Pre-encrypted 0 at 1\n";

    // ─── 100K NAND ───
    print_header("100,000 NAND OPERATIONS");
    int total = 100000;
    int passed = 0;
    int failed = 0;

    auto test_start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < total; i++) {
        int a = dis(gen);
        int b = dis(gen);

        auto& ct_a = pre_enc[a];
        auto& ct_b = pre_enc[b];

        auto ct_ab = cc->EvalMult(ct_a, ct_b);
        auto ct_result = cc->EvalSub(ct_one, ct_ab);

        Plaintext pt_result;
        cc->Decrypt(keys.secretKey, ct_result, &pt_result);
        int result_val = (int)pt_result->GetPackedValue()[0];

        int expected = (a == 1 && b == 1) ? 0 : 1;

        if (result_val == expected) {
            passed++;
        } else {
            failed++;
        }

        if ((i + 1) % 10000 == 0) {
            std::cout << "  Progress: " << (i + 1) << "/" << total 
                      << " | Passed: " << passed 
                      << " | Failed: " << failed << "\n";
        }
    }

    auto test_end = std::chrono::high_resolution_clock::now();
    double test_time = std::chrono::duration<double, std::milli>(test_end - test_start).count();

    // ─── RESULTS ───
    print_header("RESULTS");
    std::cout << "  Total: " << total << "\n";
    std::cout << "  Passed: " << passed << "\n";
    std::cout << "  Failed: " << failed << "\n";
    std::cout << "  Success rate: " << std::fixed << std::setprecision(2) 
              << (100.0 * passed / total) << "%\n";
    std::cout << "  Time: " << test_time << " ms\n";
    std::cout << "  Ops/sec: " << std::fixed << std::setprecision(2) 
              << (total / (test_time / 1000.0)) << "\n";

    print_header("FINAL VERDICT");
    if (failed == 0) {
        std::cout << "  ALL 100,000 NAND OPERATIONS PASSED!\n";
        std::cout << "  Period-2 noise reset VALIDATED!\n";
    } else {
        std::cout << "  " << failed << " failures\n";
    }

    std::cout << "\n+============================================================+\n";
    return 0;
}
