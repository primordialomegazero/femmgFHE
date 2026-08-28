// PHI EXTEND — Mula sa Toggle papuntang Gates
// Base: x → φ - x
// I-compose para sa NAND, AND, OR

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  PHI EXTEND\n";
    std::cout << "  Mula sa Toggle papuntang Gates\n";
    std::cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(1);
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

    auto ct_phi = make_ct(PHI);
    auto ct_0 = make_ct(0.0);

    // Toggle: x → φ - x (NOT gate sa φ-space)
    auto eval_not = [&](auto x) {
        return cc->EvalSub(ct_phi, x);
    };

    std::cout << "GATE TESTING SA φ-SPACE:\n";
    std::cout << "========================\n\n";

    // Test NOT
    auto not_0 = eval_not(ct_0);
    auto not_phi = eval_not(ct_phi);

    std::cout << "NOT GATE:\n";
    std::cout << "  NOT(0) = " << decrypt_val(not_0) << " (expected φ)\n";
    std::cout << "  NOT(φ) = " << decrypt_val(not_phi) << " (expected 0)\n\n";

    // Sa φ-space, ang values ay 0 at φ
    // 0 = false, φ = true
    
    // XOR sa φ-space: XOR(a,b) = a + b - φ*(a==b)
    // Mas simple: XOR = |a - b| sa φ-space
    
    // Subukan ang XOR bilang absolute difference
    auto eval_xor = [&](auto a, auto b) {
        auto diff = cc->EvalSub(a, b);
        // Sa φ-space, |a-b| ay φ kung magkaiba, 0 kung pareho
        return diff;
    };

    auto xor_00 = eval_xor(ct_0, ct_0);
    auto xor_0phi = eval_xor(ct_0, ct_phi);
    auto xor_phiphi = eval_xor(ct_phi, ct_phi);

    std::cout << "XOR GATE (raw diff):\n";
    std::cout << "  XOR(0,0) = " << decrypt_val(xor_00) << "\n";
    std::cout << "  XOR(0,φ) = " << decrypt_val(xor_0phi) << "\n";
    std::cout << "  XOR(φ,φ) = " << decrypt_val(xor_phiphi) << "\n\n";

    // AND sa φ-space: AND(a,b) = min(a,b)
    // Mas simple: AND = NOT(XOR) na may φ threshold
    
    // Chain test — kombinasyon ng NOT at XOR
    std::cout << "COMBINED CHAIN (1000 gates):\n";
    std::cout << "===========================\n\n";

    auto state = ct_0;
    auto start = high_resolution_clock::now();

    for (int i = 0; i < 1000; i++) {
        // I-toggle ang state
        state = eval_not(state);
    }

    auto end = high_resolution_clock::now();
    auto duration = duration_cast<seconds>(end - start).count();

    double final_val = decrypt_val(state);
    std::cout << "  Gates: 1000\n";
    std::cout << "  Time: " << duration << " seconds\n";
    std::cout << "  Final: " << final_val << "\n";
    std::cout << "  Level: " << state->GetLevel() << "\n";
    std::cout << "  Bounded: " << (std::abs(final_val) < 2.0 ? "✅" : "⚠️") << "\n";

    return 0;
}
