// PERIOD-0 BOUNDED — φ mod 1 Rotation
// Subukan kung ang step na φ mod 1 ay kayang manatiling bounded
// nang walang explicit modulo

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  PERIOD-0 BOUNDED — φ mod 1\n";
    std::cout << "  Natural Boundedness Test\n";
    std::cout << "========================================\n\n";

    const double PHI_MOD = 0.6180339887498949;

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

    auto ct_phi_mod = make_ct(PHI_MOD);

    std::cout << "PERIOD-0 ROTATION TEST (100 steps):\n";
    std::cout << "===================================\n\n";

    auto state = make_ct(0.0);
    bool bounded = true;

    for (int i = 1; i <= 100; i++) {
        state = cc->EvalAdd(state, ct_phi_mod);
        double v = decrypt_val(state);

        if (v >= 1.0) {
            std::cout << "  Step " << i << ": LUMAGPAS SA [0,1] — value=" << v << "\n";
            bounded = false;
            break;
        }

        if (i <= 5 || i >= 90) {
            std::cout << "  Step " << i << ": value=" << v << "\n";
        }
    }

    std::cout << "\n  Bounded: " << (bounded ? "✓ YES (dahil φ mod 1 ay nasa [0,1])" : "✗ NO") << "\n";
    std::cout << "  Note: Kahit bounded, lumalaki pa rin ang value\n";
    std::cout << "  Ang modulo ay HINDI pa rin automatic\n";

    return 0;
}
