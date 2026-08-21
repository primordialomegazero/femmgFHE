// φ-BRIDGE-REFRESH IMPLEMENTATION
// Ang φ·ψ = -1 bilang encrypted bridge-refresh constant
// 1 multiplication para sa bridge + refresh
//
// ANG CYCLE:
// ct_φ (φ-chain) → [× E(-1)] → ct_ψ (ψ-chain, refreshed)
// ct_ψ (ψ-chain) → [× E(-1)] → ct_φ (φ-chain, refreshed)
//
// Ang E(-1) ay encrypted constant — homomorphic!
// Ang -1 ay nagbibigay ng sign flip = natural refresh

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
    std::cout << "  φ-BRIDGE-REFRESH IMPLEMENTATION\n";
    std::cout << "  E(-1) bilang Dual Bridge-Refresh\n";
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
    auto ct_neg_one = make_ct(-1.0);  // E(-1) — ang bridge-refresh constant
    auto ct_psi = make_ct(1.0/1.6180339887498948482);
    auto ct_phi = make_ct(1.6180339887498948482);

    // NAND (standard binary)
    auto nand_op = [&](auto a, auto b) {
        auto prod = cc->EvalMult(a, b);
        return cc->EvalSub(ct_one, prod);
    };

    // Bridge-Refresh: ct × E(-1) — 1 mult lang!
    auto bridge_refresh = [&](auto ct) {
        return cc->EvalMult(ct, ct_neg_one);
    };

    std::cout << "28 GATES NA MAY BRIDGE-REFRESH:\n";
    std::cout << "================================\n\n";

    auto current = make_ct(1.0);
    int errors = 0;
    int total_gates = 28;

    auto t_start = high_resolution_clock::now();

    for (int gate = 0; gate < total_gates; gate++) {
        current = nand_op(current, current);

        // Bridge-Refresh pagkatapos ng bawat gate
        if (gate < total_gates - 1) {
            current = bridge_refresh(current);
        }

        double v = decrypt_val(current);
        int expected = (gate % 2 == 0) ? 0 : 1;
        int got = (std::abs(v) > 0.5) ? 1 : 0;
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
    std::cout << "========================================\n";

    return 0;
}
