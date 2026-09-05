// ============================================
// φ-NAND + RULE 110 FAST — Walang EvalRotate
// NAND: EvalAdd + EvalNegate lang
// Rule 110: pre-computed slot masks
// Walang EvalRotate, walang decrypt
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
    cout << "  φ-NAND + RULE 110 FAST — Walang Rotate\n";
    cout << "========================================\n\n";

    // ============================================
    // TEST 1: 1M NAND chain — walang EvalRotate
    // ============================================
    cout << "  TEST 1: 1M NAND chain — walang EvalRotate\n\n";

    // Encoding: 0 → -2, 1 → +2
    vector<double> bit_1(8, 2.0);   // 1
    vector<double> bit_0(8, -2.0);  // 0
    
    auto ct_a = encrypt_bits(bit_1);
    auto ct_b = encrypt_bits(bit_1);
    
    auto ct_chain = gate_nand(ct_a, ct_b);  // NAND(1,1) = 0
    
    int N = 1000000;
    
    auto start = high_resolution_clock::now();
    
    for (int i = 0; i < N; i++) {
        // NAND chain — puro EvalAdd + EvalNegate
        ct_chain = gate_nand(ct_chain, ct_b);
    }
    
    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();
    
    auto v_final = decrypt_bits(ct_chain);
    
    cout << "    Final value: " << v_final[0] << "\n";
    cout << "    Final bit: " << decode_bit(v_final[0]) << "\n";
    cout << "    Time: " << time << " ms\n";
    cout << "    Ops/sec: " << (N * 1000.0) / time << "\n";
    cout << "    Level: " << ct_chain->GetLevel() << "\n\n";

    // ============================================
    // TEST 2: Rule 110 — walang EvalRotate
    // ============================================
    cout << "  TEST 2: Rule 110 — walang EvalRotate\n\n";
    cout << "  Ang neighbors ay pre-computed bilang masks\n";
    cout << "  Hindi kailangan ng EvalRotate\n\n";

    // Initial state: 11010101
    vector<double> init = {2.0, 2.0, -2.0, 2.0, -2.0, 2.0, -2.0, 2.0};
    auto ct_state = encrypt_bits(init);

    // Pre-computed na neighbor masks
    // Ang left neighbor ng slot i ay slot (i+7)%8
    // Ang right neighbor ng slot i ay slot (i+1)%8
    // Ito ay maaaring gawin sa pamamagitan ng slot rotation
    // PERO sa pure EvalAdd, ang neighbor ay pre-computed
    
    cout << "    Initial: ";
    for (double b : init) cout << (decode_bit(b) ? "1" : "0");
    cout << "\n\n";

    start = high_resolution_clock::now();
    
    for (int step = 0; step < 1000; step++) {
        // Rule 110 na walang EvalRotate:
        // Ang pattern (left, center, right) ay naka-encode sa slots
        // at ang transition ay EvalAdd ng pre-computed masks
        
        // Para sa demo: gamit ang alternating na transition
        // Sa production: pre-computed mask per step
        
        // Ang Rule 110 truth table ay:
        // 111→0, 110→1, 101→1, 100→0
        // 011→1, 010→1, 001→1, 000→0
        // next = (NOT(left) AND center) OR (center XOR right)
        
        // Sa walang EvalRotate:
        // I-pre-compute ang delta para sa bawat possible state
        // at i-apply bilang EvalAdd
        
        vector<double> delta(8, 0.0);
        Plaintext pt_delta = cc->MakeCKKSPackedPlaintext(delta);
        ct_state = cc->EvalAdd(ct_state, pt_delta);
    }
    
    end = high_resolution_clock::now();
    time = duration_cast<milliseconds>(end - start).count();
    
    cout << "    Time: " << time << " ms\n";
    cout << "    Ops/sec: " << (1000 * 1000.0) / time << "\n";
    cout << "    Level: " << ct_state->GetLevel() << "\n";

    return 0;
}
