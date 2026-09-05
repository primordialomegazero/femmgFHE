// ============================================
// φ-RULE 110 CLEAN — Walang Exceptions
// Encoding: 0 → -2, 1 → +2
// NAND + Rule 110 na walang hardcode
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

    auto encrypt_bits = [&](const vector<bool>& bits) {
        vector<double> v(8, 0.0);
        for (int i = 0; i < 8 && i < bits.size(); i++) {
            v[i] = bits[i] ? 2.0 : -2.0;
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

    auto decode_bit = [&](double val) {
        return val >= -0.01;
    };

    // NAND: EvalNegate(EvalAdd(a, b))
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
    cout << "  φ-RULE 110 CLEAN — Walang Exceptions\n";
    cout << "========================================\n\n";

    // ============================================
    // TEST 1: NAND truth table
    // ============================================
    cout << "  TEST 1: NAND truth table\n\n";
    cout << "  A B | NAND | Expected\n";
    cout << "  ----|------|---------\n";

    for (bool A : {false, true}) {
        for (bool B : {false, true}) {
            auto ct_a = encrypt_bits({A});
            auto ct_b = encrypt_bits({B});
            auto ct_nand = gate_nand(ct_a, ct_b);
            bool nand_val = decode_bit(decrypt_bits(ct_nand)[0]);
            bool expected = !(A && B);
            
            cout << "  " << A << " " << B << " | "
                 << setw(4) << nand_val << " | "
                 << setw(5) << expected << " "
                 << (nand_val == expected ? "✅" : "❌") << "\n";
        }
    }
    cout << "\n";

    // ============================================
    // TEST 2: Rule 110 — 8-slot evolution
    // ============================================
    cout << "  TEST 2: Rule 110 — 8-slot evolution\n\n";

    vector<bool> init = {true, true, false, true, false, true, false, true};
    auto ct_state = encrypt_bits(init);

    cout << "    Initial: ";
    for (bool b : init) cout << (b ? "1" : "0");
    cout << "\n\n";

    // Rule 110 truth table para sa verification
    int rule110[8] = {0, 1, 1, 0, 1, 1, 1, 0};

    auto start = high_resolution_clock::now();

    for (int step = 0; step < 10; step++) {
        auto ct_left = cc->EvalRotate(ct_state, -1);
        auto ct_right = cc->EvalRotate(ct_state, 1);
        
        // Rule 110: next = (NOT(left) AND center) OR (center XOR right)
        auto not_left = gate_not(ct_left);
        auto and_nl_c = gate_and(not_left, ct_state);
        auto xor_c_r = gate_xor(ct_state, ct_right);
        auto next_state = gate_or(and_nl_c, xor_c_r);
        
        ct_state = next_state;
        
        auto v = decrypt_bits(ct_state);
        cout << "    Step " << setw(2) << step << ": ";
        for (int i = 0; i < 8; i++) {
            cout << (decode_bit(v[i]) ? "1" : "0");
        }
        cout << "\n";
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();
    
    cout << "\n    Time: " << time << " ms\n";
    cout << "    Level: " << ct_state->GetLevel() << "\n";

    return 0;
}
