// φ-DOMAIN PERIOD-2 — TAMANG ENCODING
// "1" = φ, "0" = 0
// NAND_φ(a,b) = φ - a·b·ψ
// Period-2: NOT_φ(NOT_φ(x)) = x

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
    std::cout << "  φ-DOMAIN PERIOD-2 — CORRECT\n";
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
    auto nand_phi = [&](auto a, auto b) {
        auto prod = cc->EvalMult(a, b);        // a·b
        auto scaled = cc->EvalMult(prod, ct_psi); // a·b·ψ
        return cc->EvalSub(ct_phi, scaled);     // φ - a·b·ψ
    };

    // ============================================
    // TRUTH TABLE — TAMANG ENCODING
    // ============================================
    std::cout << "TRUTH TABLE (φ = true, 0 = false):\n";
    std::cout << "==================================\n\n";

    auto t00 = nand_phi(ct_zero, ct_zero);
    auto t0phi = nand_phi(ct_zero, ct_phi);
    auto tphiphi = nand_phi(ct_phi, ct_phi);

    std::cout << "  NAND(0,0) = " << decrypt_val(t00) << " (expected " << phi << ")\n";
    std::cout << "  NAND(0,φ) = " << decrypt_val(t0phi) << " (expected " << phi << ")\n";
    std::cout << "  NAND(φ,φ) = " << decrypt_val(tphiphi) << " (expected 0)\n\n";

    // ============================================
    // PERIOD-2 — 12 GATES (2 mults each = 24 mults)
    // ============================================
    std::cout << "PERIOD-2 — 12 GATES:\n";
    std::cout << "=====================\n\n";

    auto current = ct_phi;
    int errors = 0;
    int total_gates = 12;

    for (int gate = 0; gate < total_gates; gate++) {
        current = nand_phi(current, current);

        double v = decrypt_val(current);
        double expected = (gate % 2 == 0) ? 0.0 : phi;
        bool ok = (std::abs(v - expected) < 0.15 * phi);
        if (!ok) errors++;

        std::cout << "  Gate " << gate << ": v=" << v
                  << " exp=" << expected
                  << " level=" << current->GetLevel()
                  << (ok ? " ✓" : " ✗") << "\n";
    }

    std::cout << "\n  Errors: " << errors << "/" << total_gates << "\n";
    std::cout << "  (2 mults per gate, 12 gates = 24 mults sa depth 30)\n";

    return 0;
}
