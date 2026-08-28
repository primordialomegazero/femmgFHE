// NAND VIA TOGGLE — NAND mula sa f(x) = 1 - x
// Ang toggle ay level 0 at bounded
// NAND = toggle ng sum

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  NAND VIA TOGGLE\n";
    std::cout << "  f(x) = 1 - x\n";
    std::cout << "========================================\n\n";

    const double PHI = 1.6180339887498948482;
    const double PSI = -0.6180339887498948482;

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
    auto ct_psi = make_ct(PSI);
    auto ct_one = make_ct(1.0);

    // Toggle: f(x) = 1 - x
    auto eval_toggle = [&](auto x) {
        return cc->EvalSub(ct_one, x);
    };

    // NAND gamit ang toggle:
    // NAND(a,b) = toggle(a + b - 1)
    // = 1 - (a + b - 1)
    // = 2 - a - b
    
    auto eval_nand = [&](auto a, auto b) {
        auto sum = cc->EvalAdd(a, b);
        auto minus_one = cc->EvalSub(sum, ct_one);
        return eval_toggle(minus_one);  // 1 - (a+b-1) = 2-a-b
    };

    std::cout << "NAND TEST:\n";
    std::cout << "==========\n\n";

    auto nand_phi_phi = eval_nand(ct_phi, ct_phi);
    auto nand_phi_psi = eval_nand(ct_phi, ct_psi);
    auto nand_psi_psi = eval_nand(ct_psi, ct_psi);

    std::cout << "  NAND(φ,φ) = " << decrypt_val(nand_phi_phi) << "\n";
    std::cout << "  NAND(φ,ψ) = " << decrypt_val(nand_phi_psi) << "\n";
    std::cout << "  NAND(ψ,ψ) = " << decrypt_val(nand_psi_psi) << "\n";
    std::cout << "  Level: " << nand_phi_phi->GetLevel() << "\n\n";

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
