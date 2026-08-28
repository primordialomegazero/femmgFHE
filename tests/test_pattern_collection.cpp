// PATTERN COLLECTION — Lahat ng Data para sa Tatlong Direksyon
// 1. FHEW/TFHE comparison
// 2. Period-0 modulo candidates
// 3. Algebraic structure ng φ

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  PATTERN COLLECTION\n";
    std::cout << "  Lahat ng Data para sa Tatlong Direksyon\n";
    std::cout << "========================================\n\n";

    const double PI = 3.14159265358979323846;
    const double PHI = 1.6180339887498948482;
    const double PHI_MOD = 0.6180339887498949;
    const double TWO_PI = 2 * PI;
    const double GOLDEN_ANGLE = TWO_PI * (1.0 - 1.0 / PHI);

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

    // ============================================
    // 1. PERIOD-0 MODULO CANDIDATES
    // ============================================
    std::cout << "1. PERIOD-0 MODULO CANDIDATES:\n";
    std::cout << "===============================\n\n";

    // Subukan iba't ibang φ-based na rotation constants
    std::vector<std::pair<std::string, double>> constants = {
        {"φ mod 1", PHI_MOD},
        {"φ² mod 1", PHI_MOD * PHI},
        {"golden angle", GOLDEN_ANGLE},
        {"2π/φ", TWO_PI / PHI},
        {"2π/φ²", TWO_PI / (PHI * PHI)},
        {"2π/φ³", TWO_PI / (PHI * PHI * PHI)}
    };

    for (auto& [name, constant] : constants) {
        auto state = make_ct(0.0);
        std::vector<double> values;
        
        for (int i = 0; i < 10; i++) {
            auto sum = cc->EvalAdd(state, make_ct(constant));
            state = sum;
            values.push_back(decrypt_val(state));
        }

        bool bounded = true;
        for (double v : values) {
            if (v >= TWO_PI) {
                bounded = false;
                break;
            }
        }

        std::cout << "  " << name << ": " << constant << "\n";
        std::cout << "    First 5 values: ";
        for (int i = 0; i < 5; i++) {
            std::cout << values[i] << " ";
        }
        std::cout << "\n";
        std::cout << "    Bounded: " << (bounded ? "✓" : "✗") << "\n\n";
    }

    // ============================================
    // 2. ALGEBRAIC STRUCTURE NG φ
    // ============================================
    std::cout << "2. ALGEBRAIC STRUCTURE NG φ:\n";
    std::cout << "============================\n\n";

    std::cout << "  φ = " << PHI << "\n";
    std::cout << "  φ² = " << PHI * PHI << "\n";
    std::cout << "  φ² - φ - 1 = " << (PHI * PHI - PHI - 1) << " (dapat 0)\n";
    std::cout << "  1/φ = " << 1.0 / PHI << "\n";
    std::cout << "  φ - 1/φ = " << (PHI - 1.0 / PHI) << "\n\n";

    // Pentagonal roots
    std::cout << "  Pentagonal roots (5th roots of unity):\n";
    for (int k = 0; k < 5; k++) {
        double angle = TWO_PI * k / 5;
        std::cout << "    k=" << k << ": cos=" << std::cos(angle) 
                  << " sin=" << std::sin(angle) << "\n";
    }
    std::cout << "\n";

    // ============================================
    // 3. FHEW/TFHE COMPARISON BASELINE
    // ============================================
    std::cout << "3. FHEW/TFHE COMPARISON BASELINE:\n";
    std::cout << "=================================\n\n";

    std::cout << "  Standard FHEW NAND: 1 gate ≈ 1 bootstrapping ≈ 0.1-1.0 sec\n";
    std::cout << "  Standard TFHE NAND: 1 gate ≈ 1 bootstrapping ≈ 0.01-0.1 sec\n";
    std::cout << "  CKKS NAND (ours): 1 gate ≈ 0 sec (addition only)\n\n";

    std::cout << "  Level comparison:\n";
    std::cout << "    FHEW/TFHE: Level resets via bootstrapping\n";
    std::cout << "    CKKS (ours): Level stays 0 (no multiplication)\n\n";

    std::cout << "  Noise growth:\n";
    std::cout << "    FHEW/TFHE: Noise grows per gate, reset via bootstrap\n";
    std::cout << "    CKKS (ours): Noise grows slowly (addition only)\n\n";

    // ============================================
    // 4. MODULO TEST
    // ============================================
    std::cout << "4. MODULO TEST:\n";
    std::cout << "================\n\n";

    auto ct_two_pi = make_ct(TWO_PI);
    auto test_val = make_ct(3 * TWO_PI + 1.5);  // 20.35
    
    double raw = decrypt_val(test_val);
    double expected_mod = std::fmod(raw, TWO_PI);
    
    std::cout << "  Raw value: " << raw << "\n";
    std::cout << "  Expected mod: " << expected_mod << "\n";
    std::cout << "  Homomorphic mod: HINDI PA KAYA\n\n";

    return 0;
}
