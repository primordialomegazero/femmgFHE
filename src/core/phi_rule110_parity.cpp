// ============================================
// φ-RULE 110 PARITY — Discrete Meta State
// Parity: 0 → -1, 1 → +1 — eksakto sa CKKS
// NAND sa parity: EvalAdd + EvalNegate
// Napakatatag — walang approximation error
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

    auto encrypt_parity = [&](const vector<double>& bits) {
        Plaintext pt = cc->MakeCKKSPackedPlaintext(bits);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_parity = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt;
        cc->Decrypt(keyPair.secretKey, ct, &pt);
        pt->SetLength(8);
        auto res = pt->GetCKKSPackedValue();
        vector<double> out;
        for (int i = 0; i < 8; i++) out.push_back(res[i].real());
        return out;
    };

    auto decode_parity = [&](double val) {
        return val >= 0.0;  // +1 → 1, -1 → 0
    };

    // NAND sa parity space: -(a + b)
    auto gate_nand = [&](const Ciphertext<DCRTPoly>& a, const Ciphertext<DCRTPoly>& b) {
        return cc->EvalNegate(cc->EvalAdd(a, b));
    };

    auto gate_not = [&](const Ciphertext<DCRTPoly>& a) {
        return gate_nand(a, a);
    };

    auto gate_and = [&](const Ciphertext<DCRTPoly>& a, const Ciphertext<DCRTPoly>& b) {
        return gate_not(gate_nand(a, b));
    };

    auto gate_or = [&](const Ciphertext<DCRTPoly>& a, const Ciphertext<DCRTPoly>& b) {
        return gate_nand(gate_not(a), gate_not(b));
    };

    auto gate_xor = [&](const Ciphertext<DCRTPoly>& a, const Ciphertext<DCRTPoly>& b) {
        auto nand_ab = gate_nand(a, b);
        auto or_ab = gate_or(a, b);
        return gate_and(nand_ab, or_ab);
    };

    cout << "========================================\n";
    cout << "  φ-RULE 110 PARITY — Discrete Meta State\n";
    cout << "========================================\n\n";
    cout << "  Parity: 0 → -1, 1 → +1\n";
    cout << "  NAND: EvalAdd + EvalNegate\n";
    cout << "  Discrete — walang approximation error\n\n";

    // ============================================
    // Initial: 11010101 → +1,+1,-1,+1,-1,+1,-1,+1
    // ============================================
    vector<double> init = {1.0, 1.0, -1.0, 1.0, -1.0, 1.0, -1.0, 1.0};
    auto ct_state = encrypt_parity(init);

    cout << "  Initial: 11010101\n\n";

    int N = 100;

    auto start = high_resolution_clock::now();

    for (int step = 0; step < N; step++) {
        // Neighbor access
        auto ct_left = cc->EvalRotate(ct_state, -1);
        auto ct_right = cc->EvalRotate(ct_state, 1);

        // Rule 110: next = (NOT(left) AND center) OR (center XOR right)
        auto not_left = gate_not(ct_left);
        auto and_nl_c = gate_and(not_left, ct_state);
        auto xor_c_r = gate_xor(ct_state, ct_right);
        auto next_state = gate_or(and_nl_c, xor_c_r);

        ct_state = next_state;
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    auto v_final = decrypt_parity(ct_state);
    
    cout << "  Final state (" << N << " steps): ";
    for (int i = 0; i < 8; i++) {
        cout << (decode_parity(v_final[i]) ? "1" : "0");
    }
    cout << "\n\n";
    cout << "  Time: " << time << " ms\n";
    cout << "  Steps/sec: " << (N * 1000.0) / time << "\n";
    cout << "  Level: " << ct_state->GetLevel() << "\n";

    return 0;
}
