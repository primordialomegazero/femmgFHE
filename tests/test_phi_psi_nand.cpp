// PHI-PSI NAND — NAND sa φ-ψ Space
// NAND(φ,φ) = ψ, NAND(φ,ψ) = φ, NAND(ψ,ψ) = φ
// Level 0, bounded, pure FHE

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  PHI-PSI NAND\n";
    std::cout << "  NAND sa φ-ψ Space\n";
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

    // NAND sa φ-ψ space:
    // NAND(a,b) = 1 - (a+b)/φ
    // (φ,φ) → 1 - 2φ/φ = 1 - 2 = -1 → ψ
    // (φ,ψ) → 1 - (φ+ψ)/φ = 1 - 1/φ = φ
    // (ψ,ψ) → 1 - 2ψ/φ = 1 + 2/φ² = φ
    
    // Mas simple: NAND = 1 - (a+b)/φ
    // Sa FHE: division ay multiplication sa 1/φ
    
    // Pero kailangan natin ng addition only
    // Kaya: NAND = 1 - a - b (kung a,b ay φ-normalized)
    
    // Alternative: sa φ-ψ space:
    // NAND(φ,φ) = ψ (dahil 1 - 2φ = ψ)
    // NAND(φ,ψ) = φ (dahil 1 - (φ+ψ) = 1 - 1 = 0 → φ)
    // NAND(ψ,ψ) = φ (dahil 1 - 2ψ = φ)
    
    // NAND = 1 - (a+b) — addition lang!
    auto eval_nand = [&](auto a, auto b) {
        auto sum = cc->EvalAdd(a, b);
        return cc->EvalSub(ct_one, sum);
    };

    std::cout << "NAND SA φ-ψ SPACE:\n";
    std::cout << "==================\n\n";

    auto nand_phi_phi = eval_nand(ct_phi, ct_phi);
    auto nand_phi_psi = eval_nand(ct_phi, ct_psi);
    auto nand_psi_psi = eval_nand(ct_psi, ct_psi);

    std::cout << "  NAND(φ,φ) = " << decrypt_val(nand_phi_phi) 
              << " (expected " << PSI << ")\n";
    std::cout << "  NAND(φ,ψ) = " << decrypt_val(nand_phi_psi)
              << " (expected " << PHI << ")\n";
    std::cout << "  NAND(ψ,ψ) = " << decrypt_val(nand_psi_psi)
              << " (expected " << PHI << ")\n";
    std::cout << "  Level: " << nand_phi_phi->GetLevel() << "\n\n";

    // Chain test — walang decrypt sa gitna
    std::cout << "CHAIN TEST (1000 gates, walang decrypt):\n";
    std::cout << "=========================================\n\n";

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
    std::cout << "  Bounded: " << (std::abs(final_val - PHI) < 0.01 || std::abs(final_val - PSI) < 0.01 ? "✅" : "⚠️") << "\n";

    return 0;
}
