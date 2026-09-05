// ============================================
// φ-BEATTY GATES — Natural φ-based Decode
// NAND + Rule 110 gamit ang Beatty theorem
// Walang hardcode na threshold
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

    const double PHI = (1.0 + sqrt(5.0)) / 2.0;
    const double SQRT5 = sqrt(5.0);

    // Beatty check — φ-natural na binary decode
    auto is_beatty = [&](int k) {
        if (k <= 0) return true;
        return floor((k + 1) / PHI) > floor(k / PHI);
    };

    auto encrypt_bit = [&](bool bit, int slot) {
        double val = bit ? SQRT5 : -SQRT5;
        vector<double> v(8, 0.0);
        v[slot] = val;
        Plaintext pt = cc->MakeCKKSPackedPlaintext(v);
        return cc->Encrypt(keyPair.publicKey, pt);
    };

    auto decrypt_vals = [&](const Ciphertext<DCRTPoly>& ct) {
        Plaintext pt;
        cc->Decrypt(keyPair.secretKey, ct, &pt);
        pt->SetLength(8);
        auto res = pt->GetCKKSPackedValue();
        vector<double> out;
        for (int i = 0; i < 8; i++) out.push_back(res[i].real());
        return out;
    };

    auto decode_beatty = [&](double val) {
        int int_k = (int)round(abs(val));
        return is_beatty(int_k);
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
    cout << "  φ-BEATTY GATES — Natural φ-based Decode\n";
    cout << "========================================\n\n";

    // ============================================
    // TEST 1: Lahat ng gates — Beatty decode
    // ============================================
    cout << "  TEST 1: Lahat ng gates\n\n";
    cout << "  A B | NAND | XOR | AND | OR | NOT(A)\n";
    cout << "  ----|------|-----|-----|----|-------\n";

    int total_pass = 0;
    int total_tests = 0;

    for (bool A : {false, true}) {
        for (bool B : {false, true}) {
            auto ct_a = encrypt_bit(A, 0);
            auto ct_b = encrypt_bit(B, 0);
            
            auto nand_ct = gate_nand(ct_a, ct_b);
            auto xor_ct = gate_xor(ct_a, ct_b);
            auto and_ct = gate_and(ct_a, ct_b);
            auto or_ct = gate_or(ct_a, ct_b);
            auto not_ct = gate_not(ct_a);
            
            int nand_val = decode_beatty(decrypt_vals(nand_ct)[0]);
            int xor_val = decode_beatty(decrypt_vals(xor_ct)[0]);
            int and_val = decode_beatty(decrypt_vals(and_ct)[0]);
            int or_val = decode_beatty(decrypt_vals(or_ct)[0]);
            int not_val = decode_beatty(decrypt_vals(not_ct)[0]);
            
            int exp_nand = !(A && B);
            int exp_xor = (A != B);
            int exp_and = (A && B);
            int exp_or = (A || B);
            int exp_not = !A;
            
            total_pass += (nand_val == exp_nand) + (xor_val == exp_xor) +
                         (and_val == exp_and) + (or_val == exp_or) + (not_val == exp_not);
            total_tests += 5;
            
            cout << "  " << A << " " << B << " | "
                 << setw(4) << nand_val << " | "
                 << setw(3) << xor_val << " | "
                 << setw(3) << and_val << " | "
                 << setw(2) << or_val << " | "
                 << setw(5) << not_val << " "
                 << ((nand_val == exp_nand && xor_val == exp_xor &&
                      and_val == exp_and && or_val == exp_or && not_val == exp_not)
                     ? "✅" : "❌") << "\n";
        }
    }

    cout << "\n  Total: " << total_pass << "/" << total_tests << "\n\n";

    // ============================================
    // TEST 2: Rule 110 — 8-slot evolution
    // ============================================
    cout << "  TEST 2: Rule 110 — 8-slot\n\n";

    vector<double> init_vals(8, 0.0);
    init_vals[0] = SQRT5;  // 1
    init_vals[1] = SQRT5;  // 1
    init_vals[2] = -SQRT5; // 0
    init_vals[3] = SQRT5;  // 1
    init_vals[4] = -SQRT5; // 0
    init_vals[5] = SQRT5;  // 1
    init_vals[6] = -SQRT5; // 0
    init_vals[7] = SQRT5;  // 1

    Plaintext pt_init = cc->MakeCKKSPackedPlaintext(init_vals);
    auto ct_state = cc->Encrypt(keyPair.publicKey, pt_init);

    cout << "    Initial: 11010101\n\n";

    auto start = high_resolution_clock::now();

    for (int step = 0; step < 8; step++) {
        auto ct_left = cc->EvalRotate(ct_state, -1);
        auto ct_right = cc->EvalRotate(ct_state, 1);
        
        // Rule 110: next = (NOT(left) AND center) OR (center XOR right)
        auto not_left = gate_not(ct_left);
        auto and_nl_c = gate_and(not_left, ct_state);
        auto xor_c_r = gate_xor(ct_state, ct_right);
        auto next_state = gate_or(and_nl_c, xor_c_r);
        
        ct_state = next_state;
        
        auto v = decrypt_vals(ct_state);
        cout << "    Step " << step << ": ";
        for (int i = 0; i < 8; i++) {
            cout << (decode_beatty(v[i]) ? "1" : "0");
        }
        cout << "\n";
    }

    auto end = high_resolution_clock::now();
    auto time = duration_cast<milliseconds>(end - start).count();
    
    cout << "\n    Time: " << time << " ms\n";
    cout << "    Level: " << ct_state->GetLevel() << "\n";

    return 0;
}
