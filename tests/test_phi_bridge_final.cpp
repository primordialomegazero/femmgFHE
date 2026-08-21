// φ-BRIDGE FINAL — Scale-Aware NAND
// Ang ψ-chain ay kailangan ng ψ-based NAND
// Hindi na 1-based NAND

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
    std::cout << "  φ-BRIDGE FINAL — SCALE-AWARE\n";
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

    // φ-chain NAND: 1 - a·b
    auto nand_phi_chain = [&](auto a, auto b) {
        auto prod = cc->EvalMult(a, b);
        return cc->EvalSub(ct_one, prod);
    };

    // ψ-chain NAND: ψ - a·b/ψ = ψ - a·b·φ
    auto nand_psi_chain = [&](auto a, auto b) {
        auto prod = cc->EvalMult(a, b);
        auto scaled = cc->EvalMult(prod, ct_phi);  // a·b·φ = a·b/ψ
        return cc->EvalSub(ct_psi, scaled);
    };

    std::cout << "29 GATES (14 φ + bridge + 14 ψ) — SCALE-AWARE:\n";
    std::cout << "=================================================\n\n";

    auto current = make_ct(1.0);
    int errors = 0;

    // φ-chain: 14 gates (scale 1)
    for (int gate = 0; gate < 14; gate++) {
        current = nand_phi_chain(current, current);
        double v = decrypt_val(current);
        int expected = (gate % 2 == 0) ? 0 : 1;
        int got = (std::abs(v) > 0.5) ? 1 : 0;
        if (got != expected) errors++;
    }

    // Bridge: φ → ψ (1 mult)
    current = cc->EvalMult(current, ct_psi);

    // ψ-chain: 14 gates (scale ψ)
    for (int gate = 14; gate < 28; gate++) {
        current = nand_psi_chain(current, current);
        double v = decrypt_val(current);
        double expected_val = (gate % 2 == 0) ? 0.0 : psi;
        bool ok = (std::abs(v - expected_val) < 0.15);
        if (!ok) errors++;

        std::cout << "  Gate " << gate << ": v=" << v
                  << " exp=" << expected_val
                  << (ok ? " ✓" : " ✗") << "\n";
    }

    std::cout << "\n  Total errors: " << errors << "/28\n";
    std::cout << "  Accuracy: " << (100.0 * (28 - errors) / 28) << "%\n";

    return 0;
}
