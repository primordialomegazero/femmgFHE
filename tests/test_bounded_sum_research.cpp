// BOUNDED SUM RESEARCH
// Hanapin ang formula na hindi lumalaki

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  BOUNDED SUM RESEARCH\n";
    std::cout << "  Iba't ibang formula para sa neighbors\n";
    std::cout << "========================================\n\n";

    const double phi_sq = 2.618033988749895;
    const double two_phi_sq = 2 * phi_sq;

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

    // Initial
    std::vector<std::complex<double>> init(slots, {0.0, 0.0});
    init[128] = {phi_sq, 0.0};
    auto state = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(init));

    // ============================================
    // FORMULA 1: Mod 2φ² (bounded oscillation)
    // ============================================
    std::cout << "FORMULA 1: state = 2φ² - (L + R)\n";
    std::cout << "-------------------------------------\n\n";

    auto state1 = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(init));
    auto ct_two = make_uniform(two_phi_sq);

    int errors1 = 0;
    for (int step = 0; step < 20; step++) {
        auto left = cc->EvalAtIndex(state1, -1);
        auto right = cc->EvalAtIndex(state1, 1);
        auto sum = cc->EvalAdd(left, right);
        state1 = cc->EvalSub(ct_two, sum);
        
        double v = decrypt_slot(state1, 128);
        bool bounded = (std::abs(v) < 3 * phi_sq);
        if (!bounded) errors1++;
    }
    std::cout << "  Bounded: " << (errors1 == 0 ? "YES" : "NO") << "\n\n";

    // ============================================
    // FORMULA 2: K - (L + R) na may K = φ²
    // ============================================
    std::cout << "FORMULA 2: state = φ² - (L + R)\n";
    std::cout << "--------------------------------\n\n";

    auto state2 = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(init));
    auto ct_phi = make_uniform(phi_sq);

    int errors2 = 0;
    for (int step = 0; step < 20; step++) {
        auto left = cc->EvalAtIndex(state2, -1);
        auto right = cc->EvalAtIndex(state2, 1);
        auto sum = cc->EvalAdd(left, right);
        state2 = cc->EvalSub(ct_phi, sum);
        
        double v = decrypt_slot(state2, 128);
        bool bounded = (std::abs(v) < 3 * phi_sq);
        if (!bounded) errors2++;
    }
    std::cout << "  Bounded: " << (errors2 == 0 ? "YES" : "NO") << "\n\n";

    // ============================================
    // FORMULA 3: (L + R) / 2 gamit pre-scaling
    // ============================================
    std::cout << "FORMULA 3: state = φ² - (L + R)/2\n";
    std::cout << "----------------------------------\n\n";

    // I-encode ang state na may half scale
    std::vector<std::complex<double>> init_half(slots, {0.0, 0.0});
    init_half[128] = {phi_sq / 2.0, 0.0};
    auto state3 = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(init_half));
    auto ct_phi_half = make_uniform(phi_sq / 2.0);

    int errors3 = 0;
    for (int step = 0; step < 20; step++) {
        auto left = cc->EvalAtIndex(state3, -1);
        auto right = cc->EvalAtIndex(state3, 1);
        auto sum = cc->EvalAdd(left, right);
        state3 = cc->EvalSub(ct_phi_half, sum);
        
        double v = decrypt_slot(state3, 128);
        bool bounded = (std::abs(v) < 2 * phi_sq);
        if (!bounded) errors3++;
    }
    std::cout << "  Bounded: " << (errors3 == 0 ? "YES" : "NO") << "\n\n";

    // ============================================
    // FORMULA 4: XOR-style na may alternating sign
    // ============================================
    std::cout << "FORMULA 4: state = φ² - |L - R|\n";
    std::cout << "--------------------------------\n\n";

    auto state4 = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(init));

    int errors4 = 0;
    for (int step = 0; step < 20; step++) {
        auto left = cc->EvalAtIndex(state4, -1);
        auto right = cc->EvalAtIndex(state4, 1);
        auto diff = cc->EvalSub(left, right);
        state4 = cc->EvalSub(ct_phi, diff);
        
        double v = decrypt_slot(state4, 128);
        bool bounded = (std::abs(v) < 3 * phi_sq);
        if (!bounded) errors4++;
    }
    std::cout << "  Bounded: " << (errors4 == 0 ? "YES" : "NO") << "\n\n";

    std::cout << "========================================\n";
    std::cout << "  SUMMARY:\n";
    std::cout << "  Formula 1 (2φ² - L - R): " << (errors1 == 0 ? "BOUNDED" : "UNBOUNDED") << "\n";
    std::cout << "  Formula 2 (φ² - L - R): " << (errors2 == 0 ? "BOUNDED" : "UNBOUNDED") << "\n";
    std::cout << "  Formula 3 (φ²/2 - L - R): " << (errors3 == 0 ? "BOUNDED" : "UNBOUNDED") << "\n";
    std::cout << "  Formula 4 (φ² - |L-R|): " << (errors4 == 0 ? "BOUNDED" : "UNBOUNDED") << "\n";
    std::cout << "========================================\n";

    return 0;
}
