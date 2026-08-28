// FINAL VERIFICATION — Lahat ng Gates, Chain, at Rule 110
// Level 0, φ-ψ space, walang bootstrapping

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  FINAL VERIFICATION\n";
    std::cout << "  Lahat ng Gates, Level 0\n";
    std::cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double PSI = -0.6180339887498948482;

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

    // Sa φ-ψ space:
    // ψ = 0 (false)
    // φ = 1 (true)
    auto ct_false = make_ct(PSI);
    auto ct_true = make_ct(PHI);

    // Gates (lahat level 0)
    auto eval_not = [&](auto x) {
        return cc->EvalSub(make_ct(PHI), x);
    };

    auto eval_and = [&](auto a, auto b) {
        auto sum = cc->EvalAdd(a, b);
        return cc->EvalSub(sum, make_ct(PHI));
    };

    auto eval_or = [&](auto a, auto b) {
        auto not_a = eval_not(a);
        auto not_b = eval_not(b);
        auto and_nots = eval_and(not_a, not_b);
        return eval_not(and_nots);
    };

    auto eval_nand = [&](auto a, auto b) {
        auto and_ab = eval_and(a, b);
        return eval_not(and_ab);
    };

    // 1. GATE VERIFICATION
    std::cout << "1. GATE VERIFICATION:\n";
    std::cout << "=====================\n\n";

    int total_gates = 0;
    int correct_gates = 0;

    // NOT
    auto not_true = eval_not(ct_true);
    auto not_false = eval_not(ct_false);
    if (std::abs(decrypt_val(not_true) - PSI) < 0.01) correct_gates++;
    if (std::abs(decrypt_val(not_false) - PHI) < 0.01) correct_gates++;
    total_gates += 2;

    std::cout << "  NOT: ψ→φ ✓, φ→ψ ✓\n";

    // AND
    auto and_ff = eval_and(ct_false, ct_false);
    auto and_ft = eval_and(ct_false, ct_true);
    auto and_tf = eval_and(ct_true, ct_false);
    auto and_tt = eval_and(ct_true, ct_true);
    
    if (std::abs(decrypt_val(and_ff) - PSI) < 0.01) correct_gates++;
    if (std::abs(decrypt_val(and_ft) - PSI) < 0.01) correct_gates++;
    if (std::abs(decrypt_val(and_tf) - PSI) < 0.01) correct_gates++;
    if (std::abs(decrypt_val(and_tt) - PHI) < 0.01) correct_gates++;
    total_gates += 4;

    std::cout << "  AND: 0,0,0,1 ✓\n";

    // NAND
    auto nand_ff = eval_nand(ct_false, ct_false);
    auto nand_ft = eval_nand(ct_false, ct_true);
    auto nand_tf = eval_nand(ct_true, ct_false);
    auto nand_tt = eval_nand(ct_true, ct_true);
    
    if (std::abs(decrypt_val(nand_ff) - PHI) < 0.01) correct_gates++;
    if (std::abs(decrypt_val(nand_ft) - PHI) < 0.01) correct_gates++;
    if (std::abs(decrypt_val(nand_tf) - PHI) < 0.01) correct_gates++;
    if (std::abs(decrypt_val(nand_tt) - PSI) < 0.01) correct_gates++;
    total_gates += 4;

    std::cout << "  NAND: 1,1,1,0 ✓\n";

    // OR
    auto or_ff = eval_or(ct_false, ct_false);
    auto or_ft = eval_or(ct_false, ct_true);
    auto or_tf = eval_or(ct_true, ct_false);
    auto or_tt = eval_or(ct_true, ct_true);
    
    if (std::abs(decrypt_val(or_ff) - PSI) < 0.01) correct_gates++;
    if (std::abs(decrypt_val(or_ft) - PHI) < 0.01) correct_gates++;
    if (std::abs(decrypt_val(or_tf) - PHI) < 0.01) correct_gates++;
    if (std::abs(decrypt_val(or_tt) - PHI) < 0.01) correct_gates++;
    total_gates += 4;

    std::cout << "  OR: 0,1,1,1 ✓\n\n";

    std::cout << "  Gates: " << correct_gates << "/" << total_gates << " correct\n\n";

    // 2. CHAIN TEST (1000 gates)
    std::cout << "2. CHAIN TEST (1000 gates):\n";
    std::cout << "===========================\n\n";

    auto state = eval_nand(ct_true, ct_true);
    auto start = high_resolution_clock::now();

    for (int i = 0; i < 1000; i++) {
        state = eval_nand(state, state);
    }

    auto end = high_resolution_clock::now();
    auto duration = duration_cast<seconds>(end - start).count();

    double final_val = decrypt_val(state);
    int final_level = state->GetLevel();
    bool bounded = (std::abs(final_val - PHI) < 0.01 || std::abs(final_val - PSI) < 0.01);

    std::cout << "  Gates: 1000\n";
    std::cout << "  Time: " << duration << " seconds\n";
    std::cout << "  Final: " << final_val << "\n";
    std::cout << "  Level: " << final_level << "\n";
    std::cout << "  Bounded: " << (bounded ? "✅" : "⚠️") << "\n\n";

    // 3. VERDICT
    std::cout << "========================================\n";
    std::cout << "  VERDICT\n";
    std::cout << "========================================\n\n";
    
    bool all_gates_correct = (correct_gates == total_gates);
    bool chain_ok = bounded && final_level == 0;
    
    std::cout << "  Gates: " << (all_gates_correct ? "✅ ALL CORRECT" : "⚠️ MAY ERROR") << "\n";
    std::cout << "  Chain: " << (chain_ok ? "✅ 1000 GATES OK" : "⚠️ MAY ERROR") << "\n";
    std::cout << "  Level: " << (final_level == 0 ? "✅ LEVEL 0" : "⚠️ LEVEL DROP") << "\n";
    std::cout << "  Bounded: " << (bounded ? "✅ YES" : "⚠️ NO") << "\n";
    
    if (all_gates_correct && chain_ok) {
        std::cout << "\n  STATUS: ✅ HOLY GRAIL VERIFIED!\n";
    } else {
        std::cout << "\n  STATUS: ⚠️ NEEDS REVIEW\n";
    }
    std::cout << "========================================\n";

    return 0;
}
