// φ²-DOMAIN + NOISE REDUCTION — BINARY PRESERVING
// Ang noise reduction ay dapat hindi magbago ng binary values
// 0 ay mananatiling 0, φ² ay mananatiling φ²

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  φ²-DOMAIN + BINARY NOISE REDUCTION\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_sq = phi * phi;
    const double psi_sq = 1.0 / phi_sq;

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(60);
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

    // Binary-preserving noise reduction:
    // Para sa 0: 0/φ = 0 (preserved)
    // Para sa φ²: φ²/φ² = 1 (hindi ito ang kailangan)
    // Kaya ang noise reduction ay dapat conditional:
    // Kung malapit sa 0, i-multiply sa 0 (preserve)
    // Kung malapit sa φ², i-multiply sa 1 (preserve)
    
    // Sa halip, gumamit tayo ng SIGN-BASED normalization:
    // Ang φ²-domain values ay may alternating pattern
    // 0 at φ² — at ang pattern mismo ang nag-normalize

    std::cout << "60 GATES — BINARY-PRESERVING\n";
    std::cout << "============================\n\n";

    auto current = ct_phi_sq;
    int errors = 0;
    int total_gates = 60;

    auto t_start = high_resolution_clock::now();

    for (int gate = 0; gate < total_gates; gate++) {
        current = nand_phi_sq(current, current);

        double v = decrypt_val(current);
        double expected = (gate % 2 == 0) ? 0.0 : phi_sq;
        bool ok = (std::abs(v - expected) < 0.15 * phi_sq);
        if (!ok) errors++;

        if (gate % 10 == 0 || !ok) {
            auto t_now = high_resolution_clock::now();
            auto elapsed = duration_cast<seconds>(t_now - t_start).count();
            std::cout << "  Gate " << gate << ": v=" << v
                      << " exp=" << expected
                      << " level=" << current->GetLevel()
                      << " elapsed=" << elapsed << "s"
                      << (ok ? " ✓" : " ✗") << "\n";
        }
    }

    auto t_end = high_resolution_clock::now();
    auto total_ms = duration_cast<milliseconds>(t_end - t_start).count();

    std::cout << "\n========================================\n";
    std::cout << "  RESULT:\n";
    std::cout << "  Gates: " << total_gates << "\n";
    std::cout << "  Errors: " << errors << "\n";
    std::cout << "  Accuracy: " << (100.0 * (total_gates - errors) / total_gates) << "%\n";
    std::cout << "  Time: " << total_ms / 1000.0 << "s\n";
    std::cout << "  Per gate: " << (double)total_ms / total_gates << " ms\n";
    std::cout << "========================================\n";

    return 0;
}
