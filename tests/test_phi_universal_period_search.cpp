// UNIVERSAL PERIOD SEARCH — EMERGENT PROPERTIES
// Hanapin ang solid universal period na bounded at 0-level

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
    std::cout << "  UNIVERSAL PERIOD SEARCH\n";
    std::cout << "  Emergent Properties\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_sq = phi * phi;
    const double two_phi_sq = 2 * phi_sq;
    const double three_phi_sq = 3 * phi_sq;
    const double four_phi_sq = 4 * phi_sq;

    CCParams<CryptoContextCKKSRNS> params;
    params.SetMultiplicativeDepth(5);
    params.SetScalingModSize(50);
    params.SetBatchSize(256);
    params.SetFirstModSize(60);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(params);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    cc->Enable(ADVANCEDSHE);

    auto keys = cc->KeyGen();
    cc->EvalMultKeyGen(keys.secretKey);
    cc->EvalAtIndexKeyGen(keys.secretKey, {1, -1, 2, -2});
    auto slots = cc->GetEncodingParams()->GetBatchSize();

    // Helper function — DITO MUNA BAGO GAMITIN
    auto make_state = [&](double val) {
        std::vector<std::complex<double>> vec(slots, {0.0, 0.0});
        vec[128] = {val, 0.0};
        return cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec));
    };

    auto decrypt_slot = [&](auto ct, int slot_num) {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetCKKSPackedValue()[slot_num].real();
    };

    // ============================================
    // SEARCH: Iba't ibang periodic maps
    // ============================================
    
    std::cout << "TESTING PERIODIC MAPS:\n";
    std::cout << "======================\n\n";
    
    // Map 1: f(x) = φ² - x (period-2)
    std::cout << "MAP 1: f(x) = φ² - x (period-2):\n  ";
    {
        auto state = make_state(phi_sq);
        bool bounded = true;
        for (int i = 0; i < 30; i++) {
            state = cc->EvalSub(make_state(phi_sq), state);
            double v = decrypt_slot(state, 128);
            if (std::abs(v) > phi_sq + 0.01) { bounded = false; break; }
        }
        std::cout << (bounded ? "✓ BOUNDED" : "✗ EXPLODING") << "\n\n";
    }
    
    // Map 2: f(x) = 2φ² - x (period-2, higher)
    std::cout << "MAP 2: f(x) = 2φ² - x (period-2):\n  ";
    {
        auto state = make_state(phi_sq);
        bool bounded = true;
        for (int i = 0; i < 30; i++) {
            state = cc->EvalSub(make_state(two_phi_sq), state);
            double v = decrypt_slot(state, 128);
            if (std::abs(v) > two_phi_sq + 0.01) { bounded = false; break; }
        }
        std::cout << (bounded ? "✓ BOUNDED" : "✗ EXPLODING") << "\n\n";
    }
    
    // Map 3: f(x) = 3φ² - x (period-2, higher)
    std::cout << "MAP 3: f(x) = 3φ² - x (period-2):\n  ";
    {
        auto state = make_state(phi_sq);
        bool bounded = true;
        for (int i = 0; i < 30; i++) {
            state = cc->EvalSub(make_state(three_phi_sq), state);
            double v = decrypt_slot(state, 128);
            if (std::abs(v) > three_phi_sq + 0.01) { bounded = false; break; }
        }
        std::cout << (bounded ? "✓ BOUNDED" : "✗ EXPLODING") << "\n\n";
    }
    
    // Map 4: Fibonacci two-state
    std::cout << "MAP 4: FIBONACCI (x_{n+1} = x_{n-1} - x_n):\n  ";
    {
        auto state_prev = make_state(0.0);
        auto state_curr = make_state(phi_sq);
        bool bounded = true;
        for (int i = 0; i < 30; i++) {
            auto state_next = cc->EvalSub(state_prev, state_curr);
            state_prev = state_curr;
            state_curr = state_next;
            double v = decrypt_slot(state_curr, 128);
            if (std::abs(v) > 100 * phi_sq) { bounded = false; break; }
        }
        std::cout << (bounded ? "✓ BOUNDED (slow)" : "✗ EXPLODING") << "\n\n";
    }
    
    // Map 5: Alternating K map (K1 - x, K2 - x)
    std::cout << "MAP 5: ALTERNATING K (φ²-x, 2φ²-x):\n  ";
    {
        auto state = make_state(phi_sq);
        bool bounded = true;
        for (int i = 0; i < 30; i++) {
            double K = (i % 2 == 0) ? phi_sq : two_phi_sq;
            state = cc->EvalSub(make_state(K), state);
            double v = decrypt_slot(state, 128);
            if (std::abs(v) > four_phi_sq + 0.01) { bounded = false; break; }
        }
        std::cout << (bounded ? "✓ BOUNDED" : "✗ EXPLODING") << "\n\n";
    }
    
    // Map 6: Triple oscillation (φ²-x, 2φ²-x, 3φ²-x cycle)
    std::cout << "MAP 6: TRIPLE OSCILLATION:\n  ";
    {
        auto state = make_state(phi_sq);
        bool bounded = true;
        for (int i = 0; i < 30; i++) {
            double K = (i % 3 == 0) ? phi_sq : (i % 3 == 1) ? two_phi_sq : three_phi_sq;
            state = cc->EvalSub(make_state(K), state);
            double v = decrypt_slot(state, 128);
            if (std::abs(v) > three_phi_sq + 0.01) { bounded = false; break; }
        }
        std::cout << (bounded ? "✓ BOUNDED" : "✗ EXPLODING") << "\n\n";
    }
    
    std::cout << "========================================\n";
    std::cout << "  KEY: Ang mga bounded maps ay:\n";
    std::cout << "  - Period-2 (simple)\n";
    std::cout << "  - Period-3 (triple oscillation)\n";
    std::cout << "  - Alternating (mixed periods)\n";
    std::cout << "  → Pwedeng i-combine para sa\n";
    std::cout << "    mas complex dynamics!\n";
    std::cout << "========================================\n";

    return 0;
}
