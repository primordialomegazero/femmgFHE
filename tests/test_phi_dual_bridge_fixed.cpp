// φ-DUAL CONTEXT BRIDGE — FIXED
// Tamang state management sa pagitan ng φ at ψ chains

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <chrono>
#include <cmath>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  φ-DUAL BRIDGE — FIXED\n";
    std::cout << "  Tamang State Management\n";
    std::cout << "========================================\n\n";

    // Context A (φ-chain)
    CCParams<CryptoContextCKKSRNS> paramsA;
    paramsA.SetMultiplicativeDepth(30);
    paramsA.SetScalingModSize(40);
    paramsA.SetBatchSize(256);

    auto ccA = GenCryptoContext(paramsA);
    ccA->Enable(PKE);
    ccA->Enable(KEYSWITCH);
    ccA->Enable(LEVELEDSHE);

    auto keysA = ccA->KeyGen();
    ccA->EvalMultKeyGen(keysA.secretKey);

    // Context B (ψ-chain)
    CCParams<CryptoContextCKKSRNS> paramsB;
    paramsB.SetMultiplicativeDepth(30);
    paramsB.SetScalingModSize(40);
    paramsB.SetBatchSize(256);

    auto ccB = GenCryptoContext(paramsB);
    ccB->Enable(PKE);
    ccB->Enable(KEYSWITCH);
    ccB->Enable(LEVELEDSHE);

    auto keysB = ccB->KeyGen();
    ccB->EvalMultKeyGen(keysB.secretKey);

    auto slots = ccA->GetEncodingParams()->GetBatchSize();

    auto make_ct_A = [&](double val) {
        std::vector<std::complex<double>> vec(slots, {0.0, 0.0});
        vec[0] = {val, 0.0};
        return ccA->Encrypt(keysA.publicKey, ccA->MakeCKKSPackedPlaintext(vec));
    };

    auto make_ct_B = [&](double val) {
        std::vector<std::complex<double>> vec(slots, {0.0, 0.0});
        vec[0] = {val, 0.0};
        return ccB->Encrypt(keysB.publicKey, ccB->MakeCKKSPackedPlaintext(vec));
    };

    auto decrypt_A = [&](auto ct) {
        Plaintext pt;
        ccA->Decrypt(keysA.secretKey, ct, &pt);
        return pt->GetCKKSPackedValue()[0].real();
    };

    auto decrypt_B = [&](auto ct) {
        Plaintext pt;
        ccB->Decrypt(keysB.secretKey, ct, &pt);
        return pt->GetCKKSPackedValue()[0].real();
    };

    auto ct_one_A = make_ct_A(1.0);
    auto ct_one_B = make_ct_B(1.0);

    auto nand_A = [&](auto a, auto b) {
        auto prod = ccA->EvalMult(a, b);
        return ccA->EvalSub(ct_one_A, prod);
    };

    auto nand_B = [&](auto a, auto b) {
        auto prod = ccB->EvalMult(a, b);
        return ccB->EvalSub(ct_one_B, prod);
    };

    // Tamang bridge: decrypt + re-encrypt + state sync
    auto bridge_A_to_B = [&](auto ct_A) {
        double v = decrypt_A(ct_A);
        int bit = (std::abs(v) > 0.5) ? 1 : 0;
        return std::make_pair(make_ct_B(bit), bit);
    };

    auto bridge_B_to_A = [&](auto ct_B) {
        double v = decrypt_B(ct_B);
        int bit = (std::abs(v) > 0.5) ? 1 : 0;
        return std::make_pair(make_ct_A(bit), bit);
    };

    std::cout << "50 GATES — ALTERNATING A/B\n";
    std::cout << "============================\n\n";

    auto current_A = make_ct_A(1.0);
    auto current_B = make_ct_B(1.0);
    int errors = 0;
    int total_gates = 50;

    auto t_start = high_resolution_clock::now();

    for (int gate = 0; gate < total_gates; gate++) {
        if (gate % 2 == 0) {
            // Context A
            current_A = nand_A(current_A, current_A);
            double v = decrypt_A(current_A);
            int expected = (gate % 2 == 0) ? 0 : 1;
            int got = (std::abs(v) > 0.5) ? 1 : 0;
            if (got != expected) errors++;

            if (gate % 10 == 0 || gate >= total_gates - 3) {
                std::cout << "  Gate " << gate << " [A]: v=" << v
                          << " exp=" << expected << " level=" << current_A->GetLevel()
                          << (got == expected ? " ✓" : " ✗") << "\n";
            }

            // Bridge to B
            if (gate < total_gates - 1) {
                auto bridged = bridge_A_to_B(current_A);
                current_B = bridged.first;
            }
        } else {
            // Context B
            current_B = nand_B(current_B, current_B);
            double v = decrypt_B(current_B);
            int expected = (gate % 2 == 0) ? 0 : 1;
            int got = (std::abs(v) > 0.5) ? 1 : 0;
            if (got != expected) errors++;

            if (gate % 10 == 0 || gate >= total_gates - 3) {
                std::cout << "  Gate " << gate << " [B]: v=" << v
                          << " exp=" << expected << " level=" << current_B->GetLevel()
                          << (got == expected ? " ✓" : " ✗") << "\n";
            }

            // Bridge to A
            if (gate < total_gates - 1) {
                auto bridged = bridge_B_to_A(current_B);
                current_A = bridged.first;
            }
        }
    }

    auto t_end = high_resolution_clock::now();
    auto total_ms = duration_cast<milliseconds>(t_end - t_start).count();

    std::cout << "\n========================================\n";
    std::cout << "  RESULT:\n";
    std::cout << "  Gates: " << total_gates << "\n";
    std::cout << "  Errors: " << errors << "\n";
    std::cout << "  Accuracy: " << (100.0 * (total_gates - errors) / total_gates) << "%\n";
    std::cout << "  Time: " << total_ms / 1000.0 << "s\n";
    std::cout << "========================================\n";

    return 0;
}
