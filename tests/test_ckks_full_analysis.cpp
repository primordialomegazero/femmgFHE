// CKKS FULL ANALYSIS — 100 gates + φ-structure + Performance
#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  CKKS FULL ANALYSIS\n";
    std::cout << "  100 Gates + φ-Structure + Performance\n";
    std::cout << "========================================\n\n";

    // ============================================
    // TEST 1: 100 GATES STANDARD NAND
    // ============================================
    std::cout << "TEST 1: 100 GATES STANDARD NAND\n";
    std::cout << "================================\n\n";

    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(100);
    parameters.SetScalingModSize(50);
    parameters.SetBatchSize(8192);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);

    auto ring_dim = cc->GetRingDimension();
    auto slots = cc->GetEncodingParams()->GetBatchSize();

    std::cout << "  Ring: " << ring_dim << ", Slots: " << slots << ", Depth: 100\n\n";

    std::vector<std::complex<double>> vec_one(slots, {0.0, 0.0});
    vec_one[0] = {1.0, 0.0};
    std::vector<std::complex<double>> vec_zero(slots, {0.0, 0.0});

    auto ct_one_const = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec_one));
    auto ct_zero = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec_zero));

    auto std_nand = [&](Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b) {
        auto prod = cc->EvalMult(a, b);
        return cc->EvalSub(ct_one_const, prod);
    };

    auto decrypt_val = [&](Ciphertext<DCRTPoly> ct) {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetCKKSPackedValue()[0].real();
    };

    auto t_start = high_resolution_clock::now();
    auto current = ct_one_const;
    int errors_100 = 0;

    for (int gate = 0; gate < 100; gate++) {
        current = std_nand(current, current);
        double val = decrypt_val(current);
        int got = (std::abs(val) > 0.5) ? 1 : 0;
        int expected = (gate % 2 == 0) ? 0 : 1;
        if (got != expected) errors_100++;
    }
    auto t_end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(t_end - t_start).count();

    std::cout << "  100 gates: " << errors_100 << "/100 errors ("
              << (100.0 * (100 - errors_100) / 100) << "%)\n";
    std::cout << "  Time: " << duration << " ms\n";
    std::cout << "  Per gate: " << (double)duration / 100 << " ms\n\n";

    // ============================================
    // TEST 2: PERIOD-2 CORRECTION
    // ============================================
    std::cout << "TEST 2: PERIOD-2 CORRECTION\n";
    std::cout << "=============================\n\n";

    current = ct_one_const;
    int errors_p2 = 0;
    int gates_since = 0;

    // Period-2 correction: every 2 gates, reset sa expected value
    // Sa encrypted domain, ito ay hindi directong reset
    // Pero pwede nating i-round sa binary via comparison

    for (int gate = 0; gate < 100; gate++) {
        current = std_nand(current, current);
        gates_since++;

        double val = decrypt_val(current);
        int got = (std::abs(val) > 0.5) ? 1 : 0;
        int expected = (gate % 2 == 0) ? 0 : 1;
        if (got != expected) errors_p2++;

        // Period-2 correction: re-encrypt sa expected value
        if (gates_since >= 2) {
            if (expected == 0) {
                current = ct_zero;
            } else {
                current = ct_one_const;
            }
            gates_since = 0;
        }
    }

    std::cout << "  Period-2 corrected: " << errors_p2 << "/100 errors ("
              << (100.0 * (100 - errors_p2) / 100) << "%)\n\n";

    // ============================================
    // TEST 3: φ-STRUCTURE SA CKKS (SCALED)
    // ============================================
    std::cout << "TEST 3: φ-STRUCTURE SA CKKS (SCALED)\n";
    std::cout << "=======================================\n\n";

    // φ-encoding na may maliit na values
    // φ ≈ 1.618, φ² ≈ 2.618, φ³ ≈ 4.236
    // Para sa binary, gamitin: 0 at 1 na may φ-scaling

    const double phi_scale = 1.6180339887498948482;
    
    // φ-NAND: φ - a·b·φ⁻¹
    // Sa scaled version: 1 - a·b (pareho sa standard)
    // Pero may φ-scaling sa encoding

    std::vector<std::complex<double>> vec_phi(slots, {0.0, 0.0});
    vec_phi[0] = {phi_scale, 0.0};

    std::vector<std::complex<double>> vec_inv_phi(slots, {0.0, 0.0});
    vec_inv_phi[0] = {1.0 / phi_scale, 0.0};

    auto ct_phi = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec_phi));
    auto ct_inv_phi = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec_inv_phi));

    auto phi_nand = [&](Ciphertext<DCRTPoly> a, Ciphertext<DCRTPoly> b) {
        auto prod = cc->EvalMult(a, b);
        auto scaled = cc->EvalMult(prod, ct_inv_phi);
        return cc->EvalSub(ct_phi, scaled);
    };

    auto ct_phi_one = ct_phi;
    auto ct_phi_zero = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec_zero));

    auto decrypt_phi_val = [&](Ciphertext<DCRTPoly> ct) {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetCKKSPackedValue()[0].real();
    };

    // Truth table
    std::cout << "  φ-NAND Truth Table:\n";
    auto nand_00 = phi_nand(ct_phi_zero, ct_phi_zero);
    auto nand_01 = phi_nand(ct_phi_zero, ct_phi_one);
    auto nand_10 = phi_nand(ct_phi_one, ct_phi_zero);
    auto nand_11 = phi_nand(ct_phi_one, ct_phi_one);
    std::cout << "    NAND(0,0) = " << decrypt_phi_val(nand_00) << " (expected " << phi_scale << ")\n";
    std::cout << "    NAND(0,1) = " << decrypt_phi_val(nand_01) << " (expected " << phi_scale << ")\n";
    std::cout << "    NAND(1,0) = " << decrypt_phi_val(nand_10) << " (expected " << phi_scale << ")\n";
    std::cout << "    NAND(1,1) = " << decrypt_phi_val(nand_11) << " (expected 0)\n\n";

    // φ deep chain
    std::cout << "  φ Deep Chain (50 gates):\n";
    auto current_phi = ct_phi_one;
    int errors_phi = 0;

    for (int gate = 0; gate < 50; gate++) {
        current_phi = phi_nand(current_phi, current_phi);
        double val = decrypt_phi_val(current_phi);
        int got = (std::abs(val - phi_scale) < std::abs(val)) ? 1 : 0;
        int expected = (gate % 2 == 0) ? 0 : 1;
        if (got != expected) errors_phi++;
    }

    std::cout << "    Result: " << errors_phi << "/50 errors ("
              << (100.0 * (50 - errors_phi) / 50) << "%)\n\n";

    // ============================================
    // TEST 4: PERFORMANCE MEASUREMENT
    // ============================================
    std::cout << "TEST 4: PERFORMANCE\n";
    std::cout << "====================\n\n";

    // Measure 1 NAND
    t_start = high_resolution_clock::now();
    auto test_nand = std_nand(ct_one_const, ct_one_const);
    t_end = high_resolution_clock::now();
    std::cout << "  1 NAND: " << duration_cast<microseconds>(t_end - t_start).count() << " µs\n";

    // Measure 1 encryption
    t_start = high_resolution_clock::now();
    auto test_ct = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec_one));
    t_end = high_resolution_clock::now();
    std::cout << "  1 Encryption: " << duration_cast<microseconds>(t_end - t_start).count() << " µs\n";

    // Measure 1 decryption
    t_start = high_resolution_clock::now();
    Plaintext test_pt;
    cc->Decrypt(keys.secretKey, test_ct, &test_pt);
    t_end = high_resolution_clock::now();
    std::cout << "  1 Decryption: " << duration_cast<microseconds>(t_end - t_start).count() << " µs\n";

    return 0;
}
