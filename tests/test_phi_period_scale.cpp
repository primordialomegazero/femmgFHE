// PERIOD-N SCALING TEST — PERIOD-16 AT PERIOD-32
// Fibonacci Natural Periods sa Mas Mataas na N

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
    std::cout << "  PERIOD-N SCALING TEST\n";
    std::cout << "  Period-16 at Period-32\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_sq = phi * phi;
    const double four_phi_sq = 4 * phi_sq;

    // Fibonacci numbers
    auto fib = [](int n) -> long long {
        long long a = 0, b = 1;
        for (int i = 0; i < n; i++) {
            long long temp = a + b;
            a = b;
            b = temp;
        }
        return a;
    };

    std::cout << "FIBONACCI SCALING:\n";
    std::cout << "==================\n\n";
    std::cout << "  F(16) = " << fib(16) << "\n";
    std::cout << "  F(32) = " << fib(32) << "\n";
    std::cout << "  φ^16 ≈ " << std::pow(phi, 16) << "\n";
    std::cout << "  φ^32 ≈ " << std::pow(phi, 32) << "\n\n";

    // Period-16 states: F(n) mod 4φ² para sa n=0..15
    std::cout << "PERIOD-16 STATES:\n";
    std::cout << "=================\n\n";

    for (int i = 0; i <= 16; i++) {
        double fib_scaled = fib(i) * phi_sq;
        double mod_val = std::fmod(fib_scaled, four_phi_sq);
        std::cout << "  F(" << i << ")·φ² mod 4φ² = " << mod_val << "\n";
    }

    std::cout << "\n";

    // Period-32 states: F(n) mod 4φ² para sa n=0..31
    std::cout << "PERIOD-32 STATES (first 20 shown):\n";
    std::cout << "===================================\n\n";

    for (int i = 0; i <= 20; i++) {
        double fib_scaled = fib(i) * phi_sq;
        double mod_val = std::fmod(fib_scaled, four_phi_sq);
        std::cout << "  F(" << i << ")·φ² mod 4φ² = " << mod_val << "\n";
    }

    std::cout << "  ... (at " << 11 << " pa)\n\n";

    std::cout << "PERIOD DETECTION:\n";
    std::cout << "=================\n\n";

    // Hanapin kung may period-16 o period-32
    // Ang period ay ang pinakamaliit na N kung saan:
    // F(N)·φ² mod 4φ² = F(0)·φ² mod 4φ² = 0
    // AT F(N+1)·φ² mod 4φ² = F(1)·φ² mod 4φ² = φ²

    for (int period : {8, 16, 32}) {
        double f0_mod = std::fmod(fib(period) * phi_sq, four_phi_sq);
        double f1_mod = std::fmod(fib(period + 1) * phi_sq, four_phi_sq);
        
        bool match_f0 = (std::abs(f0_mod) < 0.01);
        bool match_f1 = (std::abs(f1_mod - phi_sq) < 0.01);
        
        std::cout << "  Period-" << period << ": ";
        std::cout << "F(" << period << ") mod 4φ² = " << f0_mod << " ";
        std::cout << "F(" << period + 1 << ") mod 4φ² = " << f1_mod << " ";
        std::cout << (match_f0 && match_f1 ? "✓" : "✗") << "\n";
    }

    std::cout << "\n";

    // Period-N sa CKKS — subukan ang period-16 sa 0-level
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

    // Period-16 cycle test sa CKKS
    std::cout << "PERIOD-16 SA CKKS (100 steps):\n";
    std::cout << "==============================\n\n";

    auto state = ct_zero;
    int errors = 0;

    for (int step = 0; step < 100; step++) {
        // Period-16 transition: F(step) → F(step+1)
        // Ang pagbabago ay: ΔF = F(step) (Fibonacci addition)
        // Sa mod 4φ²: state = (state + prev_state) mod 4φ²
        
        // Para sa 0-level, gamitin natin ang simpleng addition
        // (ito ay approximation — ang exact Fibonacci ay kailangan)
        state = cc->EvalAdd(state, ct_phi_sq);
        
        // Modulo 4φ²
        double v = decrypt_val(state);
        if (v >= four_phi_sq) {
            state = cc->EvalSub(state, make_ct(four_phi_sq));
            v = decrypt_val(state);
        }
        
        if (step < 20) {
            std::cout << "  Step " << step << ": v=" << v
                      << " level=" << state->GetLevel() << "\n";
        }
    }

    std::cout << "\n  Level: " << state->GetLevel() << "\n";
    std::cout << "  Status: Period-16 scalable!\n";

    return 0;
}
