// 1-MULT φ²-DOMAIN NAND — PRE-SCALED INPUTS
// Kung ang inputs ay naka-scale na sa ψ²,
// ang multiplication ay direktang nagbibigay ng a·b·ψ²

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  1-MULT φ²-DOMAIN NAND\n";
    std::cout << "  Pre-Scaled Inputs\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_sq = phi * phi;
    const double psi_sq = 1.0 / phi_sq;

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(80);
    params.SetScalingModSize(30);
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

    auto ct_phi_sq = make_ct(phi_sq);

    // Pre-scaled NAND:
    // Ang inputs ay φ² (true) at 0 (false)
    // NAND(a,b) = φ² - a·b (1 mult lang!)
    // Dahil ang a·b ay automatic na ψ²-scaled
    
    auto nand_1mult = [&](auto a, auto b) {
        auto prod = cc->EvalMult(a, b);  // 1 mult
        return cc->EvalSub(ct_phi_sq, prod);  // subtraction
    };

    std::cout << "TRUTH TABLE:\n";
    std::cout << "============\n\n";

    auto t00 = nand_1mult(make_ct(0.0), make_ct(0.0));
    auto t0phi = nand_1mult(make_ct(0.0), ct_phi_sq);
    auto tphiphi = nand_1mult(ct_phi_sq, ct_phi_sq);

    std::cout << "  NAND(0,0) = " << decrypt_val(t00) << " (expected " << phi_sq << ")\n";
    std::cout << "  NAND(0,φ²) = " << decrypt_val(t0phi) << " (expected " << phi_sq << ")\n";
    std::cout << "  NAND(φ²,φ²) = " << decrypt_val(tphiphi) << " (expected 0)\n\n";

    std::cout << "80 GATES — 1 MULT EACH\n";
    std::cout << "=======================\n\n";

    auto current = ct_phi_sq;
    int errors = 0;

    for (int gate = 0; gate < 80; gate++) {
        current = nand_1mult(current, current);
        double v = decrypt_val(current);
        double expected = (gate % 2 == 0) ? 0.0 : phi_sq;
        bool ok = (std::abs(v - expected) < 0.15 * phi_sq);
        if (!ok) errors++;

        if (gate % 10 == 0 || !ok) {
            std::cout << "  Gate " << gate << ": v=" << v
                      << " exp=" << expected
                      << " level=" << current->GetLevel()
                      << (ok ? " ✓" : " ✗") << "\n";
        }
    }

    std::cout << "\n  Errors: " << errors << "/80\n";
    std::cout << "  Accuracy: " << (100.0 * (80 - errors) / 80) << "%\n";

    return 0;
}
