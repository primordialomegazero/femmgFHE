// φ²-DOMAIN + CASSINI VERIFICATION
// Pagsamahin ang φ²-domain NAND sa Cassini invariant
// para ma-detect kung kailan kailangan ng natural reset

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  φ²-DOMAIN + CASSINI\n";
    std::cout << "  Natural Reset Detection\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_sq = phi * phi;
    const double psi_sq = 1.0 / phi_sq;

    // Cassini invariant: φ·ψ + 1 = 0 (o φ·ψ - 1 = 0)
    std::cout << "CASSINI INVARIANT:\n";
    std::cout << "  φ · ψ + 1 = " << (phi * (1.0/phi) + 1.0) << " (should be 2)\n";
    std::cout << "  φ · ψ - 1 = " << (phi * (1.0/phi) - 1.0) << " (should be 0)\n";
    std::cout << "  φ · (-ψ) + 1 = " << (phi * (-1.0/phi) + 1.0) << " (should be 0)\n\n";

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(50);
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
    auto ct_psi_sq = make_ct(psi_sq);

    // NAND sa φ²-domain (2 mults)
    auto nand_phi_sq = [&](auto a, auto b) {
        auto prod = cc->EvalMult(a, b);
        auto scaled = cc->EvalMult(prod, ct_psi_sq);
        return cc->EvalSub(ct_phi_sq, scaled);
    };

    // Cassini tracking
    std::cout << "50 GATES + CASSINI TRACKING\n";
    std::cout << "============================\n\n";

    auto current = ct_phi_sq;
    int errors = 0;
    std::vector<double> cassini_values;

    for (int gate = 0; gate < 50; gate++) {
        current = nand_phi_sq(current, current);
        double v = decrypt_val(current);

        // Compute Cassini-like invariant para sa kasalukuyang value
        double cassini = std::abs(v * (1.0/v - 1.0) + 1.0);
        cassini_values.push_back(cassini);

        double expected = (gate % 2 == 0) ? 0.0 : phi_sq;
        bool ok = (std::abs(v - expected) < 0.15 * phi_sq);
        if (!ok) errors++;

        if (gate % 10 == 0 || !ok) {
            std::cout << "  Gate " << gate << ": v=" << v
                      << " cassini=" << cassini
                      << " level=" << current->GetLevel()
                      << (ok ? " ✓" : " ✗") << "\n";
        }
    }

    // Check kung may pattern sa Cassini values
    std::cout << "\nCASSINI ANALYSIS:\n";
    std::cout << "=================\n\n";

    double cassini_avg = 0;
    for (auto c : cassini_values) cassini_avg += c;
    cassini_avg /= cassini_values.size();

    std::cout << "  Average Cassini: " << cassini_avg << "\n";
    std::cout << "  Expected (φ·ψ+1): 2.0\n";
    std::cout << "  Expected (φ·(-ψ)+1): 0.0\n\n";

    std::cout << "  Errors: " << errors << "/50\n";
    std::cout << "  Pattern: " << (errors == 0 ? "STABLE!" : "DRIFTING") << "\n";

    return 0;
}
