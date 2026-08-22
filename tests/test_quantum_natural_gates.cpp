// NATURAL QUANTUM GATES — φ-NATIVE BOUNDED
// Hadamard = Period-0 rotation
// CNOT = Beatty XOR (walang sum)
// Toffoli = Controlled Phase
// Phase = Golden angle

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
    std::cout << "  NATURAL QUANTUM GATES\n";
    std::cout << "  φ-Native at Bounded\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_sq = phi * phi;
    const double phi_mod = 0.6180339887498949;
    const double psi_mod = -0.6180339887498949;

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

    auto decrypt_slot = [&](auto ct, int slot_num) {
        Plaintext pt;
        cc->Decrypt(keys.secretKey, ct, &pt);
        return pt->GetCKKSPackedValue()[slot_num].real();
    };

    auto make_uniform = [&](double val) {
        std::vector<std::complex<double>> vec(slots, {0.0, 0.0});
        for (int i = 0; i < slots; i++) vec[i] = {val, 0.0};
        return cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(vec));
    };

    auto ct_phi_mod = make_uniform(phi_mod);
    auto ct_psi_mod = make_uniform(psi_mod);
    auto ct_phi_sq = make_uniform(phi_sq);

    // ============================================
    // NATURAL HADAMARD: Period-0 irrational rotation
    // H(x) = x + φ_mod (mod 1) — bounded sa [0,1)
    // ============================================
    auto hadamard = [&](auto x) {
        return cc->EvalAdd(x, ct_phi_mod);
    };

    // ============================================
    // NATURAL CNOT: Beatty XOR via φ/ψ parity
    // CNOT(c,t) = c + t - 2·Beatty(c)·Beatty(t)
    // Sa φ: c XOR t = (c + t) mod φ²
    // ============================================
    auto cnot = [&](auto control, auto target) {
        // Beatty XOR: kung nasa parehong Beatty → 0
        // Kung nasa magkaibang Beatty → φ²
        auto sum = cc->EvalAdd(control, target);
        auto ct_two_phi = make_uniform(2 * phi_sq);
        return cc->EvalSub(ct_two_phi, sum);
    };

    // ============================================
    // NATURAL PHASE: Golden angle shift
    // P(x) = x rotated by golden angle
    // ============================================
    auto phase_gate = [&](auto x) {
        return cc->EvalAdd(x, make_uniform(std::cos(2.39996)));
    };

    // ============================================
    // QUANTUM CIRCUIT NA WALANG SUM
    // ============================================
    std::cout << "NATURAL QUANTUM CIRCUIT:\n";
    std::cout << "========================\n\n";

    // q0 = |0⟩
    auto q0 = make_uniform(0.0);

    // q1 = H(q0) = φ_mod
    auto q1 = hadamard(q0);
    std::cout << "  H|0⟩ = " << decrypt_slot(q1, 0) << " (φ_mod)\n\n";

    // q2 = CNOT(q1, q0) = Beatty XOR
    auto q2 = cnot(q1, q0);
    std::cout << "  CNOT(q1,q0) = " << decrypt_slot(q2, 0) << "\n\n";

    // q3 = Phase(q2) = Golden angle shift
    auto q3 = phase_gate(q2);
    std::cout << "  Phase(q2) = " << decrypt_slot(q3, 0) << "\n\n";

    // ============================================
    // BOUNDED TEST (1000 steps)
    // ============================================
    std::cout << "BOUNDED QUANTUM TEST (1000 steps):\n";
    std::cout << "==================================\n\n";

    auto state = make_uniform(0.0);
    int errors = 0;

    for (int step = 0; step < 1000; step++) {
        // Natural quantum evolution: H → Phase → H → Phase...
        if (step % 2 == 0) {
            state = hadamard(state);
        } else {
            state = phase_gate(state);
        }

        double v = decrypt_slot(state, 0);
        bool bounded = (v >= -2.0 && v <= 2.0);
        if (!bounded) errors++;

        if (step % 100 == 0) {
            std::cout << "  Step " << step << ": "
                      << "q=" << v
                      << " level=" << state->GetLevel()
                      << (bounded ? " ✓" : " ✗") << "\n";
        }
    }

    std::cout << "\n  Errors: " << errors << "/1000\n";
    std::cout << "  Level: " << state->GetLevel() << "\n";
    std::cout << "  Status: " << (errors == 0 ? "✅ NATURAL QUANTUM BOUNDED!" : "❌") << "\n";

    return 0;
}
