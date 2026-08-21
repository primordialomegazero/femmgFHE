// φ²-DOMAIN + CONDITIONAL ADD/SUB
// Subukan kung kaya nating gawin ang NAND na
// addition/subtraction lang (0 levels)

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  φ²-DOMAIN + CONDITIONAL ADD/SUB\n";
    std::cout << "  0-Level NAND Attempt\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_sq = phi * phi;

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
    auto ct_zero = make_ct(0.0);

    std::cout << "STRATEGY:\n";
    std::cout << "  Kung current = φ²: ibalik sa 0 (subtract φ²)\n";
    std::cout << "  Kung current = 0: ibalik sa φ² (add φ²)\n";
    std::cout << "  Period-2: φ² → 0 → φ² → 0\n\n";

    std::cout << "PROBLEMA:\n";
    std::cout << "  Hindi natin alam kung ang current ay φ² o 0\n";
    std::cout << "  nang hindi nagde-decrypt. Kaya ang conditional\n";
    std::cout << "  add/sub ay hindi direktang possible.\n\n";

    std::cout << "PERO MAY TRICK:\n";
    std::cout << "  Kung gumamit tayo ng SIGN-BASED na operasyon:\n";
    std::cout << "  φ² + (-1)^n · φ² = 0 o φ²\n\n";

    // Subukan ang alternating add/sub
    auto current = ct_phi_sq;
    int errors = 0;
    int total_gates = 20;

    std::cout << "20 GATES — ALTERNATING ADD/SUB\n";
    std::cout << "==============================\n\n";

    for (int gate = 0; gate < total_gates; gate++) {
        if (gate % 2 == 0) {
            current = cc->EvalSub(current, ct_phi_sq);  // φ² → 0
        } else {
            current = cc->EvalAdd(current, ct_phi_sq);  // 0 → φ²
        }

        double v = decrypt_val(current);
        double expected = (gate % 2 == 0) ? 0.0 : phi_sq;
        bool ok = (std::abs(v - expected) < 0.15 * phi_sq);
        if (!ok) errors++;

        std::cout << "  Gate " << gate << ": v=" << v
                  << " exp=" << expected
                  << " level=" << current->GetLevel()
                  << (ok ? " ✓" : " ✗") << "\n";
    }

    std::cout << "\n  Errors: " << errors << "/" << total_gates << "\n";
    std::cout << "  Level per gate: " << (double)current->GetLevel() / total_gates << "\n";
    std::cout << "  (Kung 0 level per gate, HOLY GRAIL!)\n";

    return 0;
}
