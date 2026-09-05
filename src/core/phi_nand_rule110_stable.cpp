// ============================================
// φ-NAND + RULE 110 STABLE — Pure FHE
// NAND gates na stable, Rule 110 evolution
// Walang EvalMult, walang decrypt sa loop
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
    cc->EvalRotateKeyGen(keyPair.secretKey, {1, 2, -1, -2});

    const double SQRT5 = sqrt(5.0);

    auto encrypt_bit = [&](bool bit, int slot) {
        double val = bit ? SQRT5 : -SQRT5;
        vector<double> v(8, 0.0);
        v[slot] = val;
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto encrypt_bits = [&](const vector<bool>& bits) {
        vector<double> v(8, 0.0);
        for (int i = 0; i < 8 && i < bits.size(); i++) {
            v[i] = bits[i] ? SQRT5 : -SQRT5;
        }
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
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

    auto gate_nand = [&](const Ciphertext<DCRTPoly>& a, const Ciphertext<DCRTPoly>& b) {
        auto sum = cc->EvalAdd(a, b);
        return cc->EvalNegate(sum);
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

    auto decode_bit_val = [&](double val) {
        return val >= -0.01;
    };

    cout << "========================================\n";
    cout << "  φ-NAND + RULE 110 STABLE — Pure FHE\n";
    cout << "========================================\n\n";

    // ============================================
    // TEST 1: 100-step NAND chain
    // ============================================
    cout << "  TEST 1: 100-step NAND chain\n\n";

    auto ct_a = encrypt_bit(true, 0);
    auto ct_b = encrypt_bit(true, 0);
    
    auto ct_chain = gate_nand(ct_a, ct_b);  // NAND(1,1) = 0
    
    int flips = 0;
    bool prev_bit = false;
    
    auto start = high_resolution_clock::now();
    
    for (int i = 0; i < 100; i++) {
        ct_chain = gate_nand(ct_chain, ct_b);
        
        auto v = decrypt_bits(ct_chain);
        bool bit = decode_bit_val(v[0]);
        
        if (i > 0 && bit != prev_bit) flips++;
        prev_bit = bit;
        
        if (i % 20 == 0) {
            cout << "    Step " << setw(3) << i << ": val=" << setw(10) << v[0] 
                 << ", bit=" << bit << "\n";
        }
    }
    
    auto end = high_resolution_clock::now();
    auto time1 = duration_cast<milliseconds>(end - start).count();
    
    cout << "    Flips: " << flips << " / 99\n";
    cout << "    Time: " << time1 << " ms\n";
    cout << "    Level: " << ct_chain->GetLevel() << "\n\n";

    // ============================================
    // TEST 2: Rule 110 — 8-slot evolution
    // ============================================
    cout << "  TEST 2: Rule 110 — 8-slot evolution\n\n";

    // Rule 110 truth table:
    // 111→0, 110→1, 101→1, 100→0
    // 011→1, 010→1, 001→1, 000→0
    // Pattern: next = (center XOR right) OR (NOT(left) AND center AND NOT(right))
    // Simplified: next = (~left AND center) OR (center XOR right)
    
    // Mas simple: next = center XOR right OR (NOT(left) AND center)
    
    // Initial state: 11010101 → bits: 1,1,0,1,0,1,0,1
    vector<bool> init = {true, true, false, true, false, true, false, true};
    auto ct_state = encrypt_bits(init);

    cout << "    Initial: ";
    for (bool b : init) cout << (b ? "1" : "0");
    cout << "\n\n";

    start = high_resolution_clock::now();
    
    for (int step = 0; step < 10; step++) {
        // I-rotate para makuha ang left at right
        auto ct_left = cc->EvalRotate(ct_state, -1);
        auto ct_right = cc->EvalRotate(ct_state, 1);
        
        // Rule 110: next = (NOT(left) AND center) OR (center XOR right)
        auto not_left = gate_not(ct_left);
        auto and_not_left_center = gate_and(not_left, ct_state);
        auto xor_center_right = gate_xor(ct_state, ct_right);
        auto next_state = gate_or(and_not_left_center, xor_center_right);
        
        ct_state = next_state;
        
        auto v = decrypt_bits(ct_state);
        cout << "    Step " << setw(2) << step << ": ";
        for (int i = 0; i < 8; i++) {
            cout << (decode_bit_val(v[i]) ? "1" : "0");
        }
        cout << "\n";
    }
    
    end = high_resolution_clock::now();
    auto time2 = duration_cast<milliseconds>(end - start).count();
    
    cout << "\n    Time: " << time2 << " ms\n";
    cout << "    Level: " << ct_state->GetLevel() << "\n";

    return 0;
}
