// φ-REDUCTION SA APPLICATION LEVEL
// Subukan kung kaya nating i-implement ang φ-reduction
// nang hindi binabago ang OpenFHE source

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
    std::cout << "  φ-REDUCTION APPLICATION\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(30);
    params.SetScalingModSize(40);
    params.SetBatchSize(256);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(params);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    cc->Enable(ADVANCEDSHE);

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

    // Standard NAND
    auto nand_std = [&](auto a, auto b) {
        auto prod = cc->EvalMult(a, b);
        return cc->EvalSub(ct_one, prod);
    };

    // Subukan natin ang φ-reduction:
    // Sa halip na ModReduce ng 1 level, subukan ang mas maliit
    // na reduction sa pamamagitan ng scaling

    std::cout << "TEST: 20 GATES na may φ-reduction attempts\n";
    std::cout << "==========================================\n\n";

    auto current = make_ct(1.0);
    int errors = 0;
    int total_gates = 20;

    for (int gate = 0; gate < total_gates; gate++) {
        current = nand_std(current, current);

        double v = decrypt_val(current);
        int expected = (gate % 2 == 0) ? 0 : 1;
        int got = (std::abs(v) > 0.5) ? 1 : 0;
        if (got != expected) errors++;

        if (gate < 5 || gate >= total_gates - 3) {
            std::cout << "  Gate " << gate << ": v=" << v
                      << " exp=" << expected
                      << " level=" << current->GetLevel()
                      << (got == expected ? " ✓" : " ✗") << "\n";
        }
    }

    std::cout << "\n  Errors: " << errors << "/" << total_gates << "\n";
    std::cout << "  Level per gate: " << (double)current->GetLevel() / total_gates << "\n";

    return 0;
}
