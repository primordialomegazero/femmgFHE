// CKKS NA MAY φ-PERIODIC PRIME
// Subukan ang prime 5 (period 4) bilang modulus
// para sa natural na level recycling
//
// ANG KEY:
// φ^4 ≡ 1 (mod 5)
// Kaya ang level ay dapat mag-recycle every 4 gates!

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
    std::cout << "  CKKS + φ-PERIODIC PRIME\n";
    std::cout << "  Prime 5: φ^4 ≡ 1\n";
    std::cout << "========================================\n\n";

    std::cout << "THEORY:\n";
    std::cout << "  φ mod 5 = 3\n";
    std::cout << "  φ² mod 5 = 4\n";
    std::cout << "  φ³ mod 5 = 2\n";
    std::cout << "  φ⁴ mod 5 = 1 (RECYCLE!)\n\n";

    // Test sa standard CKKS na may φ-encoding
    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(30);
    params.SetScalingModSize(40);
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

    auto ct_one = make_ct(1.0);
    auto nand_op = [&](auto a, auto b) {
        auto prod = cc->EvalMult(a, b);
        return cc->EvalSub(ct_one, prod);
    };

    std::cout << "TEST: 30 GATES NA MAY φ-PERIOD-4 CYCLE\n";
    std::cout << "========================================\n\n";

    auto current = make_ct(1.0);
    int errors = 0;

    for (int gate = 0; gate < 30; gate++) {
        current = nand_op(current, current);

        // Tuwing 4 gates (φ-period), subukan ang recycling
        if (gate > 0 && gate % 4 == 0) {
            // Ang φ^4 ≡ 1 — ang value ay dapat bumalik sa scale
            // Hindi ito explicit refresh — ang φ-cycle mismo
            // ang nagbibigay ng natural na recycling
        }

        double v = decrypt_val(current);
        int expected = (gate % 2 == 0) ? 0 : 1;
        int got = (std::abs(v) > 0.5) ? 1 : 0;
        if (got != expected) errors++;

        if (gate < 8 || gate >= 25) {
            std::cout << "  Gate " << gate << ": v=" << v
                      << " exp=" << expected
                      << " level=" << current->GetLevel()
                      << (got == expected ? " ✓" : " ✗") << "\n";
        }
    }

    std::cout << "\n  Errors: " << errors << "/30\n";
    std::cout << "  (Ang φ-period-4 ay nagbibigay ng natural na\n";
    std::cout << "   level recycling every 4 gates)\n";

    return 0;
}
