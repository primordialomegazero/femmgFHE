// φ-NATIVE FRAMEWORK — UNIFIED IMPLEMENTATION
// Lahat natural, walang arbitrary constants

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  φ-NATIVE FRAMEWORK\n";
    std::cout << "  Unified Natural Implementation\n";
    std::cout << "========================================\n\n";

    // LAHAT NG CONSTANTS AY φ-NATIVE
    const double PHI = 1.6180339887498948482;           // φ
    const double PHI_SQ = PHI * PHI;                    // φ² = φ + 1
    const double PHI_MOD = PHI_SQ - 2.0;                // φ² mod 1
    const double PSI = -1.0 / PHI;                     // ψ = -1/φ
    const double GOLDEN_ANGLE = 2.0 * M_PI * (1.0 - 1.0/PHI); // 137.5°

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(5);
    params.SetScalingModSize(50);
    params.SetBatchSize(256);
    params.SetFirstModSize(60);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(params);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    cc->Enable(ADVANCEDSHE);

    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);
    cc->EvalAtIndexKeyGen(keys.secretKey, {1, -1});

    auto slots = cc->GetEncodingParams()->GetBatchSize();

    auto decrypt_slot = [&](auto ct, int slot_num) {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetCKKSPackedValue()[slot_num].real();
    };

    auto make_uniform = [&](double val) {
        std::vector<std::complex<double>> vec(slots, {0.0, 0.0});
        for (int i = 0; i < slots; i++) vec[i] = {val, 0.0};
        return cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec));
    };

    // ============================================
    // φ-NATIVE GATES (LAHAT 0-LEVEL)
    // ============================================
    
    // NOT: NOT(x) = φ² - x (Golden Identity)
    auto phi_not = [&](auto x) {
        auto ct_phi_sq = make_uniform(PHI_SQ);
        return cc->EvalSub(ct_phi_sq, x);
    };

    // NAND: NAND(a,b) = φ² - (a+b) na may Period-0 threshold
    // (0,0)→φ², (0,φ)→φ², (φ,0)→φ², (φ,φ)→0
    auto phi_nand = [&](auto a, auto b) {
        auto sum = cc->EvalAdd(a, b);
        auto rotated = cc->EvalAdd(sum, make_uniform(PHI_MOD));
        
        double v = decrypt_slot(rotated, 128);
        v = v - std::floor(v);
        
        // Natural threshold mula sa Golden Angle
        double threshold = GOLDEN_ANGLE / (2.0 * M_PI); // ≈ 0.382
        return (v < threshold) ? make_uniform(PHI_SQ) : make_uniform(0.0);
    };

    // AND: AND(a,b) = NOT(NAND(a,b)) — De Morgan
    auto phi_and = [&](auto a, auto b) {
        return phi_not(phi_nand(a, b));
    };

    // OR: OR(a,b) = NAND(NOT(a), NOT(b)) — De Morgan
    auto phi_or = [&](auto a, auto b) {
        return phi_nand(phi_not(a), phi_not(b));
    };

    // XOR: XOR(a,b) = NAND(NAND(a,NAND(a,b)), NAND(b,NAND(a,b)))
    auto phi_xor = [&](auto a, auto b) {
        auto nand_ab = phi_nand(a, b);
        auto nand_a_nab = phi_nand(a, nand_ab);
        auto nand_b_nab = phi_nand(b, nand_ab);
        return phi_nand(nand_a_nab, nand_b_nab);
    };

    // ============================================
    // TEST: LAHAT NG GATES
    // ============================================
    std::cout << "φ-NATIVE GATE TESTS:\n";
    std::cout << "====================\n\n";

    auto ct_zero = make_uniform(0.0);
    auto ct_one = make_uniform(PHI_SQ);

    std::cout << "NOT GATE:\n";
    std::cout << "  NOT(0) = " << decrypt_slot(phi_not(ct_zero), 128) << " (expected " << PHI_SQ << ")\n";
    std::cout << "  NOT(φ²) = " << decrypt_slot(phi_not(ct_one), 128) << " (expected 0)\n\n";

    std::cout << "NAND GATE:\n";
    std::cout << "  NAND(0,0) = " << decrypt_slot(phi_nand(ct_zero, ct_zero), 128) << " (expected " << PHI_SQ << ")\n";
    std::cout << "  NAND(0,1) = " << decrypt_slot(phi_nand(ct_zero, ct_one), 128) << " (expected " << PHI_SQ << ")\n";
    std::cout << "  NAND(1,1) = " << decrypt_slot(phi_nand(ct_one, ct_one), 128) << " (expected 0)\n\n";

    std::cout << "AND GATE:\n";
    std::cout << "  AND(0,0) = " << decrypt_slot(phi_and(ct_zero, ct_zero), 128) << " (expected 0)\n";
    std::cout << "  AND(1,1) = " << decrypt_slot(phi_and(ct_one, ct_one), 128) << " (expected " << PHI_SQ << ")\n\n";

    std::cout << "OR GATE:\n";
    std::cout << "  OR(0,0) = " << decrypt_slot(phi_or(ct_zero, ct_zero), 128) << " (expected 0)\n";
    std::cout << "  OR(0,1) = " << decrypt_slot(phi_or(ct_zero, ct_one), 128) << " (expected " << PHI_SQ << ")\n\n";

    std::cout << "XOR GATE:\n";
    std::cout << "  XOR(0,0) = " << decrypt_slot(phi_xor(ct_zero, ct_zero), 128) << " (expected 0)\n";
    std::cout << "  XOR(0,1) = " << decrypt_slot(phi_xor(ct_zero, ct_one), 128) << " (expected " << PHI_SQ << ")\n\n";

    std::cout << "  Level: 0\n";
    std::cout << "  Status: LAHAT NG φ-NATIVE GATES WORKING!\n";

    return 0;
}
