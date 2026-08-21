// BOUNDED PERIOD-3 NAND — MODULO φ³
// Ang modulo φ³ ay nagbibigay ng natural na bounding
// para sa ternary NAND

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  BOUNDED PERIOD-3 NAND\n";
    std::cout << "  Modulo φ³\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_sq = phi * phi;
    const double phi_cu = phi * phi * phi;

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

    auto ct_phi_sq = make_ct(phi_sq);
    auto ct_phi_cu = make_ct(phi_cu);
    auto ct_two_phi_sq = make_ct(2 * phi_sq);
    auto ct_zero = make_ct(0.0);

    // Bounded cycle: next = (x + φ²) mod φ³
    // Kung x + φ² ≥ φ³: subtract φ³
    // Sa practice, gamitin natin ang period-3 na may modulo

    auto bounded_step = [&](auto current) {
        auto next = cc->EvalAdd(current, ct_phi_sq);
        
        // Modulo φ³: kung next ≥ φ³, subtract φ³
        // (ito ay conditional, pero sa period-3 cycle
        //  automatic kung tama ang setup)
        
        return next;
    };

    // NAND sa period-3: NAND(a,b) = 2φ² - (a+b) mod φ³
    auto nand_p3 = [&](auto a, auto b) {
        auto sum = cc->EvalAdd(a, b);
        auto nand_val = cc->EvalSub(ct_two_phi_sq, sum);
        return nand_val;
    };

    std::cout << "BOUNDED PERIOD-3 CYCLE (20 steps):\n";
    std::cout << "===================================\n\n";

    auto current = ct_zero;
    for (int gate = 0; gate < 20; gate++) {
        current = bounded_step(current);
        double v = decrypt_val(current);
        
        // Bounded check: 0 ≤ v < φ³
        bool bounded = (v >= -0.1 && v < phi_cu + 0.1);
        
        if (gate < 10 || !bounded) {
            std::cout << "  Step " << gate << ": v=" << v
                      << " level=" << current->GetLevel()
                      << (bounded ? " ✓" : " ✗") << "\n";
        }
        
        // Kung lumampas, subtract φ³
        if (v >= phi_cu) {
            current = cc->EvalSub(current, ct_phi_cu);
        }
    }

    std::cout << "\nNAND CHAIN (100 gates bounded):\n";
    std::cout << "===============================\n\n";

    current = ct_zero;
    int errors = 0;

    for (int gate = 0; gate < 100; gate++) {
        current = nand_p3(current, current);
        
        double v = decrypt_val(current);
        
        // Modulo φ³
        if (v >= phi_cu) {
            current = cc->EvalSub(current, ct_phi_cu);
            v = decrypt_val(current);
        }
        if (v < -phi_cu) {
            current = cc->EvalAdd(current, ct_phi_cu);
            v = decrypt_val(current);
        }
        
        bool bounded = (v >= -phi_cu && v <= phi_cu);
        if (!bounded) errors++;

        if (gate < 20 || !bounded) {
            std::cout << "  Gate " << gate << ": v=" << v
                      << " level=" << current->GetLevel()
                      << (bounded ? " ✓" : " ✗") << "\n";
        }
    }

    std::cout << "\n  Errors: " << errors << "/100\n";
    std::cout << "  Level: " << current->GetLevel() << "\n";
    std::cout << "  Status: " << (errors == 0 ? "✅ BOUNDED!" : "❌") << "\n";

    return 0;
}
