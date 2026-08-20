// CKKS SCALE-AWARE NAND + φ-REFRESH — FULL CYCLE
// Ang kumpletong algorithm:
// 1. NAND_1 sa scale 1 (20 gates)
// 2. φ-refresh → scale ψ^10
// 3. NAND_ψ sa scale ψ^10 (20 gates)
// 4. φ-refresh → scale ψ^20
// 5. Ulitin
//
// TARGET: 60+ gates na True FHE na may refresh

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  SCALE-AWARE NAND + φ-REFRESH\n";
    std::cout << "  Full Cycle Test\n";
    std::cout << "========================================\n\n";

    // CKKS Setup
    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(60);
    params.SetScalingModSize(40);
    params.SetBatchSize(1024);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(params);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);

    auto slots = cc->GetEncodingParams()->GetBatchSize();

    // φ at ψ constants
    const double phi = 1.6180339887498948482;
    const double psi_10 = std::pow(1.0/phi, 10.0);  // ψ^10 ≈ 0.00813
    const double phi_10 = std::pow(phi, 10.0);       // φ^10 ≈ 122.99

    std::cout << "ψ^10 = " << psi_10 << "\n";
    std::cout << "φ^10 = " << phi_10 << "\n\n";

    // Encrypted constants
    std::vector<std::complex<double>> v1(slots, {0.0, 0.0});
    v1[0] = {1.0, 0.0};

    std::vector<std::complex<double>> vec_psi(slots, {0.0, 0.0});
    vec_psi[0] = {psi_10, 0.0};

    std::vector<std::complex<double>> vec_phi(slots, {0.0, 0.0});
    vec_phi[0] = {phi_10, 0.0};

    auto ct1 = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(v1));
    auto ct_psi = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec_psi));
    auto ct_phi = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec_phi));

    // ============================================
    // SCALE-AWARE NAND VARIANTS
    // ============================================
    
    // NAND sa scale 1: 1 - a·b
    auto nand_scale1 = [&](auto a, auto b) {
        auto prod = cc->EvalMult(a, b);
        return cc->EvalSub(ct1, prod);
    };

    // NAND sa scale ψ^10: ψ^10 - a·b·φ^10
    auto nand_scale_psi = [&](auto a, auto b) {
        auto prod = cc->EvalMult(a, b);       // a·b = m₁m₂·ψ^20
        auto scaled = cc->EvalMult(prod, ct_phi);  // × φ^10 = m₁m₂·ψ^10
        return cc->EvalSub(ct_psi, scaled);    // ψ^10 - m₁m₂·ψ^10
    };

    // φ-refresh: ct → ct · ψ^10 (nagbabago ng scale pababa)
    auto refresh = [&](auto ct) {
        return cc->EvalMult(ct, ct_psi);
    };

    auto decrypt_val = [&](auto ct) {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetCKKSPackedValue()[0].real();
    };

    std::cout << "SCALE-AWARE NAND TEST:\n";
    std::cout << "=======================\n\n";

    // ============================================
    // TRUTH TABLE SA SCALE ψ^10
    // ============================================
    std::cout << "TRUTH TABLE (Scale ψ^10):\n";
    std::cout << "--------------------------\n\n";

    auto ct_psi_one = ct_psi;  // "1" sa ψ^10 scale
    auto ct_zero = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(
        std::vector<std::complex<double>>(slots, {0.0, 0.0})));

    auto nand_00 = nand_scale_psi(ct_zero, ct_zero);
    auto nand_01 = nand_scale_psi(ct_zero, ct_psi_one);
    auto nand_11 = nand_scale_psi(ct_psi_one, ct_psi_one);

    std::cout << "  NAND(0,0) = " << decrypt_val(nand_00) << " (expected " << psi_10 << ")\n";
    std::cout << "  NAND(0,1) = " << decrypt_val(nand_01) << " (expected " << psi_10 << ")\n";
    std::cout << "  NAND(1,1) = " << decrypt_val(nand_11) << " (expected 0)\n\n";

    // ============================================
    // FULL CYCLE: 10 gates + refresh + 10 gates
    // ============================================
    std::cout << "FULL CYCLE TEST:\n";
    std::cout << "=================\n\n";

    // Cycle 1: Scale 1, 10 gates
    std::cout << "Cycle 1 (Scale 1):\n";
    auto current = ct1;
    int errors = 0;

    for (int gate = 0; gate < 10; gate++) {
        current = nand_scale1(current, current);
        double v = decrypt_val(current);
        int expected = (gate % 2 == 0) ? 0 : 1;
        int got = (std::abs(v) > 0.5) ? 1 : 0;
        if (got != expected) errors++;
        std::cout << "  Gate " << gate << ": " << v << " (expected " << expected << ") "
                  << (got == expected ? "✓" : "✗") << "\n";
    }

    // Refresh → scale ψ^10
    std::cout << "\n  Refresh → scale ψ^10...\n";
    current = refresh(current);
    double v_refreshed = decrypt_val(current);
    std::cout << "  After refresh: " << v_refreshed << "\n\n";

    // Cycle 2: Scale ψ^10, 10 gates
    std::cout << "Cycle 2 (Scale ψ^10):\n";
    for (int gate = 10; gate < 20; gate++) {
        current = nand_scale_psi(current, current);
        double v = decrypt_val(current);
        int expected = (gate % 2 == 0) ? 1 : 0;  // Continue pattern
        int got = (std::abs(v - psi_10) < 0.01) ? 1 : 0;
        if (got != expected) errors++;
        std::cout << "  Gate " << gate << ": " << v << " (expected " 
                  << (expected ? psi_10 : 0.0) << ") "
                  << (got == expected ? "✓" : "✗") << "\n";
    }

    std::cout << "\n  Total errors: " << errors << "/20\n";

    return 0;
}
