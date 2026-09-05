// ============================================
// φ-RULE 110 HYBRID — EvalRotate + EvalAdd
// Neighbors: EvalRotate (2x per step)
// Transition: NAND gates (EvalAdd + EvalNegate)
// Tunay na FHE — encrypted state
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

    auto encrypt_bits = [&](const vector<double>& bits) {
        Plaintext pt = cc->MakeCKKSPackedPlaintext(bits);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_bits = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt;
        cc->Decrypt(keyPair.secretKey, ct, &pt);
        pt->SetLength(8);
        auto res = pt->GetCKKSPackedValue();
        vector<double> out;
        for (int i = 0; i < 8; i++) out.push_back(res[i].real());
        return out;
    };

    auto decode_bit = [&](double val) {
        return val >= -0.01;
    };

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
    cout << "  φ-RULE 110 HYBRID — EvalRotate + EvalAdd\n";
    cout << "========================================\n\n";
    cout << "  Neighbors: EvalRotate (2x per step)\n";
    cout << "  Transition: NAND gates\n";
    cout << "  Tunay na FHE — encrypted state\n\n";

    // ============================================
    // Rule 110 evolution
    // ============================================
    cout << "  Rule 110: next = (NOT(left) AND center) OR (center XOR right)\n\n";

    // Initial: 11010101
    vector<double> init = {2.0, 2.0, -2.0, 2.0, -2.0, 2.0, -2.0, 2.0};
    auto ct_state = encrypt_bits(init);

    cout << "  Initial: 11010101\n\n";

    int N = 1000;
    
    auto start = high_resolution_clock::now();

    for (int step = 0; step < N; step++) {
        // 1. Neighbor access gamit ang EvalRotate
        auto ct_left = cc->EvalRotate(ct_state, -1);   // left: shift pataas
        auto ct_right = cc->EvalRotate(ct_state, 1);    // right: shift pababa
        
        // 2. Rule 110 transition
        // next = (NOT(left) AND center) OR (center XOR right)
        auto not_left = gate_not(ct_left);
        auto and_nl_c = gate_and(not_left, ct_state);
        auto xor_c_r = gate_xor(ct_state, ct_right);
        auto next_state = gate_or(and_nl_c, xor_c_r);
        
        // 3. I-update ang state
        ct_state = next_state;
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();

    auto v_final = decrypt_bits(ct_state);
    
    cout << "  Final state (1000 steps): ";
    for (int i = 0; i < 8; i++) {
        cout << (decode_bit(v_final[i]) ? "1" : "0");
    }
    cout << "\n\n";
    cout << "  Time: " << time << " ms\n";
    cout << "  Steps/sec: " << (N * 1000.0) / time << "\n";
    cout << "  Level: " << ct_state->GetLevel() << "\n";

    return 0;
}
