// QUANTUM RULE 110 — NORMALIZED ROTATION
// I-bound ang values pagkatapos ng bawat rotation
// Para walang tumaliwas

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
    std::cout << "  QUANTUM RULE 110 — NORMALIZED\n";
    std::cout << "  Bounded Rotation\n";
    std::cout << "========================================\n\n";

    const double phi_mod = 0.6180339887498949;
    const double psi_mod = -0.6180339887498949;
    const double phi_sq = 2.618033988749895;

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

    auto ct_phi_mod = make_uniform(phi_mod);
    auto ct_psi_mod = make_uniform(psi_mod);
    auto ct_phi_sq = make_uniform(phi_sq);

    // Initial
    std::vector<std::complex<double>> init(slots, {0.0, 0.0});
    for (int i = 0; i < slots; i++) {
        init[i] = {(i % 3 == 0) ? phi_mod : (i % 3 == 1) ? psi_mod : 0.0, 0.0};
    }

    auto qstate = cc->Encrypt(keys.publicKey, cc->MakeCKKSPackedPlaintext(init));

    std::cout << "QUANTUM RULE 110 NORMALIZED (1000 steps):\n";
    std::cout << "========================================\n\n";

    auto t_start = high_resolution_clock::now();
    int errors = 0;

    for (int step = 0; step < 1000; step++) {
        qstate = cc->EvalAtIndex(qstate, 1);

        // Alternating φ at ψ
        if (step % 2 == 0) {
            qstate = cc->EvalAdd(qstate, ct_phi_mod);
        } else {
            qstate = cc->EvalAdd(qstate, ct_psi_mod);
        }

        // Normalization: kung > φ_sq, subtract φ_sq
        // Ito ay natural na modulo sa φ domain
        double v = decrypt_slot(qstate, 128);
        if (v > phi_sq) {
            qstate = cc->EvalSub(qstate, ct_phi_sq);
            v = decrypt_slot(qstate, 128);
        }
        if (v < -phi_sq) {
            qstate = cc->EvalAdd(qstate, ct_phi_sq);
            v = decrypt_slot(qstate, 128);
        }

        bool bounded = (v >= -phi_sq && v <= phi_sq);
        if (!bounded) errors++;

        if (step % 100 == 0) {
            std::cout << "  Step " << step << ": "
                      << "q[128]=" << v
                      << " level=" << qstate->GetLevel()
                      << (bounded ? " ✓" : " ✗") << "\n";
        }
    }

    auto t_end = high_resolution_clock::now();
    auto total_seconds = duration_cast<seconds>(t_end - t_start).count();

    std::cout << "\n  Errors: " << errors << "/1000\n";
    std::cout << "  Time: " << total_seconds << "s\n";
    std::cout << "  Level: " << qstate->GetLevel() << "\n";
    std::cout << "  Status: " << (errors == 0 ? "✅ NORMALIZED BOUNDED!" : "❌") << "\n";

    return 0;
}
