// NAND FROM NOT+XOR — Universal Gate sa φ-Space
// NOT(x) = φ - x
// XOR(a,b) = a - b (raw)
// NAND = NOT(AND) = NOT(NOT(XOR)) — kailangan ng tamang composition

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  NAND FROM NOT+XOR\n";
    std::cout << "  Universal Gate sa φ-Space\n";
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

    // NOT: x → φ - x
    auto eval_not = [&](auto x) {
        return cc->EvalSub(ct_phi, x);
    };

    // AND sa φ-space:
    // AND(a,b) = NOT(NOT(a) XOR NOT(b))
    // Sa φ-space: XOR(a,b) = a - b (raw)
    // NOT(a) = φ - a
    // NOT(b) = φ - b
    // XOR(NOT(a), NOT(b)) = (φ - a) - (φ - b) = b - a
    // NOT(XOR) = φ - (b - a) = φ + a - b
    
    auto eval_and = [&](auto a, auto b) {
        auto not_a = eval_not(a);
        auto not_b = eval_not(b);
        auto xor_nots = cc->EvalSub(not_a, not_b);  // XOR ng NOTs
        return eval_not(xor_nots);  // NOT ng XOR
    };

    // NAND = NOT(AND)
    auto eval_nand = [&](auto a, auto b) {
        auto and_ab = eval_and(a, b);
        return eval_not(and_ab);
    };

    std::cout << "GATE TESTS:\n";
    std::cout << "===========\n\n";

    // Test AND
    auto and_00 = eval_and(ct_0, ct_0);
    auto and_0phi = eval_and(ct_0, ct_phi);
    auto and_phiphi = eval_and(ct_phi, ct_phi);

    std::cout << "AND GATE:\n";
    std::cout << "  AND(0,0) = " << decrypt_val(and_00) << "\n";
    std::cout << "  AND(0,φ) = " << decrypt_val(and_0phi) << "\n";
    std::cout << "  AND(φ,φ) = " << decrypt_val(and_phiphi) << "\n\n";

    // Test NAND
    auto nand_00 = eval_nand(ct_0, ct_0);
    auto nand_0phi = eval_nand(ct_0, ct_phi);
    auto nand_phiphi = eval_nand(ct_phi, ct_phi);

    std::cout << "NAND GATE:\n";
    std::cout << "  NAND(0,0) = " << decrypt_val(nand_00) << "\n";
    std::cout << "  NAND(0,φ) = " << decrypt_val(nand_0phi) << "\n";
    std::cout << "  NAND(φ,φ) = " << decrypt_val(nand_phiphi) << "\n";
    std::cout << "  Level: " << nand_00->GetLevel() << "\n\n";

    // Chain test
    std::cout << "CHAIN TEST (1000 gates):\n";
    std::cout << "========================\n\n";

    auto state = eval_nand(ct_phi, ct_phi);
    auto start = high_resolution_clock::now();

    for (int i = 0; i < 1000; i++) {
        state = eval_nand(state, state);
    }

    auto end = high_resolution_clock::now();
    auto duration = duration_cast<seconds>(end - start).count();

    double final_val = decrypt_val(state);
    std::cout << "  Gates: 1000\n";
    std::cout << "  Time: " << duration << " seconds\n";
    std::cout << "  Final: " << final_val << "\n";
    std::cout << "  Level: " << state->GetLevel() << "\n";
    std::cout << "  Bounded: " << (std::abs(final_val) < 5.0 ? "✅" : "⚠️") << "\n";

    return 0;
}
