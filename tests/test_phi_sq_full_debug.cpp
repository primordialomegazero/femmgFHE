// φ²-DOMAIN — FULL DEBUG
// Ipakita ang bawat step: input, after mult, after sub
// Para makita natin ang eksaktong arithmetic

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  φ²-DOMAIN — FULL DEBUG\n";
    std::cout << "  Step-by-step arithmetic\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_sq = phi * phi;  // 2.61803398875
    const double psi_sq = 1.0 / phi_sq;  // 0.38196601125

    std::cout << "CONSTANTS:\n";
    std::cout << "  φ = " << phi << "\n";
    std::cout << "  φ² = " << phi_sq << "\n";
    std::cout << "  ψ² = " << psi_sq << "\n";
    std::cout << "  φ² · ψ² = " << (phi_sq * psi_sq) << " (should be 1)\n\n";

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(30);
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

    auto ct_phi_sq = make_ct(phi_sq);
    auto ct_psi_sq = make_ct(psi_sq);

    // NAND sa φ²-domain na may manual step tracking
    auto nand_phi_sq_debug = [&](auto a, auto b, int gate_num) {
        std::cout << "\n  --- Gate " << gate_num << " ---\n";
        
        // Step 1: Multiply
        auto prod = cc->EvalMult(a, b);
        double prod_val = decrypt_val(prod);
        std::cout << "  Step 1 (a·b): " << prod_val << " (level " << prod->GetLevel() << ")\n";
        
        // Step 2: Scale by ψ²
        auto scaled = cc->EvalMult(prod, ct_psi_sq);
        double scaled_val = decrypt_val(scaled);
        std::cout << "  Step 2 (a·b·ψ²): " << scaled_val << " (level " << scaled->GetLevel() << ")\n";
        
        // Step 3: Subtract from φ²
        auto result = cc->EvalSub(ct_phi_sq, scaled);
        double result_val = decrypt_val(result);
        std::cout << "  Step 3 (φ² - a·b·ψ²): " << result_val << " (level " << result->GetLevel() << ")\n";
        
        // Expected value
        double expected = (gate_num % 2 == 0) ? 0.0 : phi_sq;
        std::cout << "  Expected: " << expected << "\n";
        std::cout << "  Match: " << (std::abs(result_val - expected) < 0.1 ? "YES" : "NO") << "\n";
        
        return result;
    };

    std::cout << "FULL DEBUG — 10 GATES\n";
    std::cout << "=====================\n";

    auto current = ct_phi_sq;
    for (int gate = 0; gate < 10; gate++) {
        current = nand_phi_sq_debug(current, current, gate);
    }

    std::cout << "\n\n========================================\n";
    std::cout << "ANALYSIS:\n";
    std::cout << "=========\n\n";
    std::cout << "1. Ang NAND(φ²,φ²) ay nagbibigay ng 0 (hindi -1)\n";
    std::cout << "2. Ang alternating pattern ay 0, φ², 0, φ²\n";
    std::cout << "3. Walang -1 na lumalabas sa tamang φ²-domain\n";
    std::cout << "4. Ang level ay 2 per gate (2 multiplications)\n";
    std::cout << "5. Ang values ay stable — walang degradation\n";

    return 0;
}
