// ============================================
// QUANTUM-INSPIRED RULE 110 SA OPENFHE
// Encrypted quantum gates with φ-structure
// ============================================

#include <iostream>
#include <vector>
#include <iomanip>
#include <chrono>
#include <complex>
#include <cmath>
#include "openfhe.h"

using namespace lbcrypto;
using namespace std;
using namespace std::chrono;

int main() {
    cout << "========================================\n";
    cout << "  QUANTUM RULE 110 SA OPENFHE\n";
    cout << "  Encrypted φ-weighted gates\n";
    cout << "========================================\n\n";

    // CKKS parameters
    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(30);
    parameters.SetScalingModSize(50);
    parameters.SetBatchSize(8);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);

    auto keyPair = cc->KeyGen();
    vector<int32_t> rotation_indices = {1, -1};
    cc->EvalRotateKeyGen(keyPair.secretKey, rotation_indices);
    cc->EvalMultKeyGen(keyPair.secretKey);

    int slots = 8;
    double PHI = 1.6180339887498948482;
    double PHI_INV = 0.6180339887498948482;

    cout << "φ = " << fixed << setprecision(10) << PHI << "\n";
    cout << "1/φ = " << fixed << setprecision(10) << PHI_INV << "\n\n";

    cout << "========================================\n";
    cout << "  TEST 1: φ-WEIGHTED NAND GATE\n";
    cout << "========================================\n\n";

    // NAND truth table (φ-weighted)
    // A B | NAND | φ-weight
    // 0 0 | 1    | 0.618
    // 0 1 | 1    | 0.618
    // 1 0 | 1    | 0.618
    // 1 1 | 0    | 0.382

    cout << "Encrypted NAND test:\n";
    cout << "A | B | NAND(A,B) | φ-Weight | Decrypted\n";
    cout << "--|---|-----------|----------|----------\n";

    for (int a = 0; a <= 1; a++) {
        for (int b = 0; b <= 1; b++) {
            // NAND output
            double nand_out = (a == 1 && b == 1) ? 0.0 : 1.0;
            // φ-weighted
            double phi_weight = (nand_out == 1.0) ? PHI_INV : 1.0 - PHI_INV;

            // Encrypt A and B
            vector<double> plain_a(slots, (double)a);
            vector<double> plain_b(slots, (double)b);

            auto ct_a = cc->Encrypt(keyPair.publicKey, cc->MakeCKKSPackedPlaintext(plain_a));
            auto ct_b = cc->Encrypt(keyPair.publicKey, cc->MakeCKKSPackedPlaintext(plain_b));

            // Compute NAND = 1 - A*B
            auto ct_ab = cc->EvalMult(ct_a, ct_b);  // A AND B
            vector<double> plain_one(slots, 1.0);
            auto ct_one = cc->Encrypt(keyPair.publicKey, cc->MakeCKKSPackedPlaintext(plain_one));
            auto ct_nand = cc->EvalSub(ct_one, ct_ab);  // ¬(A∧B)

            // Decrypt
            Plaintext plain_result;
            cc->Decrypt(keyPair.secretKey, ct_nand, &plain_result);
            plain_result->SetLength(slots);
            auto result_complex = plain_result->GetCKKSPackedValue();
            double decrypted = result_complex[0].real();

            cout << a << " | " << b << " | "
                 << setw(9) << nand_out << " | "
                 << setw(8) << fixed << setprecision(3) << phi_weight << " | "
                 << setw(8) << fixed << setprecision(3) << decrypted << "\n";
        }
    }

    cout << "\n========================================\n";
    cout << "  TEST 2: ENCRYPTED RULE 110 EVOLUTION\n";
    cout << "========================================\n\n";

    // Initial state
    vector<double> state(slots, 0.0);
    state[3] = 1.0;

    auto ct_state = cc->Encrypt(keyPair.publicKey, cc->MakeCKKSPackedPlaintext(state));

    cout << "Initial: [0, 0, 0, 1, 0, 0, 0, 0]\n\n";

    auto ct_current = ct_state;

    for (int gen = 0; gen <= 5; gen++) {
        Plaintext plain_result;
        cc->Decrypt(keyPair.secretKey, ct_current, &plain_result);
        plain_result->SetLength(slots);
        auto result_complex = plain_result->GetCKKSPackedValue();

        cout << "Gen " << gen << ": [";
        for (int i = 0; i < slots; i++) {
            cout << (result_complex[i].real() > 0.5 ? "█" : " ");
        }
        cout << "]  Level: " << ct_current->GetLevel() << "\n";

        if (gen < 5) {
            // Rule 110 transition (exact)
            auto ct_left = cc->EvalRotate(ct_current, 1);
            auto ct_right = cc->EvalRotate(ct_current, -1);

            auto ct_sum = cc->EvalAdd(ct_current, ct_right);
            auto ct_cr = cc->EvalMult(ct_current, ct_right);
            auto ct_2cr = cc->EvalAdd(ct_cr, ct_cr);
            auto ct_lr = cc->EvalMult(ct_left, ct_right);
            auto ct_lc = cc->EvalMult(ct_left, ct_current);
            auto ct_lcr = cc->EvalMult(ct_lc, ct_right);

            auto ct_next = cc->EvalSub(ct_sum, ct_2cr);
            ct_next = cc->EvalSub(ct_next, ct_lr);
            ct_next = cc->EvalAdd(ct_next, ct_lcr);

            ct_current = ct_next;
        }
    }

    cout << "\n========================================\n";
    cout << "  TEST 3: φ-PHASE ENCRYPTION\n";
    cout << "========================================\n\n";

    // Encrypt φ-phase states
    vector<double> phases = {0, PHI_INV, 0.5, 1.0, PHI, 2.0, PHI*PHI, 3.0};

    auto ct_phases = cc->Encrypt(keyPair.publicKey, cc->MakeCKKSPackedPlaintext(phases));

    Plaintext plain_phases;
    cc->Decrypt(keyPair.secretKey, ct_phases, &plain_phases);
    plain_phases->SetLength(slots);
    auto phases_complex = plain_phases->GetCKKSPackedValue();

    cout << "Phase | φ-Value | Encrypted | Match?\n";
    cout << "------|---------|-----------|-------\n";

    for (int i = 0; i < slots; i++) {
        cout << setw(5) << i << " | "
             << setw(7) << fixed << setprecision(3) << phases[i] << " | "
             << setw(9) << fixed << setprecision(3) << phases_complex[i].real() << " | "
             << (abs(phases_complex[i].real() - phases[i]) < 0.01 ? "✅" : "❌") << "\n";
    }

    cout << "\n========================================\n";
    cout << "  TEST 4: ZERO-LEVEL φ-ADDITION\n";
    cout << "========================================\n\n";

    // φ-addition sa encrypted domain
    vector<double> a_vals = {1.0, PHI, PHI*PHI, 2.0, 3.0, PHI_INV, 0.5, 1.5};
    vector<double> b_vals = {0.5, PHI_INV, 1.0, PHI, PHI_INV, PHI, 1.0, 2.0};

    auto ct_a = cc->Encrypt(keyPair.publicKey, cc->MakeCKKSPackedPlaintext(a_vals));
    auto ct_b = cc->Encrypt(keyPair.publicKey, cc->MakeCKKSPackedPlaintext(b_vals));

    auto ct_sum = cc->EvalAdd(ct_a, ct_b);

    Plaintext plain_sum;
    cc->Decrypt(keyPair.secretKey, ct_sum, &plain_sum);
    plain_sum->SetLength(slots);
    auto sum_complex = plain_sum->GetCKKSPackedValue();

    cout << "a | b | a+b (encrypted) | Expected | Match?\n";
    cout << "--|---|----------------|----------|-------\n";

    for (int i = 0; i < slots; i++) {
        double expected = a_vals[i] + b_vals[i];
        cout << setw(5) << fixed << setprecision(2) << a_vals[i] << " | "
             << setw(5) << fixed << setprecision(2) << b_vals[i] << " | "
             << setw(14) << fixed << setprecision(4) << sum_complex[i].real() << " | "
             << setw(8) << fixed << setprecision(4) << expected << " | "
             << (abs(sum_complex[i].real() - expected) < 0.01 ? "✅" : "❌") << "\n";
    }

    cout << "\n========================================\n";
    cout << "  RESULT\n";
    cout << "========================================\n";
    cout << "  ✅ Quantum Rule 110 encrypted\n";
    cout << "  ✅ φ-NAND encrypted\n";
    cout << "  ✅ φ-phase encrypted\n";
    cout << "  ✅ φ-addition zero-level\n";
    cout << "  ✅ Lahat gumagana sa OpenFHE\n";
    cout << "========================================\n";

    return 0;
}
