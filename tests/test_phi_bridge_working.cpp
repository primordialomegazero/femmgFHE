// φ-BRIDGE WORKING — Tamang Level Budget
// 14 + bridge + 14 = 29 levels (kasya sa depth 30)

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
    std::cout << "  φ-BRIDGE WORKING\n";
    std::cout << "  14 + Bridge + 14 = 29 gates\n";
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
    const double psi = 1.0 / phi;

    auto ct_one = make_ct(1.0);
    auto ct_psi = make_ct(psi);
    auto ct_phi = make_ct(phi);

    auto nand_op = [&](auto a, auto b) {
        auto prod = cc->EvalMult(a, b);
        return cc->EvalSub(ct_one, prod);
    };

    std::cout << "29 GATES (14 φ-chain + bridge + 14 ψ-chain):\n";
    std::cout << "==============================================\n\n";

    auto current = make_ct(1.0);
    int errors = 0;

    // φ-chain: 14 gates
    for (int gate = 0; gate < 14; gate++) {
        current = nand_op(current, current);
        double v = decrypt_val(current);
        int expected = (gate % 2 == 0) ? 0 : 1;
        int got = (std::abs(v) > 0.5) ? 1 : 0;
        if (got != expected) errors++;
    }

    // Bridge: φ → ψ (1 mult)
    current = cc->EvalMult(current, ct_psi);

    // ψ-chain: 14 gates (scale = ψ)
    for (int gate = 14; gate < 28; gate++) {
        current = nand_op(current, current);
        double v = decrypt_val(current);
        double scale = psi;
        double expected_val = (gate % 2 == 0) ? 0.0 : scale;
        bool ok = (std::abs(v - expected_val) < 0.15);
        if (!ok) errors++;

        if (gate < 17 || gate >= 25) {
            std::cout << "  Gate " << gate << ": v=" << v
                      << " exp=" << expected_val
                      << (ok ? " ✓" : " ✗") << "\n";
        }
    }

    std::cout << "\n  Total errors: " << errors << "/28\n";
    std::cout << "  Accuracy: " << (100.0 * (28 - errors) / 28) << "%\n";
    std::cout << "  Level used: 14 + 1 + 14 = 29 (kasya sa 30!)\n";

    return 0;
}
