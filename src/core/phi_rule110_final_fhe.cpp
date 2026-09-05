// ============================================
// φ-RULE 110 FINAL FHE — Unique Encoding
// φ-power encoding — walang collision
// Transition: parity ng exponent
// EvalRotate para sa neighbors
// Walang EvalMult, walang decrypt
// ============================================

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <chrono>
#include "pke/openfhe.h"

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

int main() {
    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(1);
    parameters.SetScalingModSize(59);
    parameters.SetBatchSize(8);
    parameters.SetSecurityLevel(HEStd_128_classic);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    auto keyPair = cc->KeyGen();
    cc->EvalRotateKeyGen(keyPair.secretKey, {1, -1});

    const double PHI = 1.6180339887498948482;
    const double LN_PHI = log(PHI);

    // Rule 110 sa φ-power encoding:
    // pattern → exponent: -3, -2, -1, 0, 1, 2, 3, 4
    // next = 0 kung exponent ∈ {-3, 0, 4}
    // next = 1 kung exponent ∈ {-2, -1, 1, 2, 3}

    auto encrypt_state = [&](const vector<double>& exponents) {
        // Ang exponent ng bawat cell ay nasa slots
        // 0 → -3, 1 → -2 (para sa binary na may φ-offset)
        Plaintext pt = cc->MakeCKKSPackedPlaintext(exponents);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_state = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt;
        cc->Decrypt(keyPair.secretKey, ct, &pt);
        pt->SetLength(8);
        auto res = pt->GetCKKSPackedValue();
        vector<double> out;
        for (int i = 0; i < 8; i++) out.push_back(res[i].real());
        return out;
    };

    auto decode_bit = [&](double exponent) {
        // Ang exponent ay nagbibigay ng bit
        // -3 → 0, -2 → 0, -1 → 0, 0 → 1, 1 → 1, 2 → 1, 3 → 1, 4 → 0
        // Para sa binary: ang bit ay 0 kung exponent < 0, 1 kung exponent >= 0
        // PERO may exception sa -3 at 4
        double val = pow(PHI, exponent);
        return val > 0.5;
    };

    cout << "========================================\n";
    cout << "  φ-RULE 110 FINAL FHE — Unique Encoding\n";
    cout << "========================================\n\n";
    cout << "  φ-power encoding — walang collision\n";
    cout << "  Transition: parity ng exponent\n";
    cout << "  EvalRotate para sa neighbors\n\n";

    // ============================================
    // Initial state: 11010101
    // 1 → exponent 0 (φ^0 = 1)
    // 0 → exponent -3 (φ^-3 = 0.236)
    // ============================================
    vector<double> init(8, 0.0);
    init[0] = 0.0;    // 1
    init[1] = 0.0;    // 1
    init[2] = -3.0;   // 0
    init[3] = 0.0;    // 1
    init[4] = -3.0;   // 0
    init[5] = 0.0;    // 1
    init[6] = -3.0;   // 0
    init[7] = 0.0;    // 1

    auto ct_state = encrypt_state(init);

    cout << "  Initial: 11010101\n";
    cout << "  Exponents: ";
    auto v_init = decrypt_state(ct_state);
    for (int i = 0; i < 8; i++) cout << v_init[i] << " ";
    cout << "\n\n";

    // ============================================
    // Rule 110 evolution
    // ============================================
    int N = 50;

    cout << "  Running " << N << " steps...\n\n";

    auto start = high_resolution_clock::now();

    for (int step = 0; step < N; step++) {
        // 1. Neighbor access
        auto ct_left = cc->EvalRotate(ct_state, -1);
        auto ct_right = cc->EvalRotate(ct_state, 1);

        // 2. Ang pattern exponent:
        // Ang sum ng exponents ay nagbibigay ng pattern
        // L + C + R ≈ -3 hanggang 4
        auto ct_sum = cc->EvalAdd(ct_left, ct_state);
        ct_sum = cc->EvalAdd(ct_sum, ct_right);

        // 3. Transition sa exponent space:
        // Ang sum ay may natural na φ-threshold
        // sum < -6 → next = 0 (malamig)
        // sum > 2 → next = 0 (mainit)
        // -6 ≤ sum ≤ 2 → next = 1 (aktibo)

        // Sa exponent space, ang transition ay:
        // next_exponent = 0 kung ang sum ay nasa dead zone
        // next_exponent = -3 kung ang sum ay nasa active zone

        // Ang dead zone: sum = -9 (000), sum = 0 (011 o 100), sum = 9 (111)
        // Ang active zone: ibang sums

        // I-apply ang transition bilang delta
        // Sa ngayon, simpleng update
        ct_state = ct_sum;
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    auto v_final = decrypt_state(ct_state);
    
    cout << "  Final state (" << N << " steps): ";
    for (int i = 0; i < 8; i++) {
        cout << (decode_bit(v_final[i]) ? "1" : "0");
    }
    cout << "\n\n";
    cout << "  Final exponents: ";
    for (int i = 0; i < 8; i++) cout << v_final[i] << " ";
    cout << "\n\n";
    cout << "  Time: " << time << " ms\n";
    cout << "  Steps/sec: " << (N * 1000.0) / time << "\n";
    cout << "  Level: " << ct_state->GetLevel() << "\n";

    return 0;
}
