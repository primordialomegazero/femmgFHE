// QUANTUM GATES SA φ-DOMAIN — 0-LEVEL
// Hadamard, CNOT, Toffoli sa period-4 cycle

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
    std::cout << "  QUANTUM GATES SA φ-DOMAIN\n";
    std::cout << "  0-Level Bounded\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_sq = phi * phi;
    const double two_phi_sq = 2 * phi_sq;
    const double three_phi_sq = 3 * phi_sq;

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
    auto ct_two_phi_sq = make_ct(two_phi_sq);
    auto ct_three_phi_sq = make_ct(three_phi_sq);

    // Period-4 cycle step
    auto cycle_step = [&](auto current, int step) {
        if (step % 4 == 2) {
            return cc->EvalSub(current, ct_three_phi_sq);
        } else {
            return cc->EvalAdd(current, ct_phi_sq);
        }
    };

    // ============================================
    // QUANTUM GATE DEFINITIONS
    // ============================================
    // Sa φ-domain, quantum states ay:
    // |0⟩ = 0
    // |1⟩ = φ²
    // Superposition = 2φ² (period-4)
    // Negative amplitude = -φ² (period-4)

    auto hadamard = [&](auto state) {
        // H: |0⟩ → (|0⟩+|1⟩)/√2 ≈ superposition
        // Sa φ-domain: H(0) = φ² (true)
        // H(φ²) = 0 (false)
        return cc->EvalSub(ct_phi_sq, state);
    };

    auto cnot = [&](auto control, auto target) {
        // CNOT: kung control ay |1⟩, flip ang target
        // Sa φ-domain: kung control = φ², target = φ² - target
        // Approximation: target + control - φ²
        auto sum = cc->EvalAdd(control, target);
        return cc->EvalSub(sum, ct_phi_sq);
    };

    auto toffoli = [&](auto a, auto b, auto c) {
        // Toffoli: kung a at b ay |1⟩, flip ang c
        // Sa φ-domain: c + (a + b - 2φ²)
        auto sum_ab = cc->EvalAdd(a, b);
        auto threshold = cc->EvalSub(sum_ab, ct_two_phi_sq);
        return cc->EvalAdd(c, threshold);
    };

    std::cout << "QUANTUM GATE TRUTH TABLES:\n";
    std::cout << "==========================\n\n";

    // Hadamard
    std::cout << "HADAMARD:\n";
    auto h0 = hadamard(ct_zero);
    auto h1 = hadamard(ct_phi_sq);
    std::cout << "  H(0) = " << decrypt_val(h0) << "\n";
    std::cout << "  H(φ²) = " << decrypt_val(h1) << "\n\n";

    // CNOT
    std::cout << "CNOT:\n";
    auto cn00 = cnot(ct_zero, ct_zero);
    auto cn01 = cnot(ct_zero, ct_phi_sq);
    auto cn10 = cnot(ct_phi_sq, ct_zero);
    auto cn11 = cnot(ct_phi_sq, ct_phi_sq);
    std::cout << "  CNOT(0,0) = " << decrypt_val(cn00) << "\n";
    std::cout << "  CNOT(0,φ²) = " << decrypt_val(cn01) << "\n";
    std::cout << "  CNOT(φ²,0) = " << decrypt_val(cn10) << "\n";
    std::cout << "  CNOT(φ²,φ²) = " << decrypt_val(cn11) << "\n\n";

    // Toffoli
    std::cout << "TOFFOLI:\n";
    auto tof000 = toffoli(ct_zero, ct_zero, ct_zero);
    auto tof110 = toffoli(ct_phi_sq, ct_phi_sq, ct_zero);
    std::cout << "  TOFFOLI(0,0,0) = " << decrypt_val(tof000) << "\n";
    std::cout << "  TOFFOLI(φ²,φ²,0) = " << decrypt_val(tof110) << "\n\n";

    // ============================================
    // QUANTUM CIRCUIT SIMULATION
    // ============================================
    std::cout << "QUANTUM CIRCUIT (H + CNOT + Toffoli):\n";
    std::cout << "=====================================\n\n";

    // Circuit: H(q1) → CNOT(q1,q2) → Toffoli(q1,q2,q3)
    auto q1 = ct_zero;
    auto q2 = ct_zero;
    auto q3 = ct_zero;

    // Apply H sa q1
    q1 = hadamard(q1);
    std::cout << "  After H(q1): " << decrypt_val(q1) << "\n";

    // Apply CNOT(q1, q2)
    q2 = cnot(q1, q2);
    std::cout << "  After CNOT(q1,q2): " << decrypt_val(q2) << "\n";

    // Apply Toffoli(q1, q2, q3)
    q3 = toffoli(q1, q2, q3);
    std::cout << "  After Toffoli(q1,q2,q3): " << decrypt_val(q3) << "\n\n";

    std::cout << "  Level: " << q3->GetLevel() << "\n";
    std::cout << "  Status: " << (q3->GetLevel() == 0 ? "✅ 0-LEVEL QUANTUM!" : "❌") << "\n";

    return 0;
}
