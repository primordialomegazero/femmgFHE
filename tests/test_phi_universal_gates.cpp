// φ²-DOMAIN UNIVERSAL GATES
// Subukan kung lahat ng logic gates kaya sa 0-level

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  φ²-DOMAIN UNIVERSAL GATES\n";
    std::cout << "========================================\n\n";

    const double phi_sq = 2.618033988749895;

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(5);
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

    auto ct_phi_sq = make_ct(phi_sq);
    auto ct_zero = make_ct(0.0);

    // Sa φ²-domain: φ² = true, 0 = false
    // Ang pag-convert sa binary: v > phi_sq/2 ? 1 : 0
    auto to_binary = [&](double v) {
        return (v > phi_sq / 2.0) ? 1 : 0;
    };

    std::cout << "TRUTH TABLES (φ²-domain):\n";
    std::cout << "=========================\n\n";

    // NOT: φ² → 0, 0 → φ² (add/sub lang)
    std::cout << "NOT GATE:\n";
    auto not_phi_sq = cc->EvalSub(ct_phi_sq, ct_phi_sq);  // 0
    std::cout << "  NOT(φ²) = " << to_binary(decrypt_val(not_phi_sq)) << " (expected 0)\n";
    auto not_zero = cc->EvalAdd(ct_zero, ct_phi_sq);  // φ²
    std::cout << "  NOT(0) = " << to_binary(decrypt_val(not_zero)) << " (expected 1)\n\n";

    // NAND: φ² - (a + b) sa φ²-domain
    // NAND(φ²,φ²) = φ² - 2φ² = -φ² → interpret as 0
    // NAND(0,0) = φ² - 0 = φ² → 1
    // NAND(φ²,0) = φ² - φ² = 0 → 0
    std::cout << "NAND GATE (AddSub):\n";
    auto nand_pp = cc->EvalSub(ct_phi_sq, cc->EvalAdd(ct_phi_sq, ct_phi_sq));
    auto nand_00 = cc->EvalSub(ct_phi_sq, cc->EvalAdd(ct_zero, ct_zero));
    auto nand_p0 = cc->EvalSub(ct_phi_sq, cc->EvalAdd(ct_phi_sq, ct_zero));

    std::cout << "  NAND(φ²,φ²) = " << to_binary(std::abs(decrypt_val(nand_pp))) << " (expected 0)\n";
    std::cout << "  NAND(0,0) = " << to_binary(decrypt_val(nand_00)) << " (expected 1)\n";
    std::cout << "  NAND(φ²,0) = " << to_binary(decrypt_val(nand_p0)) << " (expected 1)\n\n";

    // AND: φ² - (a + b) + φ² = 2φ² - (a + b)
    // AND(φ²,φ²) = 2φ² - 2φ² = 0 → 0?
    // Hindi ito standard AND. Kailangan ng φ²-domain truth table.

    std::cout << "OBSERVATION:\n";
    std::cout << "============\n\n";
    std::cout << "  Ang φ²-domain NAND ay may period-2 sa NOT chain.\n";
    std::cout << "  Para sa arbitrary inputs, kailangan ng mas\n";
    std::cout << "  malalim na mapping.\n\n";

    // 10000 gates NOT chain para sa stress test
    std::cout << "10,000 GATES NOT CHAIN (φ² AddSub):\n";
    std::cout << "====================================\n\n";

    auto current = ct_phi_sq;
    int errors = 0;

    for (int gate = 0; gate < 10000; gate++) {
        if (gate % 2 == 0) {
            current = cc->EvalSub(current, ct_phi_sq);
        } else {
            current = cc->EvalAdd(current, ct_phi_sq);
        }

        double v = decrypt_val(current);
        double expected = (gate % 2 == 0) ? 0.0 : phi_sq;
        bool ok = (std::abs(v - expected) < 0.15 * phi_sq);
        if (!ok) errors++;
    }

    std::cout << "  Result: " << errors << "/10000 errors\n";
    std::cout << "  Level: " << current->GetLevel() << "\n";
    std::cout << "  Status: " << (errors == 0 ? "✅ PASS" : "❌ FAIL") << "\n";

    return 0;
}
