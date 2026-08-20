// CKKS + φ EMERGENT PROPERTIES
// Hanapin kung may natural na depth extension sa φ-structure
//
// KEY HYPOTHESIS:
// Ang φ-NAND ay may period-2 property:
//   NAND(NAND(x,x), NAND(x,x)) = x
// Ibig sabihin: pagkatapos ng 2 gates, bumabalik sa original
//
// Kung ito ay totoo sa CKKS, ang noise ay maaaring self-limiting
// nang walang decryption

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  CKKS + φ EMERGENT PROPERTIES\n";
    std::cout << "  Period-2 sa Encrypted Domain\n";
    std::cout << "========================================\n\n";

    // Config: depth=30, scaling=40
    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(30);
    params.SetScalingModSize(40);
    params.SetBatchSize(2048);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(params);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);

    auto slots = cc->GetEncodingParams()->GetBatchSize();

    // ============================================
    // φ-STRUCTURE SA CKKS
    // ============================================
    const double phi_val = 1.6180339887498948482;
    const double inv_phi = 1.0 / phi_val;

    std::vector<std::complex<double>> vec_phi(slots, {0.0, 0.0});
    vec_phi[0] = {phi_val, 0.0};

    std::vector<std::complex<double>> vec_inv_phi(slots, {0.0, 0.0});
    vec_inv_phi[0] = {inv_phi, 0.0};

    std::vector<std::complex<double>> vec_zero(slots, {0.0, 0.0});
    std::vector<std::complex<double>> vec_one(slots, {0.0, 0.0});
    vec_one[0] = {1.0, 0.0};

    auto ct_phi = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec_phi));
    auto ct_inv_phi = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec_inv_phi));
    auto ct_zero = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec_zero));
    auto ct_one_const = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec_one));

    // φ-NAND: φ - a·b·φ⁻¹
    auto phi_nand = [&](auto a, auto b) {
        auto prod = cc->EvalMult(a, b);
        auto scaled = cc->EvalMult(prod, ct_inv_phi);
        return cc->EvalSub(ct_phi, scaled);
    };

    auto decrypt_val = [&](auto ct) {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetCKKSPackedValue()[0].real();
    };

    std::cout << "φ-Structure:\n";
    std::cout << "  φ = " << phi_val << "\n";
    std::cout << "  φ⁻¹ = " << inv_phi << "\n\n";

    // ============================================
    // TEST 1: PERIOD-2 PROPERTY SA CKKS
    // ============================================
    std::cout << "TEST 1: PERIOD-2 PROPERTY\n";
    std::cout << "==========================\n\n";
    std::cout << "  NOT(NOT(1)) = 1 ba sa encrypted domain?\n\n";

    // NOT(1) = NAND(1,1) = φ - 1·1·φ⁻¹ = φ - φ⁻¹ = 1/φ = ψ
    // NOT(NOT(1)) = NOT(ψ) = NAND(ψ,ψ) = φ - ψ²·φ⁻¹
    // = φ - ψ²/φ = φ - (ψ/φ)·ψ = φ - ψ·(1-φ)/φ
    // = φ - ψ·(1/φ - 1) = φ - ψ/φ + ψ
    // = φ + ψ - ψ/φ = 1 - ψ/φ
    // ψ/φ = (1-φ)/φ = 1/φ - 1
    // = 1 - (1/φ - 1) = 2 - 1/φ = 2 - 0.618 = 1.382
    // Hindi eksaktong 1, pero malapit

    auto ct_phi_one = ct_phi;  // φ represents "1"
    auto not1 = phi_nand(ct_phi_one, ct_phi_one);
    auto not2 = phi_nand(not1, not1);

    double val_not1 = decrypt_val(not1);
    double val_not2 = decrypt_val(not2);

    std::cout << "  NOT(φ) = " << val_not1 << "\n";
    std::cout << "  NOT(NOT(φ)) = " << val_not2 << "\n";
    std::cout << "  Expected φ: " << phi_val << "\n";
    std::cout << "  Match: " << (std::abs(val_not2 - phi_val) < 0.01 ? "YES" : "NO") << "\n\n";

    // ============================================
    // TEST 2: SELF-CORRECTING φ-NAND
    // ============================================
    std::cout << "TEST 2: SELF-CORRECTING φ-NAND\n";
    std::cout << "================================\n\n";
    std::cout << "  Pagkatapos ng bawat 2 gates, dapat bumalik sa φ\n\n";

    auto current = ct_phi_one;
    std::cout << "  Gate | Value | Expected | OK?\n";
    std::cout << "  -----|-------|----------|-----\n";

    for (int gate = 0; gate < 15; gate++) {
        current = phi_nand(current, current);
        double v = decrypt_val(current);
        double expected = (gate % 2 == 0) ? (phi_val - inv_phi) : phi_val;
        bool ok = std::abs(v - expected) < 0.1;

        if (gate < 8) {
            std::cout << "  " << gate << "    | " << v
                      << " | " << expected
                      << " | " << (ok ? "YES" : "NO") << "\n";
        }
    }

    // ============================================
    // TEST 3: DEPTH EXTENSION SA φ-NAND
    // ============================================
    std::cout << "\nTEST 3: DEPTH EXTENSION\n";
    std::cout << "========================\n\n";
    std::cout << "  Gaano karaming φ-NAND gates ang kaya?\n\n";

    for (int num_gates : {15, 20, 25, 29, 30, 31}) {
        std::cout << "  " << num_gates << " φ-NAND gates: ";
        std::cout.flush();

        try {
            auto cur = ct_phi_one;
            for (int i = 0; i < num_gates; i++) {
                cur = phi_nand(cur, cur);
            }
            Plaintext pt;
            cc->Decrypt(keys.secretKey, cur, &pt);
            double v = pt->GetCKKSPackedValue()[0].real();
            std::cout << "OK (val=" << v << ")\n";
        } catch (std::exception& e) {
            std::cout << "DEPTH EXCEEDED\n";
            break;
        }
    }

    // ============================================
    // TEST 4: φ-SCALED NAND (mas maliit na values)
    // ============================================
    std::cout << "\nTEST 4: φ-SCALED NAND\n";
    std::cout << "=======================\n\n";
    std::cout << "  Gamitin ang 0 at 1 na may φ-scaling sa decimal\n\n";

    // NAND(a,b) = 1 - a·b (standard binary sa CKKS)
    // Pero may φ-scaling sa encoding:
    // 0 → 0
    // 1 → φ (o 1/φ para sa mas maliit na values)

    const double scale = 1.0 / phi_val;  // ≈ 0.618

    std::vector<std::complex<double>> vec_scaled_one(slots, {0.0, 0.0});
    vec_scaled_one[0] = {scale, 0.0};

    auto ct_scaled_one = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec_scaled_one));

    // Scaled NAND: scale - a·b·scale⁻¹
    auto scaled_nand = [&](auto a, auto b) {
        auto prod = cc->EvalMult(a, b);
        auto inv_scale_ct = ct_phi;  // φ ≈ 1.618 = 1/0.618
        auto scaled_prod = cc->EvalMult(prod, inv_scale_ct);
        return cc->EvalSub(ct_scaled_one, scaled_prod);
    };

    std::cout << "  Scale = 1/φ = " << scale << "\n";
    std::cout << "  Scaled NAND chain (20 gates):\n\n";

    auto cur_scaled = ct_scaled_one;
    for (int gate = 0; gate < 20; gate++) {
        cur_scaled = scaled_nand(cur_scaled, cur_scaled);
        double v = decrypt_val(cur_scaled);
        double expected = (gate % 2 == 0) ? 0.0 : scale;
        bool ok = std::abs(v - expected) < 0.1;

        if (gate < 5) {
            std::cout << "  Gate " << gate << ": val=" << v
                      << " expected=" << expected
                      << (ok ? " YES" : " NO") << "\n";
        }
    }

    // Final check
    Plaintext pt_final;
    cc->Decrypt(keys.secretKey, cur_scaled, &pt_final);
    double final_val = pt_final->GetCKKSPackedValue()[0].real();
    std::cout << "\n  Final value after 20 gates: " << final_val << "\n";
    std::cout << "  Expected: " << (20 % 2 == 0 ? scale : 0.0) << "\n";

    return 0;
}
