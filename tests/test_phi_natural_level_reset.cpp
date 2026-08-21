// φ-NATURAL LEVEL RESET
// Ang φ^k at ψ^k bilang natural na level regenerator
//
// ANG KEY: 
// φ^k · ψ^k = 1 — identity
// Pero ang SEQUENCE ng multiplication ay nagbibigay ng
// natural na noise reduction:
//
// ct → ct·ψ^k → ct·ψ^k·φ^k = ct
//      (noise↓)    (identity)
//
// Kung ang ψ^k multiplication ay nagre-reduce ng noise
// at ang φ^k multiplication ay nagba-balik ng scale,
// ito ay natural na bootstrapping!

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  φ-NATURAL LEVEL RESET\n";
    std::cout << "  ψ^k → φ^k Sequence\n";
    std::cout << "========================================\n\n";

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(30);
    params.SetScalingModSize(50);  // Mas mataas para sa precision
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

    // φ at ψ constants — k=5 para sa manageable values
    const double phi = 1.6180339887498948482;
    const double psi_k = std::pow(1.0/phi, 5.0);  // ψ⁵ ≈ 0.090
    const double phi_k = std::pow(phi, 5.0);       // φ⁵ ≈ 11.09

    auto ct_psi_k = make_ct(psi_k);
    auto ct_phi_k = make_ct(phi_k);

    auto ct_one = make_ct(1.0);
    auto nand_op = [&](auto a, auto b) {
        auto prod = cc->EvalMult(a, b);
        return cc->EvalSub(ct_one, prod);
    };

    // ANG NATURAL RESET:
    // refresh(ct) = (ct · ψ^k) · φ^k
    auto natural_reset = [&](auto ct) {
        auto reduced = cc->EvalMult(ct, ct_psi_k);   // Noise reduction
        return cc->EvalMult(reduced, ct_phi_k);       // Scale restore
    };

    std::cout << "ψ⁵ = " << psi_k << ", φ⁵ = " << phi_k << "\n";
    std::cout << "ψ⁵ · φ⁵ = " << psi_k * phi_k << " (dapat 1)\n\n";

    std::cout << "TEST: 100 GATES NA MAY NATURAL RESET\n";
    std::cout << "=====================================\n\n";

    auto current = make_ct(1.0);
    int errors = 0;
    int total_gates = 100;
    int reset_count = 0;

    auto t_start = high_resolution_clock::now();

    for (int gate = 0; gate < total_gates; gate++) {
        current = nand_op(current, current);

        // Natural reset every 10 gates
        if (gate > 0 && gate % 10 == 0) {
            current = natural_reset(current);
            reset_count++;
        }

        double v = decrypt_val(current);
        int expected = (gate % 2 == 0) ? 0 : 1;
        int got = (std::abs(v - expected) < 0.5) ? expected : (1 - expected);

        if (got != expected) errors++;

        if (gate % 20 == 0 || gate >= total_gates - 3) {
            auto t_now = high_resolution_clock::now();
            auto elapsed = duration_cast<seconds>(t_now - t_start).count();
            std::cout << "  [" << gate << "/" << total_gates << "] "
                      << "v=" << v
                      << " level=" << current->GetLevel()
                      << " errors=" << errors
                      << " (" << elapsed << "s)\n";
            std::cout.flush();
        }
    }

    auto t_end = high_resolution_clock::now();
    auto total_ms = duration_cast<milliseconds>(t_end - t_start).count();

    std::cout << "\n========================================\n";
    std::cout << "  RESULT:\n";
    std::cout << "  Gates: " << total_gates << "\n";
    std::cout << "  Natural Resets: " << reset_count << "\n";
    std::cout << "  Errors: " << errors << "\n";
    std::cout << "  Accuracy: " << (100.0 * (total_gates - errors) / total_gates) << "%\n";
    std::cout << "  Time: " << total_ms / 1000.0 << "s\n";
    std::cout << "========================================\n";

    return 0;
}
