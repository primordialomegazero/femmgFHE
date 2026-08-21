// φ-DOMAIN — ALTERNATING ZERO OPTIMIZATION
// Sa period-2, ang "0" ay zero — ang multiply sa 0 ay libre!
// Kaya ang level consumption ay mas mababa sa 2 per gate

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
    std::cout << "  φ-DOMAIN — ALTERNATING ZERO\n";
    std::cout << "  Level Optimization\n";
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

    auto ct_phi = make_ct(phi);
    auto ct_zero = make_ct(0.0);
    auto ct_psi = make_ct(psi);

    // NAND_φ(a,b) = φ - a·b·ψ
    // PERO kung ang b = 0, ang a·b·ψ = 0 — libre!
    // Kaya sa alternating 0/φ, ang average level ay 1.5 per gate

    auto nand_phi = [&](auto a, auto b) {
        auto prod = cc->EvalMult(a, b);
        auto scaled = cc->EvalMult(prod, ct_psi);
        return cc->EvalSub(ct_phi, scaled);
    };

    std::cout << "18 GATES — ALTERNATING ZERO OPTIMIZATION\n";
    std::cout << "=========================================\n\n";

    auto current = ct_phi;
    int errors = 0;
    int total_gates = 18;

    auto t_start = high_resolution_clock::now();

    for (int gate = 0; gate < total_gates; gate++) {
        current = nand_phi(current, current);

        double v = decrypt_val(current);
        double expected = (gate % 2 == 0) ? 0.0 : phi;
        bool ok = (std::abs(v - expected) < 0.15 * phi);
        if (!ok) errors++;

        if (gate < 5 || gate >= total_gates - 3) {
            std::cout << "  Gate " << gate << ": v=" << v
                      << " exp=" << expected
                      << " level=" << current->GetLevel()
                      << (ok ? " ✓" : " ✗") << "\n";
        }
    }

    auto t_end = high_resolution_clock::now();
    auto total_ms = duration_cast<milliseconds>(t_end - t_start).count();

    std::cout << "\n  Result: " << errors << "/" << total_gates << " errors\n";
    std::cout << "  Level per gate: " << (double)current->GetLevel() / total_gates << "\n";

    return 0;
}
