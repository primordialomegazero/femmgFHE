// φ-CONJUGATE REFRESH — Noise Reset Verification
// Ang tanong: Ang φ-conjugate refresh ba ay nagre-reset ng noise?
//
// TEST:
// 1. Run NAND chain hanggang level ~5 (45 gates sa depth 50)
// 2. Apply φ-conjugate refresh (1 multiplication)
// 3. I-measure ang noise pagkatapos ng refresh
// 4. I-continue ang NAND chain
// 5. I-compare sa walang refresh

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  φ-REFRESH NOISE RESET TEST\n";
    std::cout << "========================================\n\n";

    // Depth 60 para sa 50 gates + 10 refresh margin
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

    // Binary encoding
    std::vector<std::complex<double>> v1(slots, {0.0, 0.0});
    v1[0] = {1.0, 0.0};

    auto ct1 = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(v1));

    // φ-conjugate: ψ^10 ≈ 0.00813
    const double psi_10 = std::pow(1.0/1.6180339887498948482, 10.0);
    std::vector<std::complex<double>> vec_psi(slots, {0.0, 0.0});
    vec_psi[0] = {psi_10, 0.0};
    auto ct_psi = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec_psi));

    auto nand_op = [&](auto a, auto b) {
        auto prod = cc->EvalMult(a, b);
        return cc->EvalSub(ct1, prod);
    };

    auto conjugate_refresh = [&](auto ct) {
        return cc->EvalMult(ct, ct_psi);
    };

    auto decrypt_val = [&](auto ct) {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetCKKSPackedValue()[0].real();
    };

    std::cout << "ψ^10 = " << psi_10 << "\n\n";

    // ============================================
    // TEST 1: REFRESH SA SIMULA (walang noise)
    // ============================================
    std::cout << "TEST 1: REFRESH SA FRESH CIPHERTEXT\n";
    std::cout << "=====================================\n\n";

    // refresh(1) = 1 · ψ^10 = ψ^10
    auto refreshed_fresh = conjugate_refresh(ct1);
    double val_fresh = decrypt_val(refreshed_fresh);
    std::cout << "  refresh(1) = " << val_fresh << " (expected " << psi_10 << ")\n";
    std::cout << "  Match: " << (std::abs(val_fresh - psi_10) < 0.01 ? "YES ✓" : "NO ✗") << "\n\n";

    // ============================================
    // TEST 2: REFRESH PAGKATAPOS NG 20 GATES
    // ============================================
    std::cout << "TEST 2: REFRESH PAGKATAPOS NG 20 GATES\n";
    std::cout << "========================================\n\n";

    // Chain ng 20 gates (level: 60 → 40)
    auto current = ct1;
    for (int i = 0; i < 20; i++) {
        current = nand_op(current, current);
    }

    // Expected value pagkatapos ng 20 gates
    double val_before_refresh = decrypt_val(current);
    int expected_before = (20 % 2 == 0) ? 1 : 0;
    std::cout << "  After 20 gates: val=" << val_before_refresh
              << " expected=" << expected_before << "\n";

    // Apply refresh
    auto refreshed_after_20 = conjugate_refresh(current);
    double val_after_refresh = decrypt_val(refreshed_after_20);

    // Ang refresh(0) = 0·ψ^10 = 0
    // Ang refresh(1) = 1·ψ^10 = ψ^10
    double expected_after_refresh = (expected_before == 1) ? psi_10 : 0.0;

    std::cout << "  After refresh: val=" << val_after_refresh
              << " expected=" << expected_after_refresh << "\n";
    std::cout << "  Match: " << (std::abs(val_after_refresh - expected_after_refresh) < 0.01 ? "YES ✓" : "NO ✗") << "\n\n";

    // ============================================
    // TEST 3: CHAIN + REFRESH + CHAIN
    // ============================================
    std::cout << "TEST 3: 20 GATES + REFRESH + 20 GATES\n";
    std::cout << "=======================================\n\n";

    // Chain 1: 20 gates
    auto chain1 = ct1;
    for (int i = 0; i < 20; i++) {
        chain1 = nand_op(chain1, chain1);
    }

    // Refresh
    auto refreshed = conjugate_refresh(chain1);

    // Chain 2: 20 more gates (pero ang scale ay iba na)
    // Ang refreshed value ay ψ^10 o 0, hindi 1 o 0
    // Kailangan i-adjust ang NAND para sa ψ^10 scale
    // NAND sa ψ^10 scale: ψ^10 - a·b·(1/ψ^10)
    // Pero para sa test, i-check lang natin ang value

    auto chain2 = refreshed;
    for (int i = 0; i < 20; i++) {
        chain2 = nand_op(chain2, chain2);
    }

    double val_chain2 = decrypt_val(chain2);
    std::cout << "  After chain 2: " << val_chain2 << "\n";
    std::cout << "  (Ang scale ay ψ^10-based, hindi 1-based)\n";
    std::cout << "  → Kailangan ng scale-aware NAND para dito\n";

    return 0;
}
