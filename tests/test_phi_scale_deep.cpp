// φ-SCALE DEEP CHAIN — 30 GATES
// Depth 30, 2 mults per gate = 60 mults total
// Ito ang boundary ng depth 30

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
    std::cout << "  φ-SCALE DEEP CHAIN — 30 GATES\n";
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

    const double phi = 1.6180339887498948482;
    auto ct_phi = make_ct(phi);
    auto ct_psi = make_ct(1.0 / phi);
    auto ct_zero = make_ct(0.0);

    // NAND sa φ-scale: NAND(a,b) = φ - a·b·ψ
    auto nand_phi_scale = [&](auto a, auto b) {
        auto prod = cc->EvalMult(a, b);           // a·b
        auto scaled = cc->EvalMult(prod, ct_psi); // a·b·ψ
        return cc->EvalSub(ct_phi, scaled);       // φ - a·b·ψ
    };

    auto current = ct_phi;
    int errors = 0;
    int total_gates = 30;

    std::cout << "30 GATES — 2 MULTS EACH\n";
    std::cout << "========================\n\n";

    auto t_start = high_resolution_clock::now();

    for (int gate = 0; gate < total_gates; gate++) {
        current = nand_phi_scale(current, current);

        double v = decrypt_val(current);
        double expected = (gate % 2 == 0) ? 0.0 : phi;
        bool ok = (std::abs(v - expected) < 0.15 * phi);
        if (!ok) errors++;

        if (gate % 5 == 0 || !ok) {
            auto t_now = high_resolution_clock::now();
            auto elapsed = duration_cast<seconds>(t_now - t_start).count();
            std::cout << "  Gate " << gate << ": v=" << v
                      << " exp=" << expected
                      << " elapsed=" << elapsed << "s"
                      << (ok ? " ✓" : " ✗") << "\n";
        }
    }

    auto t_end = high_resolution_clock::now();
    auto total_ms = duration_cast<milliseconds>(t_end - t_start).count();

    std::cout << "\n========================================\n";
    std::cout << "  RESULT:\n";
    std::cout << "  Gates: " << total_gates << "\n";
    std::cout << "  Multiplications: " << total_gates * 2 << "\n";
    std::cout << "  Errors: " << errors << "\n";
    std::cout << "  Accuracy: " << (100.0 * (total_gates - errors) / total_gates) << "%\n";
    std::cout << "  Time: " << total_ms / 1000.0 << "s\n";
    std::cout << "  Per gate: " << (double)total_ms / total_gates << " ms\n";
    std::cout << "========================================\n";

    return 0;
}
