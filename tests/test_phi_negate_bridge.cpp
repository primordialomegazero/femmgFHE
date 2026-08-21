// φ-NEGATE BRIDGE — 0-Level Transition
// Ang φ·ψ = -1 ay nagbibigay ng bridge sa pamamagitan ng SIGN FLIP
// Sa CKKS, ang sign flip ay EvalNegate — 0 LEVEL COST!
//
// ANG KEY:
// Hindi multiplication ang kailangan para sa bridge.
// Ang -1 ay sign flip — na libre sa CKKS!
//
// CYCLE:
// Gate (φ-chain) → EvalNegate → Gate (ψ-chain) → EvalNegate → ...

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
    std::cout << "  φ-NEGATE BRIDGE — 0-LEVEL\n";
    std::cout << "  Sign Flip Bilang Transition\n";
    std::cout << "========================================\n\n";

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(30);
    params.SetScalingModSize(40);
    params.SetBatchSize(256);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(params);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);

    auto slots = cc->GetEncodingParams()->GetBatchSize();

    auto make_ct = [&](double val) {
        std::vector<std::complex<double>> vec(slots, {0.0, 0.0});
        vec[0] = {val, 0.0};
        return cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec));
    };

    auto decrypt_val = [&](auto ct) {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetCKKSPackedValue()[0].real();
    };

    auto ct_one = make_ct(1.0);
    auto nand_op = [&](auto a, auto b) {
        auto prod = cc->EvalMult(a, b);  // 1 level
        return cc->EvalSub(ct_one, prod); // 0 level
    };

    std::cout << "NEGATE BRIDGE TEST:\n";
    std::cout << "===================\n\n";

    auto current = make_ct(1.0);
    int errors = 0;
    int total_gates = 30;

    auto t_start = high_resolution_clock::now();

    for (int gate = 0; gate < total_gates; gate++) {
        current = nand_op(current, current);

        // Negate bridge: sign flip (0 level!)
        if (gate % 2 == 1) {
            current = cc->EvalNegate(current);
        }

        double v = decrypt_val(current);
        int expected = (gate % 2 == 0) ? 0 : 1;
        int got = (std::abs(v - (expected ? 1.0 : 0.0)) < 0.5) ? expected : (1 - expected);
        if (got != expected) errors++;

        if (gate < 5 || gate >= total_gates - 3) {
            auto t_now = high_resolution_clock::now();
            auto elapsed = duration_cast<seconds>(t_now - t_start).count();
            std::cout << "  Gate " << gate << ": v=" << v
                      << " exp=" << expected
                      << " level=" << current->GetLevel()
                      << (got == expected ? " ✓" : " ✗")
                      << " (" << elapsed << "s)\n";
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
    std::cout << "  Level: 1 per gate (negate ay 0 level!)\n";
    std::cout << "========================================\n";

    return 0;
}
