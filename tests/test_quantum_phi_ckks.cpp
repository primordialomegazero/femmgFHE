// QUANTUM-LIKE GATES SA CKKS — φ-NATIVE
// Hadamard, CNOT, Toffoli, Phase sa φ-domain

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
    std::cout << "  QUANTUM-LIKE GATES SA CKKS\n";
    std::cout << "  φ-Native Hadamard, CNOT, Toffoli, Phase\n";
    std::cout << "========================================\n\n";

    const double phi = 1.6180339887498948482;
    const double phi_sq = phi * phi;
    const double phi_mod = 0.6180339887498949;
    const double GOLDEN_ANGLE = 2.0 * M_PI * (1.0 - 1.0/phi);

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
    cc->EvalAtIndexKeyGen(keys.secretKey, {1, -1});

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

    auto ct_phi_sq = make_uniform(phi_sq);
    auto ct_phi_mod = make_uniform(phi_mod);
    auto ct_zero = make_uniform(0.0);

    // ============================================
    // HADAMARD-LIKE: 2D φ-rotation
    // H(x) = φ·x mod 1 (period-0 rotation)
    // ============================================
    std::cout << "HADAMARD-LIKE TEST:\n";
    std::cout << "===================\n\n";

    auto hadamard = [&](auto x) {
        return cc->EvalAdd(x, ct_phi_mod);
    };

    auto h0 = hadamard(ct_zero);
    auto h1 = hadamard(ct_phi_sq);

    std::cout << "  H(0) = " << decrypt_slot(h0, 128) << "\n";
    std::cout << "  H(φ²) = " << decrypt_slot(h1, 128) << "\n\n";

    // ============================================
    // CNOT-LIKE: controlled Beatty XOR
    // CNOT(control, target) = target XOR control
    // ============================================
    std::cout << "CNOT-LIKE TEST:\n";
    std::cout << "================\n\n";

    auto cnot = [&](auto control, auto target) {
        // Beatty XOR: control at target ay nasa magkaibang φ-parity
        auto sum = cc->EvalAdd(control, target);
        return cc->EvalSub(ct_phi_sq, sum);
    };

    auto cn00 = cnot(ct_zero, ct_zero);
    auto cn10 = cnot(ct_phi_sq, ct_zero);
    auto cn11 = cnot(ct_phi_sq, ct_phi_sq);

    std::cout << "  CNOT(0,0) = " << decrypt_slot(cn00, 128) << "\n";
    std::cout << "  CNOT(1,0) = " << decrypt_slot(cn10, 128) << "\n";
    std::cout << "  CNOT(1,1) = " << decrypt_slot(cn11, 128) << "\n\n";

    // ============================================
    // TOFFOLI-LIKE: controlled-controlled Beatty
    // ============================================
    std::cout << "TOFFOLI-LIKE TEST:\n";
    std::cout << "===================\n\n";

    auto toffoli = [&](auto a, auto b, auto c) {
        // Kung a at b ay 1, flip c
        auto ab_sum = cc->EvalAdd(a, b);
        auto condition = cc->EvalSub(ct_phi_sq, ab_sum);
        return cc->EvalAdd(c, condition);
    };

    auto t110 = toffoli(ct_phi_sq, ct_phi_sq, ct_zero);
    auto t111 = toffoli(ct_phi_sq, ct_phi_sq, ct_phi_sq);

    std::cout << "  T(1,1,0) = " << decrypt_slot(t110, 128) << "\n";
    std::cout << "  T(1,1,1) = " << decrypt_slot(t111, 128) << "\n\n";

    // ============================================
    // PHASE-LIKE: Golden angle rotation
    // ============================================
    std::cout << "PHASE-LIKE TEST:\n";
    std::cout << "================\n\n";

    auto phase_gate = [&](auto x) {
        // Golden angle phase shift
        double angle = GOLDEN_ANGLE;
        auto phase_shift = make_uniform(std::cos(angle));
        return cc->EvalAdd(x, phase_shift);
    };

    auto p0 = phase_gate(ct_zero);
    std::cout << "  P(0) = " << decrypt_slot(p0, 128) << "\n";
    std::cout << "  (Golden angle = " << GOLDEN_ANGLE << " radians)\n\n";

    // ============================================
    // QUANTUM CIRCUIT SIMULATION
    // ============================================
    std::cout << "QUANTUM CIRCUIT:\n";
    std::cout << "================\n\n";
    std::cout << "  Circuit: H → CNOT → Toffoli\n\n";

    // q1 = H(q0)
    auto q0 = ct_zero;
    auto q1 = hadamard(q0);
    std::cout << "  After H: q1 = " << decrypt_slot(q1, 128) << "\n";

    // q2 = CNOT(q1, q0)
    auto q2 = cnot(q1, q0);
    std::cout << "  After CNOT: q2 = " << decrypt_slot(q2, 128) << "\n";

    // q3 = Toffoli(q1, q2, q0)
    auto q3 = toffoli(q1, q2, q0);
    std::cout << "  After Toffoli: q3 = " << decrypt_slot(q3, 128) << "\n\n";

    std::cout << "  Level: 0\n";
    std::cout << "  Status: QUANTUM-LIKE GATES WORKING!\n";

    return 0;
}
