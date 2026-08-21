// φ-SUBTRACTION REFRESH — DEEP CHAIN
// Tayo'y gagawa ng 200-gate NAND_φ chain
// Tuwing 20 gates, gagawa tayo ng φ-refresh (subtraction lang)
// Kung zero-cost nga ang refresh, aabot tayo ng 200 nang walang
// bootstrapping at walang depth error.

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  φ-SUBTRACTION REFRESH — DEEP CHAIN\n";
    std::cout << "========================================\n\n";

    // Depth 60, para sa 20 gates bago refresh
    // May refresh every 20 gates
    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(60);
    params.SetScalingModSize(40);
    params.SetBatchSize(256);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(params);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);

    auto slots = cc->GetEncodingParams()->GetBatchSize();
    auto ring = cc->GetRingDimension();
    std::cout << "Ring: " << ring << ", Slots: " << slots << "\n\n";

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

    // Constants
    const double PHI = (1.0 + std::sqrt(5.0)) / 2.0;  // 1.61803
    const double PSI = PHI - 1.0;                     // 0.618034

    auto ct_phi = make_ct(PHI);
    auto ct_psi = make_ct(PSI);
    auto ct_one = make_ct(1.0);

    // NAND_φ(a,b) = φ - (a·b)
    // Gagamit tayo ng 1 multiplication + subtraction
    auto nand_phi = [&](auto a, auto b) {
        auto prod = cc->EvalMult(a, b);    // 1 multiplication
        return cc->EvalSub(ct_phi, prod);  // subtraction (free)
    };

    // φ-refresh: c → c - ψ (subtraction only)
    // Theory: pagkatapos ng NAND_φ, ang value ay nasa φ-domain
    // Ang pag-subtract ng ψ ay nagbabalik sa tamang scale
    auto phi_refresh = [&](auto c) {
        return cc->EvalSub(c, ct_psi);  // subtraction (free)
    };

    // Initial value
    auto current = ct_phi;
    int errors = 0;
    int total_gates = 200;
    int refresh_interval = 20;

    std::cout << "200-GATE NAND_φ CHAIN\n";
    std::cout << "Refresh every " << refresh_interval << " gates\n\n";

    auto t_start = high_resolution_clock::now();

    for (int gate = 0; gate < total_gates; gate++) {
        // NAND_φ(current, current)
        auto current_copy = make_ct(decrypt_val(current));
        current = nand_phi(current, current_copy);

        // Refresh tuwing 20 gates
        if ((gate + 1) % refresh_interval == 0) {
            current = phi_refresh(current);
        }

        // Verify
        double got = decrypt_val(current);
        double expected = (gate % 2 == 0) ? 0.0 : PHI;
        if (std::abs(got - expected) > 0.1) errors++;

        if (gate % 20 == 0) {
            auto t_now = high_resolution_clock::now();
            auto elapsed = duration_cast<seconds>(t_now - t_start).count();
            std::cout << "  Gate " << gate << ": val=" << got
                      << " expected=" << expected
                      << " elapsed=" << elapsed << "s"
                      << " errors=" << errors << "\n";
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
    std::cout << "  Per gate: " << (double)total_ms / total_gates << " ms\n";
    std::cout << "========================================\n";

    return 0;
}
