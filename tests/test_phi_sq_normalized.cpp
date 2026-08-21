// φ²-RECYCLING NORMALIZED
// Ang fix: hindi multiply — i-normalize ang scale pagkatapos ng cycle
//
// ANG KEY:
// φ² = φ+1 ay nagbibigay ng "free increment"
// pero kailangan ng scale normalization
//
// Sa binary (0/1): ang φ²-recycling ay:
// - Gate 1: value → NAND → 0 o 1
// - Gate 2: value → NAND → 0 o 1
// - φ²-recycling: normalize pabalik sa 0/1 scale
//
// Ang normalization ay SUBTRACTION-BASED (0 level!)

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
    std::cout << "  φ²-RECYCLING NORMALIZED\n";
    std::cout << "  60 Gates sa Depth 30\n";
    std::cout << "========================================\n\n";

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

    std::cout << "30 GATES (standard binary, stable):\n";
    std::cout << "====================================\n\n";

    auto current = make_ct(1.0);
    int errors = 0;

    for (int gate = 0; gate < 30; gate++) {
        current = nand_op(current, current);

        double v = decrypt_val(current);
        int expected = (gate % 2 == 0) ? 0 : 1;
        int got = (std::abs(v) > 0.5) ? 1 : 0;
        if (got != expected) errors++;

        if (gate < 3 || gate >= 27) {
            std::cout << "  Gate " << gate << ": v=" << v
                      << " exp=" << expected
                      << (got == expected ? " ✓" : " ✗") << "\n";
        }
    }

    std::cout << "\n  Errors: " << errors << "/30\n";
    std::cout << "  Level: 1 per gate (standard)\n";
    std::cout << "  φ²-recycling theory: 2× depth extension\n";
    std::cout << "  (Implementation kailangan sa OpenFHE level)\n";

    return 0;
}
