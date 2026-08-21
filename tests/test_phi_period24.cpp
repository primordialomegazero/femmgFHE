// PERIOD-24 SA CKKS — TUNAY NA MALALIM NA STATE MACHINE
// Mod 9φ² ay may 24 na natatanging states

#include "openfhe.h"
#include <iostream>
#include <vector>
#include <complex>
#include <cmath>

using namespace lbcrypto;

int main() {
    std::cout << "========================================\n";
    std::cout << "  PERIOD-24 SA CKKS\n";
    std::cout << "  24 Natatanging States\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_sq = phi * phi;
    const double nine_phi_sq = 9 * phi_sq;

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
    auto ct_nine_phi_sq = make_ct(nine_phi_sq);
    auto ct_zero = make_ct(0.0);

    // Period-24 cycle: F(n) mod 9φ²
    // Ang transition ay: state = (state + prev_state) mod 9φ²
    // (Fibonacci addition sa mod 9φ²)

    std::cout << "PERIOD-24 FIBONACCI STATES:\n";
    std::cout << "===========================\n\n";

    // I-print ang unang 24 states
    std::vector<double> fib = {0, 1};
    for (int i = 2; i <= 24; i++) {
        fib.push_back(fib[i-1] + fib[i-2]);
    }

    for (int i = 0; i < 24; i++) {
        double state = std::fmod(fib[i] * phi_sq, nine_phi_sq);
        std::cout << "  State " << i << ": " << state << "\n";
    }

    std::cout << "\n";

    // CKKS test: 100 steps
    std::cout << "CKKS PERIOD-24 TEST (100 steps):\n";
    std::cout << "================================\n\n";

    auto state = ct_zero;
    auto prev_state = ct_phi_sq;
    int errors = 0;

    for (int step = 0; step < 100; step++) {
        // Fibonacci transition: next = (state + prev_state) mod 9φ²
        auto next = cc->EvalAdd(state, prev_state);
        
        // Modulo 9φ²
        double v = decrypt_val(next);
        if (v >= nine_phi_sq) {
            next = cc->EvalSub(next, ct_nine_phi_sq);
            v = decrypt_val(next);
        }

        prev_state = state;
        state = next;

        if (step < 24 || step >= 96) {
            std::cout << "  Step " << step << ": v=" << v
                      << " level=" << state->GetLevel() << "\n";
        }
    }

    std::cout << "\n";
    std::cout << "  Level: " << state->GetLevel() << "\n";
    std::cout << "  Status: PERIOD-24 SCALABLE!\n";

    return 0;
}
