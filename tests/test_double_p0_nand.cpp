// DOUBLE PERIOD-0 — TAMANG NAND MAPPING
// Hanapin ang tamang mapping mula sa difference
// papuntang NAND truth table

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  DOUBLE P0 — NAND MAPPING\n";
    std::cout << "  Tamang Mapping Search\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_sq = phi * phi;
    const double phi_cu = phi * phi * phi;

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

    auto ct_phi_sq_mod = make_ct(phi_sq - 2.0);   // 0.618034
    auto ct_phi_cu_mod = make_ct(phi_cu - 4.0);   // 0.236068

    // Ang strategy: gumamit ng DIFFERENT rotations para sa bawat input
    // Kung a=0: rotate ng φ²
    // Kung a=φ: rotate ng φ³
    // Kung b=0: rotate ng φ³
    // Kung b=φ: rotate ng φ²

    auto nand_double = [&](auto a, auto b) {
        // I-rotate ang a at b sa iba't ibang paraan
        auto a_rot = cc->EvalAdd(a, ct_phi_sq_mod);
        auto b_rot = cc->EvalAdd(b, ct_phi_cu_mod);

        // Kunin ang sum
        auto sum = cc->EvalAdd(a_rot, b_rot);

        // Bounded sa [0,1)
        double v = decrypt_val(sum);
        v = v - std::floor(v);

        // NAND: 0 kung pareho φ, φ kung hindi
        // Sa double rotation, ang sum ay may unique pattern
        return v;
    };

    std::cout << "NAND VIA DOUBLE ROTATION:\n";
    std::cout << "=========================\n\n";

    auto ct_zero = make_ct(0.0);
    auto ct_phi = make_ct(phi);

    double nand_00 = nand_double(ct_zero, ct_zero);
    double nand_0p = nand_double(ct_zero, ct_phi);
    double nand_p0 = nand_double(ct_phi, ct_zero);
    double nand_pp = nand_double(ct_phi, ct_phi);

    std::cout << "  NAND(0,0) = " << nand_00 << "\n";
    std::cout << "  NAND(0,φ) = " << nand_0p << "\n";
    std::cout << "  NAND(φ,0) = " << nand_p0 << "\n";
    std::cout << "  NAND(φ,φ) = " << nand_pp << "\n\n";

    // Ang pattern na hinahanap natin:
    // NAND(0,0) = φ (true)
    // NAND(0,φ) = φ (true)
    // NAND(φ,0) = φ (true)
    // NAND(φ,φ) = 0 (false)
    //
    // Kung ang sum ay:
    // 0+0: 0.618 + 0.236 = 0.854 (true?)
    // 0+φ: 0.618 + φ = φ + 0.618 (mod 1)
    // φ+φ: φ + 0.618 + φ + 0.236 (complicated)

    std::cout << "  Expected: φ, φ, φ, 0\n";
    std::cout << "  Level: 0\n";
    std::cout << "  Status: Mapping complete — analyze values\n";

    return 0;
}
