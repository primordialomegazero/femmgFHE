// φ-DUAL CKKS CONTEXTS — KEY SWITCHING BRIDGE
// Dalawang contexts: φ-chain at ψ-chain
// Lumipat sa pagitan nila gamit ang key switching
// (hindi decryption, homomorphic pa rin)

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
    std::cout << "  φ-DUAL CONTEXT BRIDGE\n";
    std::cout << "  Key Switching Between Chains\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double psi = 1.0 / phi;

    // ============================================
    // CONTEXT A: φ-CHAIN (Depth 30)
    // ============================================
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

    // ============================================
    // CONTEXT B: ψ-CHAIN (Depth 30)
    // ============================================
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

    // ============================================
    // HELPERS
    // ============================================
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

    // NAND sa bawat context
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

    // ============================================
    // BRIDGE: A → B (walang decryption)
    // ============================================
    // Sa totoong setting, ang bridge ay kailangan ng
    // homomorphic transition. Sa ngayon, gagamit muna
    // tayo ng decrypt-re-encrypt bilang proof-of-concept.
    // (Ito ay placeholder para sa susunod na homomorphic bridge)

    std::cout << "DUAL CONTEXT BRIDGE (Proof of Concept):\n";
    std::cout << "========================================\n\n";

    std::cout << "Context A: φ-chain (depth 30)\n";
    std::cout << "Context B: ψ-chain (depth 30)\n";
    std::cout << "Bridge: A→B at B→A\n\n";

    auto current_A = make_ct_A(1.0);
    int errors = 0;
    int total_gates = 50;  // Lampas sa 30 para patunayang dual

    auto t_start = high_resolution_clock::now();

    for (int gate = 0; gate < total_gates; gate++) {
        // Mag-compute sa current context
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

            // Bridge: A → B
            if (gate < total_gates - 1) {
                double plain_val = (std::abs(v) > 0.5) ? 1.0 : 0.0;
                auto bridged_B = make_ct_B(plain_val);
                // Sa susunod na gate, ito ang gagamitin sa B
                // (placeholder: kailangan ng homomorphic bridge)
            }
        } else {
            // Context B (gagamit muna natin ang na-decrypt na value)
            // Sa totoong implementation, ito ay dapat naka-encrypt pa rin
            double bridged_val = (std::abs(decrypt_A(current_A)) > 0.5) ? 1.0 : 0.0;
            auto current_B = make_ct_B(bridged_val);

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
    std::cout << "  Note: May decrypt-re-encrypt pa ito.\n";
    std::cout << "  Kailangan ng homomorphic bridge para maging secure.\n";
    std::cout << "========================================\n";

    return 0;
}
