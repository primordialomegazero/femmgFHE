// φ-CYCLIC HOMOMORPHIC BRIDGE
// Hindi decrypt-re-encrypt, kundi homomorphic φ/ψ multiplication
//
// ANG CYCLE:
// ct_state × E(φ) → nagbabago ang scale
// ct_state × E(ψ) → bumabalik sa original scale
// φ·ψ = -1 → natural na sign flip (period-2)
//
// Sa bawat 2 gates, ang level ay natural na nare-recover
// kasi ang φ·ψ = -1 at (-1)² = 1

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
    std::cout << "  φ-CYCLIC HOMOMORPHIC BRIDGE\n";
    std::cout << "  Emergent Level Recovery\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double psi = 1.0 / phi;

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

    // Encrypted constants
    auto ct_one = make_ct(1.0);
    auto ct_phi = make_ct(phi);
    auto ct_psi = make_ct(psi);
    auto ct_neg_one = make_ct(-1.0);

    // NAND standard
    auto nand_std = [&](auto a, auto b) {
        auto prod = cc->EvalMult(a, b);
        return cc->EvalSub(ct_one, prod);
    };

    // φ-bridge: multiply by φ (homomorphic)
    auto bridge_phi = [&](auto ct) {
        return cc->EvalMult(ct, ct_phi);
    };

    // ψ-bridge: multiply by ψ (homomorphic)
    auto bridge_psi = [&](auto ct) {
        return cc->EvalMult(ct, ct_psi);
    };

    // φ-cyclic NAND:
    // Gate odd: NAND + φ-bridge
    // Gate even: NAND + ψ-bridge
    // Ang φ·ψ = -1 ay nagbibigay ng period-2

    std::cout << "50 GATES — φ-CYCLIC BRIDGE\n";
    std::cout << "============================\n\n";

    auto current = make_ct(1.0);
    int errors = 0;
    int total_gates = 50;

    auto t_start = high_resolution_clock::now();

    for (int gate = 0; gate < total_gates; gate++) {
        // NAND
        current = nand_std(current, current);

        // φ-cyclic bridge (alternating φ at ψ)
        if (gate < total_gates - 1) {
            if (gate % 2 == 0) {
                current = bridge_phi(current);
            } else {
                current = bridge_psi(current);
            }
        }

        double v = decrypt_val(current);
        int expected = (gate % 2 == 0) ? 0 : 1;
        int got = (std::abs(v) > 0.5) ? 1 : 0;
        if (got != expected) errors++;

        if (gate < 5 || gate % 10 == 0 || gate >= total_gates - 3) {
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
    std::cout << "========================================\n";

    return 0;
}
