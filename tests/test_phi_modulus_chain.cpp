// φ-SPACED MODULUS CHAIN
// Subukan kung ang φ-spaced modulus ay nagbibigay
// ng ibang level consumption

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
    std::cout << "  φ-SPACED MODULUS CHAIN\n";
    std::cout << "  Level Economy Exploration\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;

    // ============================================
    // 1. TINGNAN ANG KASALUKUYANG MODULUS
    // ============================================
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

    std::cout << "KASALUKUYANG MODULUS INFO:\n";
    std::cout << "===========================\n\n";

    auto cryptoParams = cc->GetCryptoParameters();
    auto elementParams = cryptoParams->GetElementParams();

    // Subukan nating makuha ang modulus chain
    try {
        auto& moduli = elementParams->GetModuli();
        std::cout << "  Moduli size: " << moduli.size() << "\n";
        for (size_t i = 0; i < std::min(moduli.size(), (size_t)10); i++) {
            std::cout << "    Level " << i << ": " << moduli[i].GetMSB() << " bits\n";
        }
        if (moduli.size() > 10) {
            std::cout << "    ... (at " << moduli.size() - 10 << " pa)\n";
        }
    } catch (...) {
        std::cout << "  (hindi ma-access ang moduli)\n";
    }

    std::cout << "\n";

    // ============================================
    // 2. LEVEL CONSUMPTION ANALYSIS
    // ============================================
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
    // 3. φ-SPACED THEORETICAL ANALYSIS
    // ============================================
    std::cout << "φ-SPACED THEORETICAL ANALYSIS:\n";
    std::cout << "==============================\n\n";

    std::cout << "  Standard chain: q₀, q₀/Δ, q₀/Δ², ...\n";
    std::cout << "  φ-chain: q₀, q₀/φ, q₀/φ², ...\n\n";

    std::cout << "  Sa standard, level consumption ay 1 per mult.\n";
    std::cout << "  Sa φ-chain, ang ratio ng levels ay φ:\n";
    std::cout << "    Level n: q₀/φⁿ\n\n";

    std::cout << "  Kung ang noise growth ay may φ-factor,\n";
    std::cout << "  ang effective level consumption ay:\n";
    std::cout << "    1 - 1/φ = 0.382 levels per mult\n\n";

    std::cout << "  Kaya sa depth 30:\n";
    std::cout << "    Standard: 30 gates\n";
    std::cout << "    φ-optimized: " << (int)(30.0 / 0.382) << " gates\n\n";

    // ============================================
    // 4. ANO ANG KAILANGAN
    // ============================================
    std::cout << "ANG KAILANGAN:\n";
    std::cout << "==============\n\n";

    std::cout << "  1. Custom modulus switching na φ-based\n";
    std::cout << "  2. Prime chain na may φ-ratio\n";
    std::cout << "  3. Level tracking na φ-aware\n\n";

    std::cout << "  Ang OpenFHE ay may FIXED modulus chain.\n";
    std::cout << "  Kailangan nating i-modify ang source code\n";
    std::cout << "  para sa φ-spaced primes.\n";

    return 0;
}
