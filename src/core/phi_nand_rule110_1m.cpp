// ============================================
// φ-NAND + RULE 110 1M — Scale Test
// NAND: EvalAdd + EvalNegate lang
// Rule 110: 8-slot evolution
// Walang decrypt sa loop
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

    auto encrypt_bits = [&](const vector<double>& vals) {
        Plaintext pt = cc->MakeCKKSPackedPlaintext(vals);
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
    cout << "  φ-NAND + RULE 110 1M — Scale Test\n";
    cout << "========================================\n\n";

    // ============================================
    // TEST 1: 1M NAND chain
    // ============================================
    cout << "  TEST 1: 1M NAND chain\n\n";

    // Start sa NAND(1,1) = 0
    vector<double> v_a(8, 2.0);    // 1
    vector<double> v_b(8, 2.0);    // 1
    
    auto ct_a = encrypt_bits(v_a);
    auto ct_b = encrypt_bits(v_b);
    
    auto ct_chain = gate_nand(ct_a, ct_b);  // NAND(1,1) = 0
    
    int N = 1000000;
    
    auto start = high_resolution_clock::now();
    
    for (int i = 0; i < N; i++) {
        ct_chain = gate_nand(ct_chain, ct_b);
    }
    
    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();
    
    auto v_final = decrypt_bits(ct_chain);
    bool final_bit = decode_bit(v_final[0]);
    
    cout << "    Time: " << time / 1000.0 << " seconds\n";
    cout << "    Ops/sec: " << (N * 1000.0) / time << "\n";
    cout << "    Final bit: " << final_bit << "\n";
    cout << "    Expected: " << (N % 2 == 0 ? 0 : 1) << " (alternating)\n";
    cout << "    Level: " << ct_chain->GetLevel() << "\n\n";

    // ============================================
    // TEST 2: 1M Rule 110 steps
    // ============================================
    cout << "  TEST 2: 1M Rule 110 steps\n\n";

    // Initial: 11010101
    vector<double> init(8, 0.0);
    init[0] = 2.0;  // 1
    init[1] = 2.0;  // 1
    init[2] = -2.0; // 0
    init[3] = 2.0;  // 1
    init[4] = -2.0; // 0
    init[5] = 2.0;  // 1
    init[6] = -2.0; // 0
    init[7] = 2.0;  // 1

    auto ct_state = encrypt_bits(init);

    start = high_resolution_clock::now();
    
    for (int step = 0; step < N; step++) {
        auto ct_left = cc->EvalRotate(ct_state, -1);
        auto ct_right = cc->EvalRotate(ct_state, 1);
        
        auto not_left = gate_not(ct_left);
        auto and_nl_c = gate_and(not_left, ct_state);
        auto xor_c_r = gate_xor(ct_state, ct_right);
        auto next_state = gate_or(and_nl_c, xor_c_r);
        
        ct_state = next_state;
    }
    
    end = high_resolution_clock::now();
    time = duration_cast<milliseconds>(end - start).count();
    
    auto v_state = decrypt_bits(ct_state);
    
    cout << "    Time: " << time / 1000.0 << " seconds\n";
    cout << "    Steps/sec: " << (N * 1000.0) / time << "\n";
    cout << "    Final state: ";
    for (int i = 0; i < 8; i++) {
        cout << (decode_bit(v_state[i]) ? "1" : "0");
    }
    cout << "\n";
    cout << "    Level: " << ct_state->GetLevel() << "\n";

    return 0;
}
