// NEIGHBOR ENCODING EMERGENCE
// Hanapin kung may natural na paraan para ma-encode
// ang neighbor interaction nang walang sum

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  NEIGHBOR ENCODING EMERGENCE\n";
    std::cout << "  Natural Neighbor Interaction Search\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_sq = phi * phi;
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
    cc->EvalAtIndexKeyGen(keys.secretKey, {1, -1, 2, -2});
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
    // EMERGENT IDEA 1: XOR via Rotation Difference
    // ============================================
    std::cout << "IDEA 1: Rotation Difference (XOR-like)\n";
    std::cout << "  next = L - R (difference ng neighbors)\n\n";

    std::vector<std::complex<double>> init(slots, {0.0, 0.0});
    init[128] = {phi_sq, 0.0};
    auto state1 = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(init));

    int errors1 = 0;
    for (int step = 0; step < 20; step++) {
        auto left = cc->EvalAtIndex(state1, -1);
        auto right = cc->EvalAtIndex(state1, 1);
        state1 = cc->EvalSub(left, right);
        
        double v = decrypt_slot(state1, 128);
        bool bounded = (std::abs(v) < 3 * phi_sq);
        if (!bounded) errors1++;
    }
    std::cout << "  Bounded: " << (errors1 == 0 ? "YES" : "NO") << "\n\n";

    // ============================================
    // EMERGENT IDEA 2: φ² - (L - R) (centered diff)
    // ============================================
    std::cout << "IDEA 2: φ² - (L - R)\n";
    std::cout << "  Centered difference na may φ offset\n\n";

    auto state2 = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(init));
    auto ct_phi = make_uniform(phi_sq);

    int errors2 = 0;
    for (int step = 0; step < 20; step++) {
        auto left = cc->EvalAtIndex(state2, -1);
        auto right = cc->EvalAtIndex(state2, 1);
        auto diff = cc->EvalSub(left, right);
        state2 = cc->EvalSub(ct_phi, diff);
        
        double v = decrypt_slot(state2, 128);
        bool bounded = (std::abs(v) < 3 * phi_sq);
        if (!bounded) errors2++;
    }
    std::cout << "  Bounded: " << (errors2 == 0 ? "YES" : "NO") << "\n\n";

    // ============================================
    // EMERGENT IDEA 3: Phase shift (rotation only)
    // ============================================
    std::cout << "IDEA 3: Pure Rotation (phase shift)\n";
    std::cout << "  next = rotate(state) — walang sum\n\n";

    auto state3 = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(init));

    int errors3 = 0;
    for (int step = 0; step < 100; step++) {
        state3 = cc->EvalAtIndex(state3, 1);
        
        double v = decrypt_slot(state3, 128);
        bool bounded = (std::abs(v) < 3 * phi_sq);
        if (!bounded) errors3++;
    }
    std::cout << "  Bounded: " << (errors3 == 0 ? "YES" : "NO") << "\n\n";

    // ============================================
    // EMERGENT IDEA 4: K - (L XOR R) na φ-natural
    // ============================================
    std::cout << "IDEA 4: φ² - (L + R - φ²) — mod φ²\n";
    std::cout << "  Natural modular sum\n\n";

    auto state4 = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(init));

    int errors4 = 0;
    for (int step = 0; step < 20; step++) {
        auto left = cc->EvalAtIndex(state4, -1);
        auto right = cc->EvalAtIndex(state4, 1);
        auto sum = cc->EvalAdd(left, right);
        auto mod_sum = cc->EvalSub(sum, ct_phi);
        state4 = cc->EvalSub(ct_phi, mod_sum);
        
        double v = decrypt_slot(state4, 128);
        bool bounded = (std::abs(v) < 3 * phi_sq);
        if (!bounded) errors4++;
    }
    std::cout << "  Bounded: " << (errors4 == 0 ? "YES" : "NO") << "\n\n";

    // ============================================
    // SUMMARY
    // ============================================
    std::cout << "========================================\n";
    std::cout << "  SUMMARY:\n";
    std::cout << "  Idea 1 (L-R): " << (errors1 == 0 ? "BOUNDED" : "UNBOUNDED") << "\n";
    std::cout << "  Idea 2 (φ²-L+R): " << (errors2 == 0 ? "BOUNDED" : "UNBOUNDED") << "\n";
    std::cout << "  Idea 3 (rotation): " << (errors3 == 0 ? "BOUNDED" : "UNBOUNDED") << "\n";
    std::cout << "  Idea 4 (mod sum): " << (errors4 == 0 ? "BOUNDED" : "UNBOUNDED") << "\n";
    std::cout << "========================================\n";

    return 0;
}
