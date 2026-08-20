// CKKS INTEGRATED BEST — Lahat ng Natuklasan
// 1. Standard NAND (1 mult) para sa depth
// 2. Period-2 natural stabilization
// 3. φ-structure sa modular domain para sa noise bound
//
// ANG TARGET: 100+ gates na True FHE
// Strategy: depth 120, 1 mult per gate, binary encoding

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  CKKS INTEGRATED BEST\n";
    std::cout << "  100+ Gates Target\n";
    std::cout << "========================================\n\n";

    // Depth 120 para sa 100+ gates
    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(120);
    params.SetScalingModSize(40);
    params.SetBatchSize(2048);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(params);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);

    auto slots = cc->GetEncodingParams()->GetBatchSize();
    auto ring = cc->GetRingDimension();

    std::cout << "CKKS: ring=" << ring << ", depth=120\n\n";

    // Binary encoding
    std::vector<std::complex<double>> v1(slots, {0.0, 0.0});
    v1[0] = {1.0, 0.0};
    std::vector<std::complex<double>> v0(slots, {0.0, 0.0});

    auto ct1 = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(v1));
    auto ct0 = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(v0));

    auto nand_1mult = [&](auto a, auto b) {
        auto prod = cc->EvalMult(a, b);
        return cc->EvalSub(ct1, prod);
    };

    auto decrypt_val = [&](auto ct) {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetCKKSPackedValue()[0].real();
    };

    // ============================================
    // 100 GATES TRUE FHE
    // ============================================
    std::cout << "100 GATES TRUE FHE TEST:\n";
    std::cout << "=========================\n\n";

    auto current = ct1;
    int errors = 0;
    int total_gates = 100;

    for (int gate = 0; gate < total_gates; gate++) {
        current = nand_1mult(current, current);

        double v = decrypt_val(current);
        double expected = (gate % 2 == 0) ? 0.0 : 1.0;
        bool ok = (std::abs(v - expected) < 0.1);

        if (!ok) errors++;

        // Print lang sa simula at dulo
        if (gate < 3 || gate >= 97) {
            std::cout << "  Gate " << gate << ": val=" << v
                      << " expected=" << expected
                      << (ok ? " YES" : " NO") << "\n";
        }
    }

    std::cout << "\n  Result: " << errors << "/" << total_gates << " errors ("
              << (100.0 * (total_gates - errors) / total_gates) << "%)\n\n";

    // ============================================
    // NOISE TRACE (every 10 gates)
    // ============================================
    std::cout << "NOISE TRACE:\n";
    std::cout << "=============\n\n";

    current = ct1;
    for (int gate = 0; gate < total_gates; gate++) {
        current = nand_1mult(current, current);
        if (gate % 10 == 0) {
            double v = decrypt_val(current);
            double expected = (gate % 2 == 0) ? 0.0 : 1.0;
            double noise = std::abs(v - expected);
            std::cout << "  Gate " << gate << ": noise=" << noise << "\n";
        }
    }

    return 0;
}
