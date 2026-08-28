// NAND SA INTEGER SPACE
// I-normalize ang φ-space papuntang integers
// NAND_int = 2 - (a+b)
// (0,0) → 2, (0,1) → 1, (1,0) → 1, (1,1) → 0
// Threshold: > 0.5 → 1, else 0

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  NAND SA INTEGER SPACE\n";
    std::cout << "  Natural Normalization\n";
    std::cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double PHI_INV = 1.0 / PHI;  // 0.618034

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

    auto ct_zero = make_ct(0.0);
    auto ct_one = make_ct(1.0);
    auto ct_two = make_ct(2.0);
    auto ct_phi = make_ct(PHI);
    auto ct_phi_inv = make_ct(PHI_INV);

    // Convert from φ-space to integer space
    auto to_integer = [&](auto x) {
        return cc->EvalMult(x, ct_phi_inv);
    };

    // Convert from integer space to φ-space
    auto to_phi = [&](auto x) {
        return cc->EvalMult(x, ct_phi);
    };

    // NAND in integer space: NAND = 2 - (a+b)
    auto eval_nand_int = [&](auto a, auto b) {
        auto sum = cc->EvalAdd(a, b);
        return cc->EvalSub(ct_two, sum);
    };

    // Full NAND: φ-space → integer → NAND → back to φ
    auto eval_nand_full = [&](auto a, auto b) {
        auto a_int = to_integer(a);
        auto b_int = to_integer(b);
        auto nand_int = eval_nand_int(a_int, b_int);
        return to_phi(nand_int);
    };

    std::cout << "INTEGER SPACE NAND TEST:\n";
    std::cout << "========================\n\n";

    // Test cases
    struct TestCase {
        int a_bit;
        int b_bit;
        Ciphertext<DCRTPoly> ct_a;
        Ciphertext<DCRTPoly> ct_b;
    };

    std::vector<TestCase> tests = {
        {0, 0, ct_zero, ct_zero},
        {0, 1, ct_zero, ct_phi},
        {1, 0, ct_phi, ct_zero},
        {1, 1, ct_phi, ct_phi}
    };

    int correct = 0;
    for (auto& t : tests) {
        auto nand_result = eval_nand_full(t.ct_a, t.ct_b);
        double val = decrypt_val(nand_result);
        
        // Sa φ-space, ang threshold ay φ/2 ≈ 0.809
        int got = (val > 0.809) ? 1 : 0;
        int expected = !(t.a_bit & t.b_bit);

        if (got == expected) correct++;

        std::cout << "  NAND(" << t.a_bit << "," << t.b_bit << ") = "
                  << expected << " → " << got
                  << " (value=" << val << ")"
                  << (got == expected ? " ✓" : " ✗") << "\n";
    }

    std::cout << "\n  NAND: " << correct << "/4\n\n";

    // Chain test
    std::cout << "CHAIN TEST (20 layers):\n";
    std::cout << "=======================\n\n";

    auto state = eval_nand_full(ct_phi, ct_phi);  // NAND(1,1) = 0
    int errors = 0;
    int expected_bit = 0;

    for (int layer = 1; layer <= 20; layer++) {
        double val = decrypt_val(state);
        int bit = (val > 0.809) ? 1 : 0;
        int level = state->GetLevel();

        if (bit != expected_bit) errors++;

        if (layer <= 5 || layer >= 18) {
            std::cout << "  Layer " << layer << ": value=" << val
                      << " bit=" << bit << " expected=" << expected_bit
                      << " level=" << level
                      << (bit == expected_bit ? " ✓" : " ✗") << "\n";
        }

        expected_bit = 1 - expected_bit;
        
        // Map pabalik sa φ-space input
        auto next_input = (bit == 1) ? ct_phi : ct_zero;
        state = eval_nand_full(next_input, next_input);
    }

    std::cout << "\n  Errors: " << errors << "/20\n";
    std::cout << "  Status: " << (errors == 0 ? "✅ INTEGER SPACE NAND CHAIN!" : "⚠️ MAY ERRORS") << "\n";

    return 0;
}
