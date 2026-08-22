// NAND GATE — WALANG DECRYPT SA GITNA
// ct_result = NAND(ct_a, ct_b) — puro encrypted operations
// Walang Decrypt() bago mabuo ang ct_result

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  NAND GATE — WALANG DECRYPT SA GITNA\n";
    std::cout << "  Pure Encrypted Computation\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_sq = phi * phi;
    const double two_phi_sq = 2 * phi_sq;
    const double phi_mod = 0.6180339887498949;  // φ² mod 1
    const double THRESHOLD = 0.75;

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

    // Constants
    auto ct_phi_sq = make_ct(phi_sq);
    auto ct_two_phi_sq = make_ct(two_phi_sq);
    auto ct_phi_mod = make_ct(phi_mod);

    // ============================================
    // NAND GATE — PURE ENCRYPTED
    // WALANG DECRYPT SA GITNA
    // ============================================
    auto nand_encrypted = [&](auto ct_a, auto ct_b) {
        // Step 1: Sum ng dalawang ciphertext (encrypted)
        auto ct_sum = cc->EvalAdd(ct_a, ct_b);

        // Step 2: Period-0 irrational rotation (encrypted)
        auto ct_rotated = cc->EvalAdd(ct_sum, ct_phi_mod);

        // Step 3: Period-4 bounded transformation (encrypted)
        // NAND = 2φ² - rotated — walang decrypt!
        auto ct_result = cc->EvalSub(ct_two_phi_sq, ct_rotated);

        return ct_result;
    };

    std::cout << "PURE ENCRYPTED NAND TEST:\n";
    std::cout << "=========================\n\n";
    std::cout << "  Inputs: ct_a, ct_b (encrypted)\n";
    std::cout << "  Result: ct_result (encrypted)\n";
    std::cout << "  Walang Decrypt() bago mabuo ang ct_result!\n\n";

    // I-encrypt ang inputs — 0 o φ²
    auto ct_zero = make_ct(0.0);
    auto ct_one = make_ct(phi_sq);

    // Compute NAND para sa LAHAT ng combinations — encrypted
    auto ct_result_00 = nand_encrypted(ct_zero, ct_zero);
    auto ct_result_01 = nand_encrypted(ct_zero, ct_one);
    auto ct_result_10 = nand_encrypted(ct_one, ct_zero);
    auto ct_result_11 = nand_encrypted(ct_one, ct_one);

    // Saka lang i-decrypt ang result
    std::cout << "DECRYPT RESULTS (AFTER computation):\n";
    std::cout << "====================================\n\n";

    double n00 = decrypt_val(ct_result_00);
    double n01 = decrypt_val(ct_result_01);
    double n10 = decrypt_val(ct_result_10);
    double n11 = decrypt_val(ct_result_11);

    std::cout << "  NAND(0,0) = " << n00 << " (expected " << phi_sq << ")\n";
    std::cout << "  NAND(0,1) = " << n01 << " (expected " << phi_sq << ")\n";
    std::cout << "  NAND(1,0) = " << n10 << " (expected " << phi_sq << ")\n";
    std::cout << "  NAND(1,1) = " << n11 << " (expected 0)\n\n";

    // Accuracy check
    int correct = 0;
    if (std::abs(n00 - phi_sq) < 0.1) correct++;
    if (std::abs(n01 - phi_sq) < 0.1) correct++;
    if (std::abs(n10 - phi_sq) < 0.1) correct++;
    if (std::abs(n11) < 0.1) correct++;

    std::cout << "  Accuracy: " << correct << "/4\n";
    std::cout << "  Level: " << ct_result_00->GetLevel() << "\n";
    std::cout << "  Status: " << (correct == 4 ? "✅ NAND GATE FULLY ENCRYPTED!" : "❌") << "\n";
    std::cout << "========================================\n";

    return 0;
}
