// φ-LEVEL ECONOMY — CKKS IMPLEMENTATION
// Ang φ-level consumption (0.382/gate) ay maaaring ma-achieve
// kung ang modulus switching ay φ-proportional
//
// STRATEGY:
// - Gumamit ng φ-scaling sa bawat gate
// - Ang result ay may natural na φ-level preservation
// - Subukan sa 30 gates sa depth 12 (dapat kasya sa 0.382×30=11.5 levels)

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <chrono>

using namespace lbcrypto;
using namespace std::chrono;

int main() {
    std::cout << "========================================\n";
    std::cout << "  φ-LEVEL ECONOMY CKKS\n";
    std::cout << "  30 Gates sa Depth 12\n";
    std::cout << "========================================\n\n";

    std::cout << "THEORY:\n";
    std::cout << "  Level consumption = 0.382/gate\n";
    std::cout << "  30 gates × 0.382 = 11.5 levels\n";
    std::cout << "  Depth 12 ay dapat sapat!\n\n";

    // Depth 12 — mas mababa kaysa 30
    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(12);
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

    std::cout << "TEST: 30 GATES SA DEPTH 12\n";
    std::cout << "===========================\n\n";

    auto current = make_ct(1.0);
    int errors = 0;

    auto t_start = high_resolution_clock::now();

    try {
        for (int gate = 0; gate < 30; gate++) {
            current = nand_op(current, current);
            double v = decrypt_val(current);
            int expected = (gate % 2 == 0) ? 0 : 1;
            int got = (std::abs(v) > 0.5) ? 1 : 0;
            if (got != expected) errors++;

            if (gate % 5 == 0 || gate >= 25) {
                std::cout << "  Gate " << gate << ": v=" << v
                          << " level=" << current->GetLevel()
                          << (got == expected ? " ✓" : " ✗") << "\n";
            }
        }
    } catch (std::exception& e) {
        std::cout << "  FAILED sa gate " << errors << ": " << e.what() << "\n";
    }

    std::cout << "\n  Errors: " << errors << "/30\n";
    return 0;
}
