// φ-ALTERNATING BRIDGE — 1 Gate, 1 Bridge Pattern
// Ang bawat gate ay sinusundan ng bridge sa kabilang chain
// Ito ay nagbibigay ng natural na level distribution
//
// PATTERN:
// Gate 0 (φ-chain) → Bridge ψ → Gate 1 (ψ-chain) → Bridge φ → ...
// 
// ANG KEY: Ang level ay na-di-distribute sa dalawang chains
// kaya ang bawat chain ay may kalahati lang ng gates.

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
    std::cout << "  φ-ALTERNATING BRIDGE\n";
    std::cout << "  1 Gate + 1 Bridge Pattern\n";
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

    // NAND variants
    auto nand_phi_chain = [&](auto a, auto b) {
        auto prod = cc->EvalMult(a, b);
        return cc->EvalSub(ct_one, prod);  // 1 - a·b
    };
    auto nand_psi_chain = [&](auto a, auto b) {
        auto prod = cc->EvalMult(a, b);
        auto scaled = cc->EvalMult(prod, ct_phi);  // a·b·φ
        return cc->EvalSub(ct_psi, scaled);  // ψ - a·b·φ
    };

    std::cout << "ALTERNATING: 28 gates (14 per chain)\n";
    std::cout << "=====================================\n\n";

    auto current = make_ct(1.0);  // Start sa φ-chain
    int errors = 0;
    bool in_phi_chain = true;

    for (int gate = 0; gate < 28; gate++) {
        // NAND sa kasalukuyang chain
        if (in_phi_chain) {
            current = nand_phi_chain(current, current);
        } else {
            current = nand_psi_chain(current, current);
        }

        // Verify
        double v = decrypt_val(current);
        double scale = in_phi_chain ? 1.0 : psi;
        double expected_val = (gate % 2 == 0) ? 0.0 : scale;
        bool ok = (std::abs(v - expected_val) < 0.15);
        if (!ok) errors++;

        if (gate < 6 || gate >= 22) {
            std::cout << "  Gate " << gate << " [" << (in_phi_chain ? "φ" : "ψ")
                      << "]: v=" << v << " exp=" << expected_val
                      << (ok ? " ✓" : " ✗") << "\n";
        }

        // Bridge sa kabilang chain (1 mult)
        if (in_phi_chain) {
            current = cc->EvalMult(current, ct_psi);
        } else {
            current = cc->EvalMult(current, ct_phi);
        }
        in_phi_chain = !in_phi_chain;
    }

    std::cout << "\n  Total errors: " << errors << "/28\n";
    std::cout << "  Accuracy: " << (100.0 * (28 - errors) / 28) << "%\n";
    std::cout << "  Level: 28 gates + 27 bridges = 55 mults... \n";
    std::cout << "  (Dapat kasya sa depth 30 kung ang bridge ay 0-level)\n";

    return 0;
}
