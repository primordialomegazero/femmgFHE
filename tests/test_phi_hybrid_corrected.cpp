// φ²-DOMAIN HYBRID — CORRECTED
// Pre-scaled inputs para sa 1-mult NAND

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  φ²-DOMAIN HYBRID — CORRECTED\n";
    std::cout << "  Pre-Scaled 1-Mult NAND\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_sq = phi * phi;
    const double psi = 1.0 / phi;

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(10);
    params.SetScalingModSize(50);
    params.SetBatchSize(256);
    params.SetFirstModSize(60);

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

    // Pre-scaled values: "true" = φ (hindi φ²)
    // Sa φ-domain: NAND(a,b) = φ - a·b·ψ (2 mults)
    // Kung pre-scaled: a' = a·ψ, b' = b·ψ
    // NAND(a',b') = φ - a'·b' (1 mult)

    auto ct_phi = make_ct(phi);  // true = φ
    auto ct_zero = make_ct(0.0);

    // 1-Mult NAND sa pre-scaled φ-domain
    auto nand_1mult = [&](auto a, auto b) {
        auto prod = cc->EvalMult(a, b);  // a·b (pre-scaled na)
        return cc->EvalSub(ct_phi, prod);  // φ - a·b
    };

    std::cout << "TRUTH TABLE (pre-scaled φ-domain):\n";
    std::cout << "==================================\n\n";

    auto t00 = nand_1mult(ct_zero, ct_zero);
    auto t0p = nand_1mult(ct_zero, ct_phi);
    auto tpp = nand_1mult(ct_phi, ct_phi);

    std::cout << "  NAND(0,0) = " << decrypt_val(t00) << " (expected " << phi << ")\n";
    std::cout << "  NAND(0,φ) = " << decrypt_val(t0p) << " (expected " << phi << ")\n";
    std::cout << "  NAND(φ,φ) = " << decrypt_val(tpp) << " (expected 0)\n\n";

    // NOT chain sa pre-scaled domain
    std::cout << "NOT CHAIN (pre-scaled):\n";
    std::cout << "=======================\n\n";

    auto current = ct_phi;
    int errors = 0;

    for (int gate = 0; gate < 20; gate++) {
        current = nand_1mult(current, current);
        double v = decrypt_val(current);
        double expected = (gate % 2 == 0) ? 0.0 : phi;
        bool ok = (std::abs(v - expected) < 0.15 * phi);
        if (!ok) errors++;

        if (gate < 5 || !ok) {
            std::cout << "  Gate " << gate << ": v=" << v
                      << " exp=" << expected
                      << " level=" << current->GetLevel()
                      << (ok ? " ✓" : " ✗") << "\n";
        }
    }

    std::cout << "\n  Errors: " << errors << "/20\n";
    std::cout << "  Level: " << current->GetLevel() << "\n";

    return 0;
}
