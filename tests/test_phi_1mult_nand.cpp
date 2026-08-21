// 1-MULT φ-DOMAIN NAND
// Subukan kung kaya nating gawing 1 multiplication
// ang NAND_φ sa pamamagitan ng pre-scaling

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <chrono>
#include <cmath>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  1-MULT φ-DOMAIN NAND\n";
    std::cout << "  Pre-Scaled Inputs\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double psi = 1.0 / phi;

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(80);
    params.SetScalingModSize(30);
    params.SetBatchSize(256);

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

    auto ct_phi = make_ct(phi);

    // ============================================
    // IDEA 1: Pre-scale ang inputs by ψ
    // ============================================
    // Kung ang inputs ay naka-store na may ψ-factor:
    //   a' = a·ψ
    //   b' = b·ψ
    // Pagkatapos: a'·b' = a·b·ψ²
    // At: φ - a·b·ψ = φ - (a'·b')/ψ = φ - a'·b'·φ
    // Hindi ito 1 mult — kailangan pa rin ng scaling

    // ============================================
    // IDEA 2: I-encode ang φ bilang 1
    // ============================================
    // Kung ang "true" ay 1 at "false" ay 0:
    // NAND(1,1) = 0, NAND(0,0) = 1, NAND(1,0) = 1
    // Formula: NAND(a,b) = 1 - a·b (1 mult!)
    // Ito ay standard NAND sa {0,1} encoding!

    // ============================================
    // TEST: Standard 1-mult NAND sa {0,1}
    // ============================================
    std::cout << "STANDARD NAND (1-MULT) SA {0,1}:\n";
    std::cout << "================================\n\n";

    auto ct_one = make_ct(1.0);
    auto ct_zero = make_ct(0.0);

    auto nand_standard = [&](auto a, auto b) {
        auto prod = cc->EvalMult(a, b);  // 1 mult!
        return cc->EvalSub(ct_one, prod); // 0 mults
    };

    // Truth table
    auto t00 = nand_standard(ct_zero, ct_zero);
    auto t01 = nand_standard(ct_zero, ct_one);
    auto t11 = nand_standard(ct_one, ct_one);

    std::cout << "  NAND(0,0) = " << decrypt_val(t00) << " (expected 1)\n";
    std::cout << "  NAND(0,1) = " << decrypt_val(t01) << " (expected 1)\n";
    std::cout << "  NAND(1,1) = " << decrypt_val(t11) << " (expected 0)\n\n";

    // ============================================
    // TEST: 80 GATES — 1 MULT EACH
    // ============================================
    std::cout << "80 GATES — 1 MULT EACH\n";
    std::cout << "=======================\n\n";

    auto current = ct_one;
    int errors = 0;
    int total_gates = 80;

    auto t_start = high_resolution_clock::now();

    for (int gate = 0; gate < total_gates; gate++) {
        current = nand_standard(current, current);

        double v = decrypt_val(current);
        double expected = (gate % 2 == 0) ? 0.0 : 1.0;
        bool ok = (std::abs(v - expected) < 0.15);
        if (!ok) errors++;

        if (gate % 10 == 0 || !ok) {
            auto t_now = high_resolution_clock::now();
            auto elapsed = duration_cast<seconds>(t_now - t_start).count();
            std::cout << "  Gate " << gate << ": v=" << v
                      << " exp=" << expected
                      << " level=" << current->GetLevel()
                      << " elapsed=" << elapsed << "s"
                      << (ok ? " ✓" : " ✗") << "\n";
        }
    }

    auto t_end = high_resolution_clock::now();
    auto total_ms = duration_cast<milliseconds>(t_end - t_start).count();

    std::cout << "\n========================================\n";
    std::cout << "  RESULT:\n";
    std::cout << "  Gates: " << total_gates << "\n";
    std::cout << "  Errors: " << errors << "\n";
    std::cout << "  Accuracy: " << (100.0 * (total_gates - errors) / total_gates) << "%\n";
    std::cout << "  Time: " << total_ms / 1000.0 << "s\n";
    std::cout << "  Per gate: " << (double)total_ms / total_gates << " ms\n";
    std::cout << "========================================\n";

    return 0;
}
