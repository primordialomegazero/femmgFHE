// φ²-DOMAIN — CORRECTED ENCODING
// I-map ang -1 → 0 at φ² → φ para sa tamang binary logic

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  φ²-DOMAIN — CORRECTED\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_sq = phi * phi;

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(30);
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
    auto ct_one = make_ct(1.0);
    auto ct_phi = make_ct(phi);

    // NAND sa φ²-domain:
    // NAND(a,b) = φ² - a·b·ψ² (2 mults)
    // Pero ang normalization ay subtraction lang:
    // Kung output = -1, i-interpret natin bilang 0
    // Kung output = φ², i-interpret natin bilang φ

    auto nand_phi_sq = [&](auto a, auto b) {
        auto prod = cc->EvalMult(a, b);  // 1 mult
        auto ct_psi_sq = make_ct(1.0 / phi_sq);
        auto scaled = cc->EvalMult(prod, ct_psi_sq);  // 2nd mult
        return cc->EvalSub(ct_phi_sq, scaled);  // subtraction
    };

    std::cout << "TRUTH TABLE (φ²-domain):\n";
    std::cout << "========================\n\n";

    auto t00 = nand_phi_sq(make_ct(0.0), make_ct(0.0));
    auto t0phi = nand_phi_sq(make_ct(0.0), ct_phi_sq);
    auto tphiphi = nand_phi_sq(ct_phi_sq, ct_phi_sq);

    std::cout << "  NAND(0,0) = " << decrypt_val(t00) << " (expected " << phi_sq << ")\n";
    std::cout << "  NAND(0,φ²) = " << decrypt_val(t0phi) << " (expected " << phi_sq << ")\n";
    std::cout << "  NAND(φ²,φ²) = " << decrypt_val(tphiphi) << " (expected 0)\n\n";

    std::cout << "ALTERNATING CHAIN (15 gates):\n";
    std::cout << "=============================\n\n";

    auto current = ct_phi_sq;
    for (int gate = 0; gate < 15; gate++) {
        current = nand_phi_sq(current, current);
        double v = decrypt_val(current);
        
        // Mapping: -1 → 0, φ² → φ
        double mapped = (std::abs(v - (-1.0)) < 0.1) ? 0.0 : 
                        (std::abs(v - phi_sq) < 0.1) ? phi : v;
        
        std::cout << "  Gate " << gate << ": raw=" << v 
                  << " mapped=" << mapped
                  << " level=" << current->GetLevel() << "\n";
    }

    return 0;
}
