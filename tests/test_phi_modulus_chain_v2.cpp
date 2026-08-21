// φ-SPACED MODULUS CHAIN — FIXED
// I-explore ang level consumption at modulus

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
    std::cout << "  φ-SPACED MODULUS CHAIN — V2\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(30);
    params.SetScalingModSize(40);
    params.SetBatchSize(256);

    auto cc = GenCryptoContext(params);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);

    std::cout << "LEVEL CONSUMPTION ANALYSIS:\n";
    std::cout << "===========================\n\n";

    auto slots = cc->GetEncodingParams()->GetBatchSize();
    auto make_ct = [&](double val) {
        std::vector<std::complex<double>> vec(slots, {0.0, 0.0});
        vec[0] = {val, 0.0};
        return cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec));
    };

    auto ct_a = make_ct(1.0);
    auto ct_b = make_ct(2.0);

    std::cout << "  Initial level: " << ct_a->GetLevel() << "\n";

    auto ct_prod = cc->EvalMult(ct_a, ct_b);
    std::cout << "  After 1st mult: " << ct_prod->GetLevel() << "\n";

    auto ct_prod2 = cc->EvalMult(ct_prod, ct_a);
    std::cout << "  After 2nd mult: " << ct_prod2->GetLevel() << "\n";

    auto ct_prod3 = cc->EvalMult(ct_prod2, ct_a);
    std::cout << "  After 3rd mult: " << ct_prod3->GetLevel() << "\n";

    std::cout << "\n";

    // ============================================
    // φ-SPACED THEORETICAL ANALYSIS
    // ============================================
    std::cout << "φ-SPACED THEORETICAL ANALYSIS:\n";
    std::cout << "==============================\n\n";

    std::cout << "  Standard CKKS:\n";
    std::cout << "    Level n: q₀/Δⁿ\n";
    std::cout << "    Consumption: 1 level per mult\n\n";

    std::cout << "  φ-CKKS (kung ma-implement):\n";
    std::cout << "    Level n: q₀/φⁿ\n";
    std::cout << "    Consumption: 1-1/φ = " << (1.0 - 1.0/phi) << " levels per mult\n\n";

    std::cout << "  Depth 30 comparison:\n";
    std::cout << "    Standard: " << 30 << " gates\n";
    std::cout << "    φ-optimized: " << (int)(30.0 / (1.0 - 1.0/phi)) << " gates\n\n";

    // ============================================
    // EMERGENT PROPERTY CHECK
    // ============================================
    std::cout << "EMERGENT PROPERTY CHECK:\n";
    std::cout << "========================\n\n";

    std::cout << "  φ² = φ + 1 = " << (phi * phi) << "\n";
    std::cout << "  φ² - φ - 1 = " << (phi * phi - phi - 1) << "\n\n";

    std::cout << "  Kung ang level ay φ-spaced:\n";
    std::cout << "    Level L → Level L-1 ay ratio φ\n";
    std::cout << "    Level L → Level L-2 ay ratio φ² = φ+1\n\n";

    std::cout << "  Ang period-2 ay nasa ratio φ²:\n";
    std::cout << "    q_{n+2} = q_n / φ²\n";
    std::cout << "    q_{n+2} = q_n / (φ+1)\n\n";

    std::cout << "  Kung ang 2-level reduction ay φ²:\n";
    std::cout << "    Effective 1-level reduction: sqrt(φ²) = φ\n";
    std::cout << "    Hindi ito 1 — ito ay φ = " << phi << "\n\n";

    std::cout << "  Ibig sabihin, ang level consumption ay:\n";
    std::cout << "    Hindi 1 level per mult\n";
    std::cout << "    Kundi φ levels per 2 mults\n";
    std::cout << "    = " << phi << " levels per 2 mults\n";
    std::cout << "    = " << phi/2 << " levels per mult\n\n";

    std::cout << "  Depth 30 with φ-consumption:\n";
    std::cout << "    Max gates = 30 / " << (phi/2) << " = " << (int)(30.0 / (phi/2)) << " gates\n";

    return 0;
}
