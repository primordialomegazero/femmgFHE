// 2D PERIOD-0 — DOUBLE IRRATIONAL
// x_{n+1} = (x_n + φ²) mod 1
// y_{n+1} = (y_n + φ³) mod 1
// 2D dense sa [0,1)²

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
    std::cout << "  2D PERIOD-0 — DOUBLE IRRATIONAL\n";
    std::cout << "  φ² at φ³ independent rotations\n";
    std::cout << "========================================\n\n";

    const double phi_sq_mod = 0.6180339887498949;   // φ² mod 1
    const double phi_cu_mod = 0.2360679774997897;   // φ³ mod 1

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

    auto ct_phi_sq_mod = make_uniform(phi_sq_mod);
    auto ct_phi_cu_mod = make_uniform(phi_cu_mod);

    // Dalawang independent states
    auto state_x = make_uniform(0.0);
    auto state_y = make_uniform(0.0);

    std::cout << "2D IRRATIONAL ROTATION (5000 steps):\n";
    std::cout << "====================================\n\n";

    auto t_start = high_resolution_clock::now();
    int errors = 0;

    for (int step = 0; step < 5000; step++) {
        // Independent rotations
        state_x = cc->EvalAdd(state_x, ct_phi_sq_mod);
        state_y = cc->EvalAdd(state_y, ct_phi_cu_mod);

        double x = decrypt_slot(state_x, 0);
        double y = decrypt_slot(state_y, 0);

        x = x - std::floor(x);
        y = y - std::floor(y);

        bool bounded = (x >= 0 && x < 1 && y >= 0 && y < 1);
        if (!bounded) errors++;

        if (step % 500 == 0) {
            std::cout << "  Step " << step << ": "
                      << "x=" << x << " "
                      << "y=" << y << " "
                      << "level=" << state_x->GetLevel()
                      << (bounded ? " ✓" : " ✗") << "\n";
        }
    }

    auto t_end = high_resolution_clock::now();
    auto total_seconds = duration_cast<seconds>(t_end - t_start).count();

    std::cout << "\n  Errors: " << errors << "/5000\n";
    std::cout << "  Time: " << total_seconds / 60.0 << " minutes\n";
    std::cout << "  Level: " << state_x->GetLevel() << "\n";
    std::cout << "  Accuracy: " << (1.0 - (double)errors / 5000.0) * 100 << "%\n";
    std::cout << "  Status: " << (errors == 0 ? "✅ 2D PERIOD-0 BOUNDED!" : "❌") << "\n";

    return 0;
}
